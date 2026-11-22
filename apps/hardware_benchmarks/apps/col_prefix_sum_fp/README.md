# Column prefix sum (cumsum along dim -2)

`apps/col_prefix_sum_fp` computes an inclusive cumsum down the rows of each
matrix of a BF16 tensor in one CGRA call:

```
output[b, r, c] = input[b, 0, c] + input[b, 1, c] + ... + input[b, r, c]
```

The test tensor is 8 matrices of 128 × 128 (`vec_height` = 1024 rows in all,
`scan_rows` = 128, `vec_width` = 128), the shape of a Mamba SSD chunked
segment sum with a chunk of 128. The inputs are in [-1, 0], like the decay
terms that sum. The graph comes from the Strait template
`col_prefix_sum_bf16`.

aha-agate only scans the last axis. Its CGRA templates reject other axes, and
the model code transposes so that every scan runs along dim -1; each
transpose is then a separate two-pass CGRA kernel. This kernel scans dim -2
directly, in the activation layout the matrix unit reads and writes.

## How it runs on Zircon

The activation lives in 4 GLB tiles: input in GLB 6–9, output in GLB 10–13,
32 E64 lanes each. Each cycle the 32 lanes hold 32 consecutive columns of
one row. Lane j therefore sees column 32m + j for m = 0 .. G − 1
(G = vec_width / 32 = 4 here), then the next row, then the next matrix. An
output needs only the same column one row up, which reached the same lane G
tokens earlier:

```
y[r, m] = y[r - 1, m] + x[r, m]
```

The lanes need no data from each other, and nothing is transposed.

Per lane there is one `fp_add` PE and the Pond of its tile
(`get_col_cumsum_pond`). The Pond is a ring of its 32 words, holding the
running sums of the lane's columns.

- **Write port** (`w0`): token t (t = r · G + m within a matrix) is written
  to word t mod 32. The PE output is its only source.
- **Read port 0** (`r0`): reads word (t − G) mod 32, the sum of the same
  column one row up, into the PE's data0. The input value is data1.
- **Read port 1** (`r1`): reads word t mod 32 again, the new sum, and sends
  it to the output IO.
- **Clear port**: zeroes the Pond before each matrix, so the first row reads
  zeros.

The loop r0 → PE → w0 stays inside the tile, so its latency does not depend
on placement. With G ≥ 2 a lane takes one value per cycle. The PE output
feeds only its own Pond, and its output FIFO is bypassed, as in the
accumulator templates.

Lake's comparison network compares only a read port with a write port, and
holds one comparison per pair and direction. The five constraints are:

- r0 reads token t once w0 has written token t − G;
- r1 reads token t once w0 has written it;
- w0 stays fewer than 30 tokens ahead of r1;
- r0 starts matrix b after clear b;
- clear b waits until r1 has read matrix b − 1.

The Pond's input ready is tied high, and r1 cannot pace r0 (that would need
a read-to-read comparison). The output must therefore keep up: r1 has to stay
fewer than 30 tokens behind w0, and the w0 guard only turns an overflow into
a hang. In the waveform the ring holds at most 9 words.

Two names matter to the flow:

- The mapper's Pond packing ILP matches Pond and PE names as substrings, so
  the lane index comes first (`lane{i}_col_sum_pond`, `lane{i}_sum_pe`). A
  name such as `col_sum_pond_lane1` also matched lanes 10–19 and broke the
  packing.
- `PE_fifos_bypass_config.json` lists the sum PEs by name prefix.

Resources: 32 PEs and 32 Ponds, no MEMs, 16 IO tiles.

## Running

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_fp_RV":["apps/col_prefix_sum_fp_RV_E64_MB"]}'
```

Two matrices of 256 × 256 (the Mamba2 SSD chunk) are the env set `mamba2_256x256`:

```
aha regress '{"width":28,"height":16,"cols_removed":12,"mu_oc_0":32,"glb_tests_fp_RV":["apps/col_prefix_sum_fp_RV_E64_MB"]}' --env-parameters mamba2_256x256
```

## Validation

`apps/col_prefix_sum_fp_RV_E64_MB` passes the regression FP check.
Against an FP64 cumsum: max relative error 3.5%, mean 0.59%.

The kernel takes **4,267 cycles** for 4,096 values per lane, **96.0%** of one
value per lane per cycle. Two 256 × 256 matrices (`mamba2_256x256`) take
4,177 cycles for 4,096 values per lane, 98.1%. Kernel cycles only; no RTL
changes.

In the waveform each lane writes one value every cycle except at the 7
matrix boundaries, 11 cycles each. At a boundary, the clear waits until r1
has sent out the previous matrix. In steady state r1 runs about 9 tokens
behind w0: the 8 lanes of one output GLB packet are written together, and
their input-plus-output routes differ by up to 8–14 cycles. The remaining
cycles are about 50 of GLB start-up and about 40 of drain.

Limits:

- `vec_width` must be a multiple of 32, with 1 ≤ G < 30 column groups per
  lane. G = 1 still works, but the loop then runs below one value per cycle.
- A matrix of more than 1,024 tokens per lane (`scan_rows · G`, the 11-bit
  Pond extent limit) is counted in chunks of up to 512 tokens, so
  `scan_rows · G` must split into at most 1,024 such chunks.
- There must be 2 to 1,024 matrices.
