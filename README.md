# Lore-SM3 Reference Implementation

This repository contains Lore-SM3 implementations. SM3 keeps all four parameter sets and the optimized AVX2 implementation.

- Formal implementation files: `Implementations/`
- KAT files: `Test_Vectors/`
- Benchmark helper: `bench/`

SM3 keeps L1/L2/L3/L4 whereas the SHAKE minimal version may only keep L1/L2/L4.

## Quick Start

```bash
./bench/run_lore_sm3_measurements.sh size      # actual serialized sizes
./bench/run_lore_sm3_measurements.sh cycles    # KEM cycles + time
./bench/run_lore_sm3_measurements.sh all       # both
```

Parameters:

```bash
LEVELS="L1 L2 L3 L4" TRIALS=10000 ITERS=10000 WARMUP=1000 CORE=0 IMPLS="ref opt" ./bench/run_lore_sm3_measurements.sh all
```

## Included Levels

| Level | Paper Name | Security | k | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## Important

This repository does **not** commit fixed cycle-count result tables. Cycle counts must be measured by the runner on the target platform using `bench/run_lore_sm3_measurements.sh`. Generated outputs are written under `bench/results/` and ignored by git.
