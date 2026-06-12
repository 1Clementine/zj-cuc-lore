# Lore-SHAKE Final Benchmark Summary

**Date**: 2026-06-12
**Branch**: SHAKE256
**Commit**: f5a3ec1

## Active Levels

| Level | Paper | Security | k | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

No L3. Teacher's requirement: run up to 256-bit security. L1/L2 are primary results, L4 retained for existing comparison.

## API Sizes

| Level | pk | ct | sk | ss |
|---:|---:|---:|---:| 
| L1 | 610 | 706 | 2108 | 32 |
| L2 | 1186 | 1282 | 4518 | 32 |
| L4 | 2914 | 3170 | 11672 | 32 |

## Benchmark (10000 iters, 1000 warmup, generic)

| Level | keygen med cyc | encaps med cyc | decaps med cyc | keygen us |
|---:|---:|---:|---:|---:|
| L1 | 87,828 | 158,270 | 210,528 | 30.4 |
| L2 | 246,878 | 354,606 | 433,966 | 85.4 |
| L4 | 975,238 | 1,254,270 | 1,498,490 | 337.1 |

Failures: 0 for ALL levels.

## Implementation

- **Backend**: Pure Keccak/SHAKE FIPS202 (`fips202.c` + `symmetric-shake.c`)
- **Optimizations**: AB_inline (barrett/montgomery reduce, fqmul -> static inline)
- **No**: AES, SHA2, SM3/SM4 in algorithm path
- **DRNG**: SM3-DRNG in auxfunc/drng (KAT infrastructure only)

## Validation

- Smoke: L1/L2/L4 all 1000/1000 failures=0
- Secret output in algorithm path: NONE
- memcmp in crypto_kem_dec: NO (constant-time verify+cmov)
- Implicit rejection: verify() + cmov() correctly implemented

## Files

- CSV: `report/final_shake_l1_l2_l4_20260612_160736/shake_l1_l2_l4_kem_cycles.csv`
- Logs: `report/final_shake_l1_l2_l4_20260612_160736/bench_L*.txt`
