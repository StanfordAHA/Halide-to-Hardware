# Single-pass BERT LayerNorm

`apps/layer_norm_fp` computes a 128 × 768 BF16 LayerNorm in one CGRA invocation,
unrolled by 16. The regression entry is
`bert-layer_norm::layer_norm_fp_bert_RV_E64_MB` in `pr_aha9`.

The graph computes the row mean, centers the activation, computes reciprocal
standard deviation, and applies per-channel gamma and beta. Rows overlap
through these stages. Only the final output returns to GLB.

Two sets of 16 dual-read MEMs retain original and centered activations.
Another 16 MEMs act as deep FIFOs before channel affine. Gamma multiplication
and beta addition each use an 8-word balancing Pond. Fixed placement uses
129 PEs and 55 MEMs on the existing 28 × 16 chip, with 12 fabric columns
removed, E64 multi-bank IO, and inactive MU OC=32.

| Stream | GLB tiles | Fabric columns |
|---|---|---|
| Activation | 6–7 | 12–15 |
| Beta | 8–9 | 16–19 |
| Output | 10–11 | 20–23 |
| Gamma | 12–13 | 24–27 |

All IO sits above intact fabric, avoiding the tall switch boxes. E64 packing
preserves input transfer sizes when coalescing lanes into banks.
Activation/reduction buffers use 24-word blocks; affine FIFOs use 8-word
blocks. Both retain pending-SRAM-write margin and protect unread values
when their 2048-word storage wraps.

The existing BF16 operations, reciprocal-square-root approximation, and
omission of epsilon are preserved. The 16-lane reduction changes rounding
order relative to the split 32-lane implementation. No RTL changes are needed.

RTL verification matched all 98,304 outputs against the existing BF16
arithmetic model. The fused
kernel takes **7,534 active cycles**, compared with **16,084** across four
invocations: **2.13× faster**. Whole-tensor throughput utilization is 81.55%;
steady-state utilization is 96% (48 words per lane every 50 cycles).
These are kernel measurements; configuration and host transfers are excluded.

Fresh P&R and VCS RTL regression on 2026-09-18, with automatic placement
(exponent 4 selected) and no input extent override, reproduced **7,534 cycles**
after removing 16 add-zero output PEs and the reciprocal branch dummy PE.
No identity PEs remain. Placement and balancing IDs were remapped to the
surviving operators. Every output lane delivered 6,144 words; after warm-up,
transfers were at most 3 cycles apart.
All 98,304 random-input outputs matched the BF16 model of the programmed LUTs.
The standard FP check passed with 3.76% of outputs outside its per-element
tolerance, within its existing 6% allowance. The emitted log-ROM initialization
omits its leading zero entry; the bit-accurate model includes that existing
lookup shift, rather than treating it as ideal LayerNorm arithmetic.

The shared template also removes scalar affine and identity PEs from the
legacy 32-lane normalization pass (143 to 110 PEs). Its final block-based
SRAM schedule completes in 3,922 cycles, with all 98,304 outputs bit-exact.
The legacy 16-lane affine pass now uses two PEs per lane (64 to 32 total),
computing only `input * weight + bias`. Sixteen output MEM FIFOs absorb GLB
lane skew using 8-word blocks. Its 128 × 384 RTL invocation completes in
3,579 cycles, versus 4,191 for the original 64-PE graph with identical flags.
Deleting the identity PEs without buffering took 4,376 cycles, so the FIFOs
are needed to preserve performance. All 49,152 outputs match BF16
multiply-then-add exactly. The standard FP check passes with 0.10% of outputs
outside its per-element tolerance, within the existing 6% allowance.
