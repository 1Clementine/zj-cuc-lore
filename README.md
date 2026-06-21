# Lore-SHAKE Reference Implementation

This repository contains the Lore-SHAKE reference implementation (L1 and L2 only).

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

| Level | Paper Name | Security | KAPPA | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |

The SHAKE version provides two Lore parameter sets: L1 and L2.

## Directory Structure

```
Reference_Implementation/
    Platform-independent reference implementation of Lore.
    Each algorithm instance is placed in an individual subdirectory.

    Lore-L1/
        Reference implementation of the Lore-L1 KEM instance.

    Lore-L2/
        Reference implementation of the Lore-L2 KEM instance.

Optimized_Implementation/
    Reserved for optimized implementation variants.
    No optimized implementation is included in the current SHAKE package.

Additional_Implementation/
    Reserved for additional software or hardware implementation variants.
    No additional implementation is included in the current submission.
```

## Main Files in Each Algorithm Instance

**KEM_AlgorithmInstance.c**
: API_PKC bridge layer for the submitted KEM instance. It maps the Lore key generation, encapsulation and decapsulation routines to the API_PKC KEM interface.

**KEM_AlgorithmInstance.h**
: Algorithm instance configuration and API_PKC function declarations.

**KAT_KEM.c**
: API_PKC-provided KAT generation program for KEM schemes.

**drng.c / drng.h**
: API_PKC-provided deterministic random number generator used by the KAT program.

**auxfunc.c / auxfunc.h**
: API_PKC-provided auxiliary hash/XOF functions.

**fips202.c / fips202.h**
: Compatibility wrapper for the SHAKE/FIPS202-style interfaces used by the Lore source code. In this submission package, the wrapper routes the internal hash/XOF operations to the API_PKC auxiliary functions `pseudohash()` and `pseudoXOF()` from `auxfunc.c`.

**symmetric-shake.c / symmetric.h**
: Symmetric primitive adapter used by Lore. The SHAKE-style calls are routed through `fips202.c` and therefore use the API_PKC auxiliary functions.

**Makefile**
: Automated build script for compiling the API_PKC KAT generator.

## Build / Run Examples

Reference implementation:

```bash
cd Implementations/Reference_Implementation/Lore-L1
make clean
make KAT_KEM_1
./KAT_KEM_1

cd ../Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2
```

Optimized implementation (L2 AVX2, ~20% faster than Ref):

```bash
cd Implementations/Optimized_Implementation/Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2
```

The optimized build accelerates `poly_mul_ntt` via AVX2 8-lane Montgomery reduction and basemul4 convolution kernel.

## Test Vectors

The final KAT files are stored in the top-level `Test_Vectors/` directory:

```
KAT_KEM_Lore-L1.txt
KAT_KEM_Lore-L2.txt
```

## Backend

- **KEM hash/XOF**: Pure Keccak/SHAKE FIPS202 (`fips202.c` + `symmetric-shake.c`)
- **No**: AES, SHA2, SM3, SM4 in the KEM algorithm hash/XOF path
- **SM3 in auxfunc.c/drng.c**: KAT infrastructure DRNG only, not the KEM algorithm hash backend

## Important

This repository does **not** commit fixed cycle-count result tables. Cycle counts must be measured by the runner on the target platform using `bench/run_lore_shake_measurements.sh`.
