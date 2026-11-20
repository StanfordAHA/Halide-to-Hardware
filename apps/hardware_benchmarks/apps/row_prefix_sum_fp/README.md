# Row prefix sum (cumsum along the last axis)

`apps/row_prefix_sum_fp` computes an inclusive cumsum of each row of a
512 × 128 BF16 tensor in one CGRA call:

```
output[r, c] = input[r, 0] + input[r, 1] + ... + input[r, c]
```

The row length 128 is the Nemotron Mamba SSD chunk length. The inputs are in
[-1, 0], like the `dA` values the SSD sums. The graph comes from the Strait
template `row_prefix_sum_bf16`, the Zircon port of aha-agate's template of the
same name.

## How it runs on Zircon

The 32 lanes hold 32 consecutive columns of one row each cycle (one group); a
row is G = 4 groups.

- A Brent-Kung network of 57 `fp_add` PEs forms the group's local prefix
  sums. Its last lane is the group total T.
- The carry of group g is T[g-1] + T[g-2] + T[g-3] of the same row, the
  totals of the earlier groups. Three shift Ponds (`get_row_shift_pond`)
  return T shifted by 1, 2 and 3 within the row, with 0 where the shift
  passes the row start; two adds sum them. Nothing loops, so a carry leaves
  every cycle.
- Each lane's local prefix waits in a MEM FIFO until its carry arrives, and
  a final `fp_add` per lane adds the carry.

The placed design has a register FIFO at every interconnect hop, so two
branches of one value that meet again run at full rate only if their
latencies match. Each unbalanced fork therefore gets an elastic FIFO on its
short branch:

- **Down-sweep adds** join a completed prefix with a partial sum 1–7 adds
  shallower. The shallow operand waits in a MEM FIFO (gap ≥ 4) or a Pond
  FIFO (`get_fifo_pond`).
- **Carry broadcast:** the carry reaches each lane through a Pond FIFO, so
  the 8 lanes of one output GLB packet do not wait for the longest carry
  route.

Two Zircon behaviors shape the FIFOs:

- **No fan-out handshake inside a tile.** A PE output reaches its own tile's
  Pond without the fan-out handshake, so a Pond packed with the PE that
  writes it would store a value again whenever another reader stalls. Every
  Pond FIFO therefore sits in the tile of the PE that reads it, written
  through a register.
- **Pond input ready is tied high.** Any write the schedule holds back is
  lost. A Pond FIFO must never fill, and a shift Pond must never wait for its
  reader. Each shift Pond therefore feeds a nop PE that writes into a MEM
  FIFO, and the carry adds join the MEM outputs.

Resources: 94 PEs, 49 MEMs, 47 Ponds, 47 registers; input at x = 12–19,
output at 20–27. Rows of 2 to 8 groups (`vec_width / 32`) and 2 to 1024 rows
are supported.

## Running

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_fp_RV":["apps/row_prefix_sum_fp_RV_E64_MB"]}'
```

Bit-exact check afterwards: `python strait/coreir_backend/zircon_tools/kernel_models.py
cumsum <this app dir>`.

## Validation

`apps/row_prefix_sum_fp_RV_E64_MB` passes the regression FP check. All 65,536
RTL outputs match a bit-accurate model of the add order bit for bit. Against
an FP64 cumsum: max relative error 1.05%, mean 0.22%.

The kernel takes **2,652 cycles** for 2,048 groups per lane, 77% of one
group per cycle (previously 14,898 cycles, 14%). Kernel cycles only; no RTL
changes.

In steady state input and output run at about 82%. The rest of the loss
comes from the up-sweep. Its adds join two operands of the same depth whose
routes differ by a few hops, and none of these joins has an elastic buffer.
Through the four input GLB packets they hold all 32 lanes to the slowest
join. Making them elastic would need a MEM FIFO per join, since a Pond FIFO
there could fill. That is more MEMs than are left (15 free).

Balancing these joins exactly with pad registers was also tried. Each
join's mismatch was computed from `design.route`; this static model matches
the first-token times in the waveform to within a constant 1 cycle. Every
round of padding, however, moved the placement and opened new mismatches
of 1–4 cycles elsewhere. The first round hung, most likely because a gap-3
Pond FIFO overflowed at the higher rate (not confirmed in a waveform). The
next two rounds used MEM FIFOs for gap 3 and ran at 2,987 and 2,951 cycles,
slower than the unpadded design, so the template does not use pads.

The FIFO Ponds here peak at 18 of their 28 words (measured). Handshake
timing does not depend on the data, so this holds for every input of this
shape.
