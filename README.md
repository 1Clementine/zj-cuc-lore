# Lore-SHAKE Reference Implementation

This repository contains Lore-SHAKE reference implementations.

- Formal implementation files: `Implementations/`
- KAT files: `Test_Vectors/`
- Benchmark and size measurement: `bench/`

## Quick Start

```bash
./bench/run_lore_shake_measurements.sh size      # actual serialized sizes
./bench/run_lore_shake_measurements.sh cycles    # KEM cycles + time
./bench/run_lore_shake_measurements.sh all       # both
```

Parameters:

```bash
TRIALS=10000 ITERS=10000 WARMUP=1000 CORE=0 ./bench/run_lore_shake_measurements.sh all
```

## Included Levels

| Level | Paper Name | Security | k | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

No L3 — main requested range is up to 256-bit (L1/L2 primary). L4 retained for high-security comparison.

## Backend

- **KEM hash/XOF**: Pure Keccak/SHAKE FIPS202 (`fips202.c` + `symmetric-shake.c`)
- **No**: AES, SHA2, SM3, SM4 in the KEM algorithm hash/XOF path
- **SM3 in auxfunc.c/drng.c**: KAT infrastructure DRNG only, not the KEM algorithm hash backend

## Size Clarification

Two different size conventions:

### API sizes (KEM interface buffer sizes from `api.h`)

Used only for KEM API sanity checks, not as final paper size figures.

| Level | pk_api | ct_api | sk_api | ss |
|---:|---:|---:|---:|---:|
| L1 | 610 | 706 | 2108 | 32 |
| L2 | 1186 | 1282 | 4518 | 32 |
| L4 | 2914 | 3170 | 11672 | 32 |

### Actual/PDF sizes (compact serialized sizes)

These are the values used in paper size tables.

| Level | pk_actual | ct_actual | sk_actual |
|---:|---:|---:|---:|
| L1 | 545 | 641 | 821 |
| L2 | 1058 | 1153 | 1942 |
| L4 | 2626 | 2886 | 5373 |

API sizes are fixed KEM interface buffer sizes from `api.h`. Actual sizes are compact serialized sizes. Final size comparisons should use actual/PDF sizes, not API buffer sizes.

## Important

This repository does **not** commit fixed cycle-count result tables. Cycle counts must be measured by the runner on the target platform using `bench/run_lore_shake_measurements.sh`.
