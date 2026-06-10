# Lore SHAKE256 Report — 256-bit Only

SHAKE/SHA3-interface comparison branch. **Lore-L4 only.**

## Branch Role

This is a **256-bit security comparison / legacy** branch. The SM3 branch is the official-submission-aligned version.

Only **Reference_Implementation/Lore-L4** is active. L1-L3 and the Optimized_Implementation (SM3/pseudoXOF wrapper) have been archived.

L1-specific optimizations (t=2 bitset, zero-noise shortcut, q-only pointwise) are not applicable to L4 because L4 uses t=4. The L1 optimization note is archived at `docs/archive/`. Current L4 is a validated stable baseline, not claimed as globally optimal.

**KEM layer**: H = SHA3-256, G = SHAKE256, KDF = SHAKE256. Pure Keccak/SHAKE FIPS202 (no SM3 wrapper).

## Active Files

| Path | Description |
|------|-------------|
| `Implementations/Reference_Implementation/Lore-L4/` | 256-bit pure SHAKE Reference |
| `Test_Vectors/KAT_KEM_Lore-L4.txt` | Pure SHAKE Reference KAT |
| `report/SHAKE_FIPS202_AND_LIBOQS_PERF_REPORT.md` | Full FIPS202 replacement + ML-KEM report |
| `report/performance/` | Performance CSVs (full + 256-only) |
| `report/run_performance.sh` | One-click performance entry |

## Archived

| Item | Location | Reason |
|------|----------|--------|
| Reference Lore-L1/L2/L3 | `debug/removed_security_levels/` | Not 256-bit |
| Optimized_Implementation (all levels) | `debug/removed_optimized_wrapper/` | SM3/pseudoXOF wrapper |
| Test_Vectors L1-L3 | `debug/removed_security_levels/Test_Vectors/` | Not 256-bit |

## Performance — Lore-L4 (256-bit)

### Pure SHAKE vs Old Wrapper

| Operation | Before (wrapper) | After (pure SHAKE) | Speedup |
|---|---:|---:|---:|
| keypair | 1040.2 us | 388.6 us | 2.68x |
| encaps | 1173.6 us | 516.4 us | 2.27x |
| decaps | 1292.4 us | 671.5 us | 1.92x |

### SHAKE Pure vs SM3 Reference

| Operation | SHAKE us | SM3 us | SM3 slower |
|---|---:|---:|---:|
| keypair | 388.6 | 1029.2 | 2.65x |
| encaps | 516.4 | 1157.5 | 2.24x |
| decaps | 671.5 | 1267.9 | 1.89x |

### SHAKE vs ML-KEM-1024

| Operation | Lore-SHAKE-L4 | ML-KEM-1024 |
|---|---:|---:|
| keypair | 388.6 us | 65.2 us |
| encaps | 516.4 us | 137.7 us |
| decaps | 671.5 us | 217.8 us |

CSV: `performance/shake_256_vs_liboqs_mlkem1024_reference_generic.csv`

## Build and Test

```bash
cd /home/syh/Work/zj-cuc-lore/Implementations/Reference_Implementation/Lore-L4
make clean && make KAT_KEM_4 && ./KAT_KEM_4
```

## Notes

- This is NOT the official submission branch (that is SM3).
- No Optimized SHAKE implementation is active (pending rewrite from SM3 wrapper).
- All performance numbers are generic mode (no native/AVX2/SIMD).
