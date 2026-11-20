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

One channel needs 17 PEs and 5 MEMs, so 32 channels do not fit the 192 PE and
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

The configurations in `application_parameters.json` are `pass0`, `pass1`,
`pass2_pb` and `pass3_pb` (`default` is `pass0`). The `_pb` passes add
path-balance Ponds (see below).

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
  swish template. `y`, `-y/ln2` and `1 + exp(-y)` each feed a long branch
  directly and a short branch through a FIFO:
  - `y` reaches the final multiply through a MEM FIFO.
  - The other two use a Pond FIFO (`get_fifo_pond`) in the tile of the PE
    that reads it. That Pond is written through the interconnect. Inside a
    tile, a PE output reaches the tile's Pond without the fan-out handshake.
    A Pond packed with the PE that writes it would therefore store a value
    again whenever the other reader stalls.
  - A Pond's input ready is tied high, so a Pond FIFO must never fill (a
    held-back write would be lost). These two only hold the latency
    difference of their fork.

A Pond FIFO only helps when its branch is the short one. Placement decides
that. In pass 3 the branch through the `1 + exp(-y)` Pond of lane 5 was the
long one (21 cycles against 14 through the reciprocal ROM, which is rigid).
That lane ran at 2 · 14 / 35 = 80% and held the whole output packet to it:
1,438 cycles. Pass 2 had smaller mismatches of the same kind (1,290 cycles).

### Path balancing

`pb_plan.py` (in `/aha/tmp/nemotron_port`) reads the placed and routed design
(`design.packed`, `design.route`, `design.place`, `design_top.json`) and
builds the fork-join graph:

- **Latency model.** One cycle per interconnect REG, 2 per PE, 4 per ROM, 1
  per FIFO Pond. A register's initial tokens reduce its edge's latency.
- **Elastic nodes.** Streaming MEM read ports and repeat Ponds are free
  sources. A FIFO Pond lets its reader wait, but its occupancy must stay at
  most 28 words.
- **Packets.** Each E64 multi-bank GLB tile is one input node and one output
  node.

The planner gives every node a firing time and asks every rigid edge to match
its latency exactly. It then places path-balance Ponds (lake
`get_path_balancing_pond`) where the times do not match: at most one per PE
tile, none in a tile that already holds a Pond, with a length of 1, 2, 4, 8
or 16. The second run keeps the first run's placement (`manual.place`,
`MANUAL_PATH_BALANCE_PLACER=1`) and inserts the Ponds
(`POND_PATH_BALANCING=1`), so the measured skew stays valid.

- Pass 2: 5 Ponds (lengths 2, 1, 1, 2, 1) on the SiLU joins of lanes 0, 4 and 7.
- Pass 3: one 8-cycle Pond on the reciprocal-ROM input of lane 5's
  `silu_subexp`.

Passes 0 and 1 need none. After the second run the planner finds no slack
left; the largest FIFO Pond occupancy is 17 words. The plans are in
`path_balancing_configs/pass<k>_path_balancing.json` and
`pass<k>_manual.place`. Before a `pass<k>_pb` run, copy them to
`path_balancing_configs/path_balancing.json` and `manual.place`;
`run_pb.sh` does this and also re-plans from a first run.

The aha-agate template relies on hardware Zircon lacks: `fp_mac`, `fp_exp`,
`fp_recip`, a MEM write port that makes zeros internally, and a Pond write
filter for weight broadcast. This version uses none of them.

Resources per pass: 136 PEs, 40 MEMs, 56 Ponds (plus up to 5 path-balance
Ponds), 64 registers and 14 IO tiles.

## Validation

Each pass passes the regression FP check ("mostly passed"). All 8,192 RTL
outputs of each pass match a bit-accurate model of the PE chain bit for bit.

The 0.3% of outputs outside the default tolerance all have |out| < 0.009.
There, the BF16 tap sum cancels and its absolute error (at most 0.002) is
large relative to the result. Where |out| > 0.1, the max relative error is
2.9% and the mean 0.4%. SiLU alone adds at most 1.1%.

| Pass | Without balancing | With balancing |
|---|---|---|
| 0 | 1,189 | 1,189 (no Ponds needed) |
| 1 | 1,184 | 1,184 (no Ponds needed) |
| 2 | 1,290 | 1,185 |
| 3 | 1,438 | 1,187 |
| **Total** | 5,101 | **4,745** |

The four passes take 4,745 kernel cycles for 1,024 positions × 32 channels
(86% of one position per cycle per pass). The lanes run at one position per
cycle; the remaining ~160 cycles per pass are pipeline fill and GLB start-up
and drain. Kernel cycles exclude configuration, the reconfiguration between
passes and host transfers. No RTL changes.

The SiLU exp ROM covers |y| up to about 88; larger magnitudes overflow the
exponent arithmetic.
