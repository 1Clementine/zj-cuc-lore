# Lore-SM3 Implementation

This repository contains Lore-SM3 implementations. SM3 keeps all four parameter sets (L1–L4) and includes an optimized AVX2 implementation.

- Formal implementation files: `Implementations/`
- KAT files: `Test_Vectors/`
- Benchmark helper: `bench/`

## Quick Start

```bash
# Default (L1 + L2 + L3 + L4):
./bench/run_lore_sm3_measurements.sh size      # actual serialized sizes
./bench/run_lore_sm3_measurements.sh cycles    # KEM cycles + time
./bench/run_lore_sm3_measurements.sh all       # both
```

Recommended full benchmark:

```bash
LEVELS="1 2 3 4" \
TRIALS=10000 \
ITERS=10000 \
WARMUP=1000 \
CORE=0 \
./bench/run_lore_sm3_measurements.sh all
```

LEVELS also accepts `L1 L2 L3 L4` format:

```bash
LEVELS="L1 L2 L3 L4" TRIALS=10000 ITERS=10000 ./bench/run_lore_sm3_measurements.sh all
```

## Included Levels

| Level | Paper Name | Security | KAPPA | N | K | t |
|---|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## Directory Structure

```
Reference_Implementation/
    Platform-independent reference implementation of Lore.
    Each algorithm instance is placed in an individual subdirectory.

    Lore-L1/
        Reference implementation of the Lore-L1 KEM instance.

    Lore-L2/
        Reference implementation of the Lore-L2 KEM instance.

    Lore-L3/
        Reference implementation of the Lore-L3 KEM instance.

    Lore-L4/
        Reference implementation of the Lore-L4 KEM instance.

Optimized_Implementation/
    Optimized implementation of Lore (partial AVX2).
    Each algorithm instance is placed in an individual subdirectory and keeps
    the same API_PKC interface as the reference implementation.

    Lore-L1/
        Optimized implementation of the Lore-L1 KEM instance.

    Lore-L2/
        Optimized implementation of the Lore-L2 KEM instance.

    Lore-L3/
        Optimized implementation of the Lore-L3 KEM instance.

    Lore-L4/
        Optimized implementation of the Lore-L4 KEM instance.

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

**sm3.c / sm3.h**
: SM3 hash implementation used as the hash/XOF backend for the KEM algorithm.

**avx2_utils.h** (Optimized only)
: AVX2 intrinsic wrappers for `poly_reduce` and `poly_add` acceleration.

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

cd ../Lore-L3
make clean
make KAT_KEM_3
./KAT_KEM_3

cd ../Lore-L4
make clean
make KAT_KEM_4
./KAT_KEM_4
```

Optimized implementation (requires AVX2):

```bash
cd Implementations/Optimized_Implementation/Lore-L1
make clean
make KAT_KEM_1
./KAT_KEM_1

cd ../Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2

cd ../Lore-L3
make clean
make KAT_KEM_3
./KAT_KEM_3

cd ../Lore-L4
make clean
make KAT_KEM_4
./KAT_KEM_4
```

## Test Vectors

The final KAT files are stored in the top-level `Test_Vectors/` directory:

```
KAT_KEM_Lore-L1.txt
KAT_KEM_Lore-L2.txt
KAT_KEM_Lore-L3.txt
KAT_KEM_Lore-L4.txt
```

## Backend

- **KEM hash/XOF**: SM3 (`sm3.c` + `sm3.h`) — SM3-based KDF and hash
- **Optimized Implementation**: Partial AVX2 acceleration in `poly_reduce` and `poly_add` (`-DLORE_USE_AVX2 -mavx2`)

## Important

This repository does **not** commit fixed cycle-count result tables. Cycle counts must be measured by the runner on the target platform using `bench/run_lore_sm3_measurements.sh`. Generated outputs are written under `bench/results/` and ignored by git.
