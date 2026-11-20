# Bias add + softplus (Mamba dt path)

`apps/add_softplus_fp` computes the Nemotron Mamba `add_softplus` op on a
64 × 1024 BF16 tensor:

```
output[r, c] = softplus(input[r, c] + bias[r]),   softplus(s) = ln(1 + exp(s))
```

Rows are Mamba heads, columns are sequence positions and `bias` is the per-head
`dt_bias`, as in aha-agate. The graph comes from the Strait template
`elementwise_broadcast_add_softplus_bf16`.

## GLB layout: four passes

On Zircon an activation tensor lives in 4 GLB tiles: 8 E64 IO tiles carry 32
lanes, so each cycle the 32 lanes hold 32 consecutive values of a row. That is
how the matrix unit writes a layer's output and reads the next layer's input.
The input sits in GLB tiles 6–9 and the output in GLB tiles 10–13.

One lane of this kernel needs 13 PEs and 6 MEMs, so 32 lanes do not fit the 192
PE and 64 MEM tiles. The kernel therefore runs as four CGRA calls (passes).
Pass k reads the 8 lanes of GLB tile 6 + k and writes GLB tile 10 + k. In
the tensor, these are the columns c with c mod 32 in [8k, 8k + 8). The op is
elementwise with a per-row bias, so pass k is the same kernel on a 64 × 256
slice. Each pass is a configuration in `application_parameters.json` (`pass0`
to `pass3`; `default` is `pass0`), run with
`aha regress ... --env-parameters pass<k>`:

| Pass | Input IO x (GLB) | Bias IO x (GLB) | Output IO x (GLB) |
|---|---|---|---|
| 0 | 12–13 (6) | 10 (5) | 20–21 (10) |
| 1 | 14–15 (7) | 12 (6) | 22–23 (11) |
| 2 | 16–17 (8) | 14 (7) | 24–25 (12) |
| 3 | 18–19 (9) | 16 (8) | 26–27 (13) |

The bias is a parameter, not an activation; it sits in a GLB tile next to
the pass's input. In pass 0 that tile (GLB 5) is above the matrix unit. IO
tiles there reach the fabric only through the 16 horizontal tracks of the IO
row's tall switch boxes, and one lane uses only one of them.

## How it runs on Zircon

The Zircon PE has no exp or ln instruction, so both use 256-entry MEM ROMs:

```
s = input + bias; t = max(s, 0); u = -|s|
e = 2^(u / ln2)                      (getffrac -> exp ROM, getfint, addiexp)
v = e + 1; d = e - (v - 1)           (d: the part of e lost when v is rounded)
l = ln(v)                            (low byte of v -> ln ROM)
out = t + (l + d)
```

`d` keeps BF16 precision for negative `s`, where `1 + e` alone rounds away
most of `e`.

Every interconnect hop is a 2-entry register FIFO with a registered ready.
Two rigid paths that leave one fork and meet again run at full rate only if
their latencies match. Otherwise a value moves only every
(L_short + L_long) / (2 L_short) cycles. Two places in this kernel need care:

- **Forks inside a lane.** Four values (`s`, `u / ln2`, `e`, `v`) feed two
  branches that meet again. Each goes through a dual-read MEM, so the two
  branches do not wait for each other.
- **Bias against activation.** The 8 activation lanes of a pass leave the GLB
  together (one packet), and the bias reaches the 8 adds from one IO. These
  two forks meet at 8 PEs, so each lane's two routes would have to match. The
  bias IO writes, through one register, one dual-read MEM per two lanes (the
  bias buffers), and each read port feeds one lane's add. A read port can
  fall behind its writer, so each add takes its bias when its activation
  arrives.

  Without the bias buffers the passes took 2,442 to 3,597 cycles. The
  per-lane mismatch read from `design.route` was 2, 5 and 8 cycles in passes
  1–3, which matches the slowdown. Path-balance Ponds cannot remove the
  last 1-cycle mismatches: a 2,048-value stream needs Pond length ≥ 2, and
  these edges start at an IO, where no PE is free to hold a Pond.

After PnR, `pb_plan.py` (see below) finds no rigid fork-join with slack in
any of the four passes.

ROM tables avoid the word 0: `remove_rom_zeros` in `hack_design_top.py` drops
every 0 from a ROM `init` list, so a 0 would move the later entries to lower
addresses. `ln(1)` and unused entries are stored as -0.0 instead. Tables and
constants are rounded to nearest even.

Resources per pass: 8 lanes, 104 PEs and 52 MEMs. Each lane uses 13 PEs and
6 MEMs (2 ROMs and 4 fork buffers); the 4 bias buffers make up the rest.

## Validation

Each pass, `apps/add_softplus_fp_RV_E64_MB` with `--env-parameters pass<k>`,
passes the regression FP check. All 16,384 RTL outputs of each pass match a
bit-accurate model of the PE chain bit for bit. That model includes the
Zircon multiplier rounding (CW_fp_mult with 3 extra bits, rounded toward
zero, then RNE on those bits).

Against an FP64 reference (inputs in [-8, 8], bias in [-4, 4]): max relative
error 6.4%, mean 0.60%. The largest errors are at very negative `s`, where the
BF16 value of `u / ln2` has few fraction bits.

| Pass | Kernel cycles |
|---|---|
| 0 | 2,266 |
| 1 | 2,260 |
| 2 | 2,258 |
| 3 | 2,260 |
| **Total** | **9,044** |

The four passes move 8,192 values per lane in 9,044 cycles (91% of one value
per lane per cycle). The single-call 8-lane version took 9,235 cycles, but it
used only one GLB tile for input and output. Kernel cycles exclude
configuration, the reconfiguration between passes and host transfers. No RTL
changes.
