# One-pass MX quantization along dim -2 (get scale + apply scale)

`apps/fused_e8m0_quant_multi_IOs` quantizes a BF16 tensor of
`seq_heads_prod` rows × `head_dim` columns (here 1536 × 64) to MXINT8 in one
CGRA call. Each MX block is 64 consecutive rows of one column, so the scale
is shared along dim -2:

```
scale[s, d] = get_shared_exp(max over r < 64 of |x[64 s + r, d]|)
q[r, d]     = e8m0_quant(x[r, d], scale[r / 64, d])
```

Before this kernel the same work needed two calls with the same shapes and
layouts: `get_e8m0_scale_accum_gb_input` writes the scales to GLB, and
`apply_e8m0_scale_multi_IOs` reads the tensor and the scales again and
quantizes. This kernel writes both outputs in the layouts of those two
kernels:

- `hw_output_mxint8_act`: INT8 values, two adjacent columns per 16-bit word
  (odd column in the high byte), 16 lanes in GLB tiles 12–13;
- `hw_scale_packed_output`: scales, two adjacent columns per 16-bit word,
  16 lanes in GLB tiles 10–11.

The input activation is in GLB tiles 6–9 (32 lanes). The graph comes from the
Strait template `fused_accumulator_quant_pack_bf16`. The dim -1 version is
`apps/fused_e8m0_quant_single_IO`.

## How it runs on Zircon

Lane j holds column 32 g + j for the column groups g = 0 .. G − 1
(G = `head_dim` / 32). The input IO reads two groups at a time, interleaved:
row r of groups 2h and 2h + 1, then row r + 1, for the 64 rows of a block.
For G = 2 this is the plain row-major order. The block maximum is then a
recurrence at distance 2:

```
m[r, s] = abs_max(m[r - 1, s], x[r, s]),   m[-1, s] = 0     (s = 0, 1)
```

Per lane (4 PEs, 2 Ponds, 1 MEM, 1 register):

- **Block |max|.** One `fp_abs_max` PE and the Pond of its tile
  (`get_block_absmax_pond`). The Pond is a ring of 32 words: read port 0
  gives the PE the maximum one row up, the PE writes the new maximum back,
  and the clear zeroes the ring before each block. Read port 1 reads only the
  last row, the two block maxima, once the block is written. The loop stays
  inside the tile, and with two interleaved groups it takes one value per
  cycle.
- **Scale.** `get_shared_exp` turns the two block maxima into the two block
  scales. A register is its only sink and feeds the scale output and the
  replay Pond. A net that ends in a register must not also pass on to
  another sink.
- **Scale output.** `bit8_pack` joins the scales of lanes 2k and 2k + 1 into
  scale output lane k.
- **Scale replay.** The replay Pond (`get_block_replay_pond`) returns the two
  scales once per row of the block, through a pass-through PE (`or_` with 0),
  so the quantizer gets one scale per token, in the token order of the data.
  A Pond output reaches only data0 of its tile's PE, while `e8m0_quant` takes
  the scale on data1, hence the pass-through PE.
- **Data.** The input also goes to a MEM FIFO, which holds each block while
  its scale is computed.
- **Quantize and pack.** One `e8m0_quant` PE per lane takes the data from the
  MEM FIFO and the scale from the replay. `bit8_pack` joins lanes 2k and
  2k + 1 into data output lane k.

Every interconnect hop is a 2-entry register FIFO. A full-rate value that
feeds two joins therefore runs at full rate only if its latency difference
to the two joins is the same in every lane of an output packet. Here every
path that carries one value per cycle is a chain from an elastic source (a
MEM or Pond read port) to an output, with no fan-out. The only fan-out of a
scale is at two values per block, where a latency mismatch costs nothing.

The first version used one full-rate scale stream for both the quantizer and
the scale output: the replay Pond sat in front of `get_shared_exp`, and the
scale output IO overwrote each scale word once per row. It was bit-exact but
took 4,817 cycles (64%). After PnR, the path-balancing analysis
(`strait/coreir_backend/zircon_tools/pb_plan.py`) found rigid slack of up to
6 cycles on 21 edges, all on the two joins of that scale stream.

The mapper's Pond packing ILP may pack the ring Pond with any PE that its
read port 1 reaches. The `get_shared_exp` PE is named `..._biased_exp_pe` (an
E8M0 scale is a biased exponent). A PE whose name contains "bias" is never a
packing target, so the ring stays with its `fp_abs_max` PE.

Resources: 160 PEs, 64 Ponds, 32 MEMs, 32 registers and 16 IO tiles.

## Running

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_RV":["apps/fused_e8m0_quant_multi_IOs_RV_E64_MB"]}'
```

`--env-parameters head_dim_128` runs a 1024 × 128 tensor (G = 4, so the
input and data output IOs use the four-level interleaved address pattern).

## Validation

Both shapes pass the regression's integer comparison, which requires every
word to match the `process.cpp` reference: 49,152 data words and 768 scale
words for 1536 × 64, and 65,536 data words and 1,024 scale words for
1024 × 128.

| Shape (rows × columns) | Tokens per lane | Kernel cycles | Utilization |
|---|---|---|---|
| 1536 × 64 (`default`) | 3,072 | **3,392** | 90.6% |
| 1024 × 128 (`head_dim_128`) | 4,096 | **4,448** | 92.1% |

At 1536 × 64 the two-pass flow takes 3,271 cycles
(`get_e8m0_scale_accum_gb_input`) plus 4,620 cycles
(`apply_e8m0_scale_multi_IOs`), 7,891 in all, and it needs a second
configuration and a second GLB read of the tensor. This kernel takes 3,392
(57% fewer). Kernel cycles only; no RTL changes.

The input IOs move the 3,072 tokens per lane in 3,201 cycles, and the last
output word follows 191 cycles later. The remaining cycles are:

- about 45 cycles of GLB start-up;
- about 3.5 cycles at each of the 24 ring blocks per lane, about 84 in all.
  The clear of block b + 1 waits until read port 1 has read the maxima of
  block b, and the PE starts the block after the clear;
- the last block: its data can be quantized only after its last row has
  arrived and its scale has been computed, so its 128 tokens per lane, plus
  the scale and output latency, follow the end of the input.

After PnR, the path-balancing analysis finds rigid slack only on the scale
path (`get_shared_exp` to the scale pack and the scale output), which carries
two values per block and does not limit the rate.

Interleaving four groups instead of two would halve the block boundaries but
double the last block, so it would only pay off for much longer tensors.

Limits:

- `head_dim` must be a multiple of 64 (two groups of 32 lanes per lane pair
  of the stream).
- `seq_heads_prod` must be a multiple of 64, with 2 to 1,024 ring blocks per
  lane (`seq_heads_prod / 64 * head_dim / 64`).
- The MEM FIFO stream (`head_dim / 32 * seq_heads_prod` words per lane) must
  split into at most 1,024 rows of 8 to 256 words.
