# Lore SHAKE256 Report — 256-bit Only

SHAKE/SHA3-interface comparison branch. **Lore-L4 only.**

## Branch Role

This is a **256-bit security comparison / legacy** branch. The SM3 branch is the official-submission-aligned version.

Only **Reference_Implementation/Lore-L4** is active. L1-L3 and the Optimized_Implementation (SM3/pseudoXOF wrapper) have been archived outside the repo.

**KEM layer**: H = SHA3-256, G = SHAKE256, KDF = SHAKE256. Pure Keccak/SHAKE FIPS202.

## Active Files

| Path | Description |
|------|-------------|
| `Implementations/Reference_Implementation/Lore-L4/` | 256-bit pure SHAKE Reference |
| `Test_Vectors/KAT_KEM_Lore-L4.txt` | Pure SHAKE Reference KAT |
| `report/SHAKE_L4_REFERENCE_OPTIMIZATION_REPORT.md` | Formal optimization report (A+B inline) |
| `report/SHAKE_FIPS202_AND_LIBOQS_PERF_REPORT.md` | FIPS202 replacement + ML-KEM benchmarks |
| `report/performance/` | Performance CSVs |
| `report/run_performance.sh` | One-click performance |
| `report/performance_tests/` | bench_kem.c + build/run scripts |

## Optimization

A+B inline optimization merged into SHAKE256 (KAT bit-identical):

| Operation | Before us | After us | Speedup |
|---|---:|---:|---:|
| keypair | 420.4 | 345.8 | 1.22x |
| encaps | 523.7 | 469.2 | 1.12x |
| decaps | 700.8 | 550.8 | 1.27x |

- `reduce.h`: `barrett_reduce`, `montgomery_reduce` → `static inline`
- `ntt.h`: `fqmul` → `static inline`
- 12-bit rejection remains in `SHAKE256-L4-opt` experimental branch only

See `report/SHAKE_L4_REFERENCE_OPTIMIZATION_REPORT.md` for details.

## Performance — Lore-L4 (256-bit)

### Pure SHAKE Reference (optimized)

| Operation | us |
|---|---:|
| keypair | 345.8 |
| encaps | 469.2 |
| decaps | 550.8 |

### vs ML-KEM-1024 Reference

| Operation | Lore-SHAKE-L4 | ML-KEM-1024 |
|---|---:|---:|
| keypair | 345.8 us | 65.2 us |
| encaps | 469.2 us | 137.7 us |
| decaps | 550.8 us | 217.8 us |

### vs SM3 Reference L4

SM3 (pseudoXOF) is 1.89x–2.65x slower than pure SHAKE.

## Build and Test

```bash
cd /home/syh/Work/zj-cuc-lore/Implementations/Reference_Implementation/Lore-L4
make clean && make KAT_KEM_4 && ./KAT_KEM_4
```

## Notes

- Not the official submission branch (that is SM3).
- No Optimized SHAKE implementation (pending rewrite from SM3 wrapper).
- All performance numbers are generic mode (no native/AVX2/SIMD).
