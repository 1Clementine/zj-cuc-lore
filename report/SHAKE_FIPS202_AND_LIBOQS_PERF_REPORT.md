# SHAKE FIPS202 Replacement and liboqs ML-KEM Reference Benchmark

## 1. Purpose

This experiment:

1. Fixes the SHAKE Reference implementation's `fips202.c/h` — the previous version was not pure SHAKE but an SM3/pseudoXOF wrapper;
2. Replaces it with pure Keccak/SHAKE FIPS202 (taken from the repo root);
3. Re-runs KAT and performance;
4. Compares performance against liboqs ML-KEM Reference on the same platform, same generic compilation profile.

## 2. Environment

- **CPU**: `$(lscpu | grep 'Model name')`
- **Cores**: 32
- **gcc**: gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
- **SHAKE repo**: SHAKE256 branch, commit `8eb94b1`
- **liboqs**: commit `$(cd /home/syh/Work/liboqs && git rev-parse --short HEAD)`
- **Compile flags**: `-O3 -DNDEBUG -std=c99`, no `-march=native`, no `-mavx2`, no `-maes`, no `-msha`
- **liboqs CPU-specific flags**: NONE detected (verified: no `-march=native/-mavx2/-maes/-msha` in build artifacts)

Verification log: `report/validation/logs/liboqs_cpu_specific_flags.txt` (empty — clean).

## 3. FIPS202 Replacement Audit

| Level | Old fips202 contains pseudoXOF/sm3hash? | New fips202 pure Keccak? |
|---|---|---|
| L1 | YES (3 refs) | YES (0 pseudoXOF/sm3hash) |
| L2 | YES | YES |
| L3 | YES | YES |
| L4 | YES | YES |

Old files backed up to: `report/backups/fips202_before_pure_shake/`

## 4. KAT Validation

| Level | Reference KAT Status | Lines | Output File |
|---|---|---|---|
| L1 | PASS | 120 | `report/validation/kat_pure_shake_reference/KAT_KEM_Lore-L1.txt` |
| L2 | PASS | 120 | `report/validation/kat_pure_shake_reference/KAT_KEM_Lore-L2.txt` |
| L3 | PASS | 120 | `report/validation/kat_pure_shake_reference/KAT_KEM_Lore-L3.txt` |
| L4 | PASS | 120 | `report/validation/kat_pure_shake_reference/KAT_KEM_Lore-L4.txt` |

All 10 KAT cases per level pass. Encaps/decaps shared secrets verified internally by the KAT driver.

## 5. Lore SHAKE Reference Performance Before/After

10000 iterations, 1000 warmup, MODE=generic.

| Level | Operation | Before avg us | After avg us | Speedup |
|---|---:|---:|---:|---:|
| L1 | keypair | 63.5 | 35.6 | 1.78x |
| L1 | encaps | 103.4 | 68.6 | 1.51x |
| L1 | decaps | 130.9 | 95.5 | 1.37x |
| L2 | keypair | 218.2 | 113.1 | 1.93x |
| L2 | encaps | 275.4 | 164.9 | 1.67x |
| L2 | decaps | 320.5 | 211.7 | 1.51x |
| L3 | keypair | 495.3 | 250.0 | 1.98x |
| L3 | encaps | 584.6 | 309.2 | 1.89x |
| L3 | decaps | 644.7 | 385.2 | 1.67x |
| L4 | keypair | 1040.2 | 388.6 | 2.68x |
| L4 | encaps | 1173.6 | 516.4 | 2.27x |
| L4 | decaps | 1292.4 | 671.5 | 1.92x |

**Replacing the SM3/pseudoXOF wrapper with pure Keccak/SHAKE FIPS202 gives 1.37x–2.68x speedup.** The improvement is most pronounced at higher levels (L4 keypair: 2.68x).

CSV: `report/performance/shake_fips202_speedup_vs_wrapper.csv`

## 6. liboqs ML-KEM Reference Performance

liboqs built with `-DOQS_OPT_TARGET=generic`, no OpenSSL, no CPU-specific flags.
10000 iterations, 1000 warmup.

| Scheme | Operation | Avg us |
|---|---|---:|
| ML-KEM-512 | keypair | 26.0 |
| ML-KEM-512 | encaps | 56.6 |
| ML-KEM-512 | decaps | 92.8 |
| ML-KEM-768 | keypair | 44.1 |
| ML-KEM-768 | encaps | 92.6 |
| ML-KEM-768 | decaps | 148.1 |
| ML-KEM-1024 | keypair | 65.2 |
| ML-KEM-1024 | encaps | 137.7 |
| ML-KEM-1024 | decaps | 217.8 |

CSV: `report/performance/liboqs_mlkem_reference_generic.csv`

## 7. Lore SHAKE vs liboqs ML-KEM

Cross-family comparison on the same platform, generic compilation.
ML-KEM-512 mapped to Lore-L1/L2 (128-bit security), ML-KEM-768 to L3, ML-KEM-1024 to L4.

| Level | Operation | Lore pure SHAKE us | ML-KEM us |
|---|---:|---:|---:|
| L1 / ML-KEM-512 | keypair | 35.6 | 26.0 |
| L1 / ML-KEM-512 | encaps | 68.6 | 56.6 |
| L1 / ML-KEM-512 | decaps | 95.5 | 92.8 |
| L2 / ML-KEM-512 | keypair | 113.1 | 26.0 |
| L2 / ML-KEM-512 | encaps | 164.9 | 56.6 |
| L2 / ML-KEM-512 | decaps | 211.7 | 92.8 |
| L3 / ML-KEM-768 | keypair | 250.0 | 44.1 |
| L3 / ML-KEM-768 | encaps | 309.2 | 92.6 |
| L3 / ML-KEM-768 | decaps | 385.2 | 148.1 |
| L4 / ML-KEM-1024 | keypair | 388.6 | 65.2 |
| L4 / ML-KEM-1024 | encaps | 516.4 | 137.7 |
| L4 / ML-KEM-1024 | decaps | 671.5 | 217.8 |

CSV: `report/performance/lore_shake_vs_liboqs_mlkem_reference_generic.csv`

## 8. Conclusion for Paper Update

1. **Previous SHAKE performance was invalid** — the old `fips202.c` was not pure SHAKE but an SM3/pseudoXOF wrapper, artificially inflating cycle counts.
2. **Pure Keccak/SHAKE FIPS202 restored.** SHAKE L4 KAT passed.
3. **SM3 Reference L4 is 1.89x–2.65x slower than pure SHAKE Reference L4** — SM3 pseudoXOF overhead is significant.
4. **ML-KEM-1024 Reference is ~3x–6x faster than Lore-SHAKE L4 Reference** on the same generic/reference benchmark setting.
5. **Paper data should use:**
   - `report/performance/shake_256_reference_after_pure_fips202_generic.csv` for Lore SHAKE L4
   - `report/performance/liboqs_mlkem_reference_generic.csv` for ML-KEM Reference
6. **Current branch is 256-bit only (Lore-L4).** L1-L3 are archived; L4 is the validated stable SHAKE baseline.

## 9. SHAKE Pure vs SM3 Reference

| Level | Operation | SHAKE us | SM3 us | SM3/SHAKE |
|---|---:|---:|---:|---:|
| L1 | keypair | 35.6 | 66.4 | 1.86x |
| L1 | encaps | 68.6 | 105.1 | 1.53x |
| L1 | decaps | 95.5 | 130.2 | 1.36x |
| L2 | keypair | 113.1 | 219.5 | 1.94x |
| L2 | encaps | 164.9 | 277.3 | 1.68x |
| L2 | decaps | 211.7 | 324.3 | 1.53x |
| L3 | keypair | 250.0 | 479.1 | 1.92x |
| L3 | encaps | 309.2 | 590.3 | 1.91x |
| L3 | decaps | 385.2 | 661.5 | 1.72x |
| L4 | keypair | 388.6 | 1029.2 | 2.65x |
| L4 | encaps | 516.4 | 1157.5 | 2.24x |
| L4 | decaps | 671.5 | 1267.9 | 1.89x |

SM3 (with official pseudoXOF) is 1.36x–2.65x slower than pure SHAKE Keccak FIPS202. L4 keypair is the most affected (2.65x).

CSV: `report/performance/shake_pure_vs_sm3_reference_generic.csv`

## 10. Optimized_Implementation Status

| Level | fips202 state | Action |
|---|---|---|
| Ref L1-L4 | PURE SHAKE | DONE |
| Opt L1-L4 | WRAPPER (SM3/pseudoXOF) | PENDING (not modified) |

The Optimized_Implementation's fips202.c/h in all 4 levels still contains pseudoXOF/sm3hash/pseudohash wrapper calls. This needs a separate fix pass. Until then, Optimized performance data is not usable for SHAKE comparison.

## 11. Test_Vectors

Updated 2026-06-09 to pure SHAKE Reference KAT. Old vectors backed up to `report/backups/Test_Vectors_before_pure_shake/`. SHA256 checksums in `report/validation/kat_reference_sha256sums.txt`.
