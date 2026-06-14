# Lore-SHAKE Reference Implementation

This repository contains Lore-SHAKE reference implementations.

- Formal implementation files: `Implementations/`
- KAT files: `Test_Vectors/`
- Benchmark and size measurement: `bench/`

## Quick Start

```bash
# Default (L1 + L2):
./bench/run_lore_shake_measurements.sh size      # actual serialized sizes
./bench/run_lore_shake_measurements.sh cycles    # KEM cycles + time
./bench/run_lore_shake_measurements.sh all       # both
```

Recommended full benchmark:

```bash
LEVELS="1 2" \
TRIALS=10000 \
ITERS=10000 \
WARMUP=1000 \
CORE=0 \
./bench/run_lore_shake_measurements.sh all
```

LEVELS also accepts `L1 L2` format:

```bash
LEVELS="L1 L2" TRIALS=10000 ITERS=10000 ./bench/run_lore_shake_measurements.sh all
```

## Included Levels

| Level | Paper Name | Security | k | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |

The SHAKE version provides two Lore parameter sets: L1 and L2.

## Backend

- **KEM hash/XOF**: Pure Keccak/SHAKE FIPS202 (`fips202.c` + `symmetric-shake.c`)
- **No**: AES, SHA2, SM3, SM4 in the KEM algorithm hash/XOF path
- **SM3 in auxfunc.c/drng.c**: KAT infrastructure DRNG only, not the KEM algorithm hash backend

## Important

This repository does **not** commit fixed cycle-count result tables. Cycle counts must be measured by the runner on the target platform using `bench/run_lore_shake_measurements.sh`.
