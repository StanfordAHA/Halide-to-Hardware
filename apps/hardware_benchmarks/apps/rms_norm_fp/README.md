# Single-pass Llama RMSNorm

`apps/rms_norm_fp` computes a 64 × 2048 BF16 RMSNorm in one CGRA invocation,
unrolled by 16:

```
y[row, channel] = x[row, channel] / sqrt(mean(x[row, :] ** 2)) * gamma[channel]
```

Sixteen dual-read MEMs retain activations during reduction. Two further
sets of 16 MEMs act as deep FIFOs before and after gamma multiplication.
Each gamma multiplier has an 8-word balancing Pond feeding its output MEM.
This absorbs coefficient and output skew while keeping rows overlapped.
Only the final output returns to GLB; gamma is a full per-channel vector.

Fixed placement uses 78 PEs and 53 MEMs on the existing 28 × 16 chip, with
12 fabric columns removed, E64 multi-bank IO, and inactive MU OC=32.

| Stream | GLB tiles | Fabric columns | Bytes per bank |
|---|---|---|---:|
| Activation | 6–7 | 12–15 | 65,536 |
| Output | 10–11 | 20–23 | 65,536 |
| Gamma | 12–13 | 24–27 | 1,024 |

All IO sits above intact fabric, avoiding the tall switch boxes. E64 packing
preserves input transfer sizes when coalescing lanes into banks.
Activation/reduction buffers use 64-word blocks; affine FIFOs use 8-word
blocks. Both retain pending-SRAM-write margin and protect unread values
when their 2048-word storage wraps.

The standalone regression is `apps/rms_norm_fp_RV_E64_MB`. The Llama entry is
`llama_prefill-layer_norm_default::rms_norm_fp_llama_prefill_RV_E64_MB` in
`pr_aha9`, using original activations, full gamma, and existing 64-row host
tiling. The split flow's BF16 arithmetic, reciprocal-square-root approximation,
and omission of epsilon are preserved. No RTL changes are needed.

RTL verification matched all 131,072 outputs against the existing BF16
arithmetic model. The kernel takes **9,771 active cycles**, compared with **16,365** across three invocations: **1.67×
faster**. Whole-tensor throughput utilization is 83.84%; steady-state
utilization is 98.46% (128 words per lane every 130 cycles). The normalization
broadcast showed zero downstream backpressure. These
are kernel measurements; configuration and host transfers are excluded.

The existing approximation reaches roughly 8–9% relative error against FP64
on the broad input-exponent test, for both 16- and 32-lane reductions.

Fresh P&R and VCS RTL regression on 2026-09-18, with the original placer
exponent 6 and no input extent override, took **9,771 cycles** after removing
16 output identity PEs and the reciprocal branch dummy PE (previously 9,773).
No identity PEs remain. Placement and balancing IDs were remapped to the
surviving operators. The broadcast had zero downstream backpressure; every
output lane delivered 8,192 words, with transfers at most 3 cycles apart
after warm-up.
All 131,072 random-input outputs matched the BF16 model of the programmed LUTs,
and every output passed the standard FP tolerance check. The emitted log-ROM
initialization omits its leading zero entry; the bit-accurate model includes
that existing lookup shift, rather than treating it as ideal RMSNorm arithmetic.

The shared normalization template also removes identity PEs from the legacy
32-lane RMS pass (143 to 110 PEs). That pass needs its output Ponds rebalanced
on the normalization multipliers: deleting padding without that update stalls
after eight rows in RTL. With per-lane 8-word Ponds, correct 64-row stream
extents, and block-based SRAM guards, all 131,072 outputs are bit-exact and
the pass completes in 5,450 cycles with no broadcast backpressure.
