# SHAKE L4 Reference Optimization Report

## 1. Scope

- **Branch**: SHAKE256 (formal)
- **Active instance**: `Implementations/Reference_Implementation/Lore-L4` (256-bit only)
- **Active KAT**: `Test_Vectors/KAT_KEM_Lore-L4.txt`
- **Implementation type**: Reference only
- **Primitive path**: pure Keccak/SHAKE FIPS202 (no SM3/pseudoXOF wrapper)
- **Role**: paper comparison baseline, not the official SM3 submission branch

## 2. Baseline

Pre-optimization performance (generic build, 10000 iterations, 1000 warmup):

| Operation | us |
|---|---:|
| keypair | 420.4 |
| encaps | 523.7 |
| decaps | 700.8 |

Same platform as previous SHAKE / SM3 / liboqs ML-KEM benchmarks.

## 3. Applied Optimizations

Two KAT-bit-identical optimizations were merged into the formal SHAKE256 branch.

### A. q=257 Reduction Inline

**Files**: `reduce.h`, `reduce.c`

`barrett_reduce()` and `montgomery_reduce()` moved from `reduce.c` (out-of-line) to `reduce.h` as `static inline`. This enables cross-translation-unit inlining in all call sites (poly.c, polyvec.c, indcpa.c, sampler.c, etc.). Arithmetic semantics unchanged.

### B. NTT Hot-Path Inline

**Files**: `ntt.h`, `ntt.c`

`fqmul()` moved from `ntt.c` (out-of-line) to `ntt.h` as `static inline`. This reduces function-call overhead in NTT butterfly and basemul hot paths. NTT arithmetic semantics unchanged.

## 4. Validation

| Check | Result |
|---|---|
| L4 KAT regenerated | PASS |
| Regenerated vs Test_Vectors | bit-identical |
| KAT bit-identical | YES |
| Decapsulation correctness | 10/10 cases, 0 failures |

## 5. Performance Result

| Operation | Baseline us | Optimized us | Speedup |
|---|---:|---:|---:|
| keypair | 420.4 | 345.8 | 1.22x |
| encaps | 523.7 | 469.2 | 1.12x |
| decaps | 700.8 | 550.8 | 1.27x |

## 6. Excluded Experimental Optimization

12-bit packed rejection sampling (`LORE_USE_12BIT_REJ_Q`) was tested in the `SHAKE256-L4-opt` experimental branch:

- Correctness: PASS (10/10 KAT cases)
- KAT: differs from baseline (SHAKE byte consumption changes)
- Performance gain: modest (+0.02x–0.06x beyond inline optimizations)

It remains in the experimental branch only. It is not part of the stable SHAKE256 branch because the KAT change requires separate evaluation.

## 7. Final Status

The SHAKE256 formal branch now contains a validated 256-bit pure SHAKE Lore-L4 Reference implementation with KAT-bit-identical inline optimizations for reduction and NTT hot paths. No experimental code, build artifacts, L1 archive notes, or migration audit process files are included.
