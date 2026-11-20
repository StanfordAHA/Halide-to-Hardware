# One-pass MX quantization (get scale + apply scale)

`apps/fused_e8m0_quant_single_IO` quantizes a 128 × 256 BF16 tensor to MXINT8
in one CGRA call. Each block of 64 consecutive channels of a pixel shares one
E8M0 scale. Before this kernel, the same work needed two calls:
`get_e8m0_scale_tree_gb_input` writes the scales to GLB, and
`apply_e8m0_scale_single_IO` reads the tensor again and quantizes it. This
kernel writes the same two outputs in the same GLB layouts:

- `hw_output_mxint8_act`: INT8 values, two per 16-bit word (odd channel in
  the high byte)
- `hw_scale_packed_output`: scales, two per 16-bit word, split over two bank
  toggled IOs

The graph comes from the Strait template `fused_reduction_quant_pack_bf16`. It
is the Zircon counterpart of the template of the same name on aha-agate's
`iccd-zircon` branch.

## How it runs on Zircon

32 input lanes hold one pass of 32 channels, so an MX block is an even pass
followed by an odd pass.

- **Scale:** an `fp_abs_max` tree over the lanes gives the pass maximum. A
  MEM splits the pass maxima into even and odd passes, one `fp_abs_max`
  joins each pair into the block maximum, and `get_shared_exp` turns it into
  the scale. The stream has one scale per block.
- **Data:** each lane's values wait in a MEM while their scale is computed.
  That MEM splits the lane into even passes (read port 0) and odd passes
  (read port 1). Each lane has one `e8m0_quant` PE per parity, and each of
  them uses the block scale once, so no scale has to be repeated. A MEM
  schedule that repeats each scale twice (`get_broadcast_mem` with replicate
  factor 2) stopped after 28 scales in RTL, so this design avoids it.
- **Output:** the even-pass words go to output IO tiles 0–3 and the odd-pass
  words to tiles 4–7. The host spreads a tensor's 64-bit words over its IO
  tiles in turn, so this leaves the tensor in its original order in GLB.

The placed design has a register FIFO at every interconnect hop. The GLB
hands over the 8 lanes of one input GLB packet together, and they meet again
in the abs-max tree. In the first version their routes differed by up to 8
cycles, and those rigid joins held the kernel to 63%. Every lane now enters
the tree through a Pond FIFO in the tile of the PE that reads it: even lanes
go into a first-stage PE, odd lanes through a nop PE. With elastic inputs the
tree has no rigid paths that meet again. Three details make this work:

- **Register as the only sink.** A lane register is the IO's only sink and
  writes both the lane buffer and the Pond. A net that ends in a register
  must not also pass that register's switch-box output on to another sink,
  because that output's mux would have to select the register and the bypass
  at once. This was the cause of the router failures during development.
- **The Ponds never fill.** A Pond's input ready is tied high, so a Pond that
  fills loses data. Here the tree's output goes to a MEM, and the four input
  GLB packets start in the same cycle and stream one pass per cycle. Each
  Pond therefore only holds the latency gap of its lane's path; the measured
  maximum is given under Validation.
- **Branch PEs stay.** The block scale still reaches the 64 quantizers
  through 8 branch PEs. A direct fan-out widened the arrival spread and ran
  at 1,325 cycles.

Resources: 154 PEs, 33 MEMs, 32 Ponds, 43 registers and 18 IO tiles: input at
x = 12–19, output at 20–27, scales at 8 and 10.

## Running

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_RV":["apps/fused_e8m0_quant_single_IO_RV_E64_MB"]}'
```

## Validation

`apps/fused_e8m0_quant_single_IO_RV_E64_MB` (group `glb_tests_RV`) passes the
integer comparison. Both outputs match the `process.cpp` reference bit for
bit: 16,384 data words and 256 scale words. That reference is the same as the
two-pass flow's.

The kernel takes **1,200 cycles** for 1,024 passes per lane, 85% of one pass
per cycle (the first version took 1,621 cycles, 63%). In the waveform:

- The input IOs move 1,024 words in exactly 1,024 cycles, and the GLB never
  waits for the fabric.
- Each output lane writes 512 words over 1,023 cycles, one word every two
  cycles as designed.

The steady state therefore runs at full rate, and the remaining 176 cycles
are latency:

- about 44 cycles of GLB start-up before the first input word;
- 132 cycles from the first input word to the first output word: 91 to the
  block scale, 14 through the scale branches to the quantizers, and 23 from
  there to the output IO.

The fixed cost does not grow with the tensor, so larger shapes come closer
to one pass per cycle.

The path-balancing analysis (`strait/coreir_backend/zircon_tools/pb_plan.py`) reports slack on
rigid joins between the scale branches, the quantizers, the packers and the
output packets. Those edges carry one token every two cycles, though, and at
half rate a fork-join keeps up as long as the long branch is at most three
times the short one. So these joins do not limit the kernel, and no
path-balance Ponds are used.

The two-pass flow needs at least 1,024 cycles for each of its two kernels,
plus a second configuration and a second GLB read of the tensor. Kernel
cycles only; no RTL changes.

The lane Ponds peak at 27 of the 30 words their guard allows. This was
measured from an FSDB waveform of the core ports of the lane tiles. Handshake timing does not depend on the data, so the
figure holds for every input of this shape and this placement. Another
placement can move it; an overflow drops a word, and the kernel then hangs
rather than giving wrong results. Two attempts to lower it ran slower and were
dropped:
- A MEM FIFO behind each packet's partial maximum ran at 1,301 cycles, with
  the lane Ponds still at 26 words.
- Removing the branch PEs, as described above.

Limits: `vec_width` must be a multiple of 128, so each row has an even number
of blocks for scale packing.
