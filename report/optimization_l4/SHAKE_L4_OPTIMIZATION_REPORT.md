# SHAKE L4 Optimization Report

## 1. Goal

Optimize the 256-bit Lore-L4 pure SHAKE Reference implementation while maintaining algorithm semantics and KAT correctness.

## 2. Baseline

| Metric | Value |
|---|---|
| Branch | SHAKE256-L4-opt |
| Instance | Lore-L4 (q=257, t=4, N=768, K=3) |
| KAT | PASS, bit-identical to Test_Vectors |
| keypair | 420.4 us |
| encaps | 523.7 us |
| decaps | 700.8 us |

ML-KEM-1024 comparison: keypair 65.2us (6.4x faster), encaps 137.7us (3.8x), decaps 217.8us (3.2x).
SM3 L4 comparison: SM3 1.89x–2.65x slower than SHAKE.

## 3. Hotspot Analysis

Main time consumers (from static code scan):
- **NTT/basemul**: heaviest arithmetic, fqmul called per butterfly
- **poly_reduce / barrett_reduce**: called per coefficient in many paths
- **rej_uniform_q**: 16-bit rejection sampling, SHAKE consumption
- **SHAKE/XOF**: matrix expansion, noise sampling, PRF
- **Pack/unpack**: BCH-coded serialization

## 4. Optimization Results

| Patch | Description | KAT bit-identical? | Keypair | Encaps | Decaps |
|---|---:|---:|---:|---:|---:|
| Baseline | Pure SHAKE FIPS202 | PASS | 420.4 us | 523.7 us | 700.8 us |
| A+B | barrett_reduce/montgomery_reduce/fqmul → static inline in headers | PASS | 345.8 us (1.22x) | 469.2 us (1.12x) | 550.8 us (1.27x) |
| C | SHAKE path audit | N/A (no change) | — | — | — |
| D | 12-bit packed rejection (`LORE_USE_12BIT_REJ_Q` guard) | DIFFERENT | 338.7 us (1.24x) | 443.0 us (1.18x) | 541.1 us (1.30x) |

### Best Result (A+B+D combined): 1.30x decaps, 1.24x keypair, 1.18x encaps

## 5. Implementation Details

### A: q=257 Reduction — Static Inline in Headers

**Files changed**: `reduce.h`, `reduce.c`, `ntt.h`, `ntt.c`

Moved `barrett_reduce()` and `montgomery_reduce()` from `reduce.c` to `reduce.h` as `static inline`. Moved `fqmul()` from `ntt.c` to `ntt.h` as `static inline`. This enables cross-translation-unit inlining in all callers (poly.c, polyvec.c, indcpa.c, sampler.c, etc.).

KAT: bit-identical. Speedup: 1.12x–1.27x.

### B: NTT Hot Function Inlining

Combined with A above. `fqmul` is now inlined in all NTT butterfly and basemul call sites.

### C: SHAKE Path Audit

SHAKE path is already clean:
- No malloc/free in fips202.c (pure Keccak implementation)
- No redundant SHAKE init/finalize in symmetric-shake.c
- No sm3.c/auxfunc.c linked in non-KAT builds
- No changes needed

### D: 12-bit Packed Rejection (Experiment)

**File changed**: `poly.c` — `rej_uniform_q()`

Guarded by `#ifdef LORE_USE_12BIT_REJ_Q`. Each 3 SHAKE bytes produce two 12-bit candidates. Accept range `< 3855 = 15 * 257`. Average ~12.75 SHAKE bits per valid coefficient (vs ~16.0 for 16-bit).

KAT: DIFFERENT (expected — sampling stream changes). Correctness: PASS (10/10 KAT cases, 0 failures). Speedup: +0.02x–0.06x beyond inline.

## 6. Rejected / Deferred Optimizations

| Optimization | Reason |
|---|---|
| t=2 bitset | L4 uses t=4, not applicable |
| zero-noise shortcut | L1 t=2 integer degeneracy, not applicable |
| q-only pointwise multiplication | L4 t=4, CRT path needs t-part |
| delta direct computation | Structural change, needs proof |

## 7. Final Recommendation

**Recommended for merge (SHAKE256-L4-opt → SHAKE256)**:
- Opt A+B: `static inline` for barrett_reduce, montgomery_reduce, fqmul in headers
  - KAT bit-identical, 1.12x–1.27x speedup
  - Low risk, no semantic change

**Recommended as experiment (keep branch only)**:
- Opt D: 12-bit packed rejection with `LORE_USE_12BIT_REJ_Q` guard
  - Correctness proven (0 failures), but KAT changes
  - Modest additional gain (+0.02x–0.06x)
  - Enable only if KAT change is acceptable for paper

**Final optimized performance (A+B merged)**:
- keypair: 345.8 us (was 420.4 us, 1.22x)
- encaps: 469.2 us (was 523.7 us, 1.12x)
- decaps: 550.8 us (was 700.8 us, 1.27x)
