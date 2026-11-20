# Causal depthwise Conv1D + SiLU (Mamba conv1d)

`apps/depthwise_conv1d_silu_fp` runs the Nemotron Mamba `conv1d` (kernel size
4, padding 3, groups = channels, SiLU activation) on an L2 tile of 1024
positions × 32 channels:

```
y[t, c]   = bias[c] + w[c,0] x[t-3, c] + w[c,1] x[t-2, c] + w[c,2] x[t-1, c] + w[c,3] x[t, c]
out[t, c] = y[t, c] * sigmoid(y[t, c])
```

Values before t = 0 are zero. The graph comes from the Strait template
`depthwise_conv1d_silu_bf16`. The host splits longer channel dimensions into
L2 tiles of 32 channels.

## GLB layout: four passes

On Zircon an activation tensor lives in 4 GLB tiles: 8 E64 IO tiles carry 32
lanes. Here each cycle the 32 lanes hold the 32 channels of one position.
That is how the matrix unit writes a layer's output and reads the next
layer's input. The input sits in GLB tiles 6–9 and the output in GLB tiles
10–13.

One channel needs 16 PEs and 7 MEMs, so 32 channels do not fit the 192 PE and
64 MEM tiles. The kernel therefore runs as four CGRA calls (passes). Pass k
reads the 8 lanes of GLB tile 6 + k (channels 8k to 8k + 7) and writes GLB
tile 10 + k. Each pass is the 8-channel kernel with its own weights and
biases.

The weights (4 per channel) and biases are parameters. They come from GLB
tiles that the pass does not use for activations. The GLB tiles 0–5 above the
matrix unit would have to bring all 40 parameter lanes through the 16
horizontal tracks of the IO row's tall switch boxes, so they are not used.

| Pass | Input IO x (GLB) | Weight IO x (GLB) | Bias IO x (GLB) | Output IO x (GLB) |
|---|---|---|---|---|
| 0 | 12–13 (6) | 14–19, 22–23 (7, 8, 9, 11) | 24–25 (12) | 20–21 (10) |
| 1 | 14–15 (7) | 12–13, 16–21 (6, 8, 9, 10) | 24–25 (12) | 22–23 (11) |
| 2 | 16–17 (8) | 12–15, 18–21 (6, 7, 9, 10) | 22–23 (11) | 24–25 (12) |
| 3 | 18–19 (9) | 12–17, 20–21 (6, 7, 8, 10) | 22–23 (11) | 26–27 (13) |

Each pass is a configuration in `application_parameters.json` (`pass0` to
`pass3`; `default` is `pass0`).

## How it runs on Zircon

One lane per channel; each cycle the 8 lanes of a pass receive one position
of its 8 channels.

The placed design has a register FIFO at every interconnect hop. A value that
feeds two branches which meet again therefore runs at full rate only if both
branches have the same latency. Every such fork gets an elastic buffer:

- **x**: written to two dual-read MEMs per lane. Their four read ports feed
  the four `fp_mul` taps, so each tap has its own x stream. The MEMs also
  absorb latency differences between the 8 lanes, which share their GLB
  packets.
- **Taps** (transposed form): product k is delayed by 3 − k register FIFOs
  that each start with one zero. These zeros are the values before t = 0, so
  no padding is added on the host. A balanced tree of `fp_add` PEs computes
  `((p3 + p2) + (p1 + p0)) + bias`. Multiply and add round separately, as
  aha-agate's `fp_mac` does.
- **Weights and bias** come from GLB once. Each value is kept in the Pond of
  the PE that uses it and replayed at every position (`get_repeat_pond`).
- **SiLU** uses the exp ROM and the reciprocal (mantissa) ROM of the Zircon
  swish template. Three values feed two branches that meet again:
  - `y` reaches the final multiply through a MEM FIFO; that branch waits for
    the whole SiLU chain.
  - `-y/ln2` (n) and `1 + exp(-y)` (d) are each written to a dual-read MEM,
    the value's only consumer (the fork buffers). Read port 0 feeds the
    branch through the ROM, read port 1 the other branch. Each read port runs
    ahead as far as its branch lets it, so the two branches need not match.

A ROM lookup is rigid, and which branch of a SiLU fork is longer depends on
the placement. An earlier version put a Pond FIFO on the branch without the
ROM. A Pond FIFO only helps when its branch is the short one: in one
placement of pass 3 the d branch through the Pond took 21 cycles against 14
through the reciprocal ROM. That lane ran at 2 · 14 / 35 = 80% and held the
whole output packet to it (1,438 cycles), and passes 2 and 3 needed
path-balance Ponds from a second, fixed-placement run. A buffer at the fork
works for either branch, so this version needs no path balancing and no
per-pass configuration files.

After PnR, the path-balancing analysis
(`strait/coreir_backend/zircon_tools/pb_plan.py`) finds no rigid fork-join
with slack in any of the four passes.

The aha-agate template relies on hardware Zircon lacks: `fp_mac`, `fp_exp`,
`fp_recip`, a MEM write port that makes zeros internally, and a Pond write
filter for weight broadcast. This version uses none of them.

Resources per pass: 128 PEs, 56 MEMs (two x buffers, a y FIFO, two fork
buffers and two ROMs per lane), 40 Ponds (weights and bias), 48 registers and
14 IO tiles.

## Running

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_fp_RV":["apps/depthwise_conv1d_silu_fp_RV_E64_MB"]}' --env-parameters pass<k>
```

for k = 0 to 3 (no `--env-parameters` runs pass 0). Bit-exact check
afterwards: `python strait/coreir_backend/zircon_tools/kernel_models.py
conv1d <this app dir>`.

## Validation

Each pass passes the regression FP check ("mostly passed"). All 8,192 RTL
outputs of each pass match a bit-accurate model of the PE chain bit for bit.

The 0.34% of outputs outside the default tolerance all have |out| < 0.009.
There, the BF16 tap sum cancels and its absolute error (at most 0.002) is
large relative to the result. Where |out| > 0.1, the max relative error is
2.9% and the mean 0.4%. SiLU alone adds at most 1.1%.

| Pass | Kernel cycles |
|---|---|
| 0 | 1,224 |
| 1 | 1,220 |
| 2 | 1,226 |
| 3 | 1,221 |
| **Total** | **4,891** |

The four passes take 4,891 kernel cycles for 1,024 positions × 32 channels
(84% of one position per cycle per pass). The lanes run at one position per
cycle; the remaining ~200 cycles per pass are pipeline fill and GLB start-up
and drain. The two fork buffers lie on the SiLU path and add about 35 cycles
of that latency. The Pond-FIFO version with path balancing took 4,745 cycles
but needed a second PnR run per pass with a saved placement. Kernel cycles
exclude configuration, the reconfiguration between passes and host
transfers. No RTL changes.

The SiLU exp ROM covers |y| up to about 88; larger magnitudes overflow the
exponent arithmetic.
