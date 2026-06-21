# Lore KEM Implementations

This repository combines the Lore KEM SHAKE and SM3 implementation branches in a backend-separated layout.

- SHAKE backend: L1 and L2.
- SM3 backend: L1, L2, L3, and L4.
- Formal implementation files: `Implementations/`
- KAT files: `Test_Vectors/`
- Benchmark and size measurement scripts: `bench/`

## Quick Start

Run measurements through the unified wrapper:

```bash
./bench/run_lore_measurements.sh shake size
./bench/run_lore_measurements.sh shake cycles
./bench/run_lore_measurements.sh sm3 size
./bench/run_lore_measurements.sh sm3 cycles
```

Backend-specific scripts are also kept:

```bash
./bench/run_lore_shake_measurements.sh all
./bench/run_lore_sm3_measurements.sh all
```

Recommended full benchmark examples:

```bash
LEVELS="L1 L2" \
TRIALS=10000 \
ITERS=10000 \
WARMUP=1000 \
CORE=0 \
./bench/run_lore_measurements.sh shake all

LEVELS="L1 L2 L3 L4" \
TRIALS=10000 \
ITERS=10000 \
WARMUP=1000 \
CORE=0 \
./bench/run_lore_measurements.sh sm3 all
```

## Included Levels

| Backend | Level | Paper Name | Security | KAPPA | N | K | t |
|---|---:|---:|---:|---:|---:|---:|---:|
| SHAKE | L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| SHAKE | L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| SM3 | L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| SM3 | L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| SM3 | L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| SM3 | L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## Directory Structure

```text
Implementations/
    Reference_Implementation/
        SHAKE/
            Lore-L1/
            Lore-L2/
        SM3/
            Lore-L1/
            Lore-L2/
            Lore-L3/
            Lore-L4/
    Optimized_Implementation/
        SHAKE/
            Lore-L1/
            Lore-L2/
        SM3/
            Lore-L1/
            Lore-L2/
            Lore-L3/
            Lore-L4/

Test_Vectors/
    SHAKE/
        KAT_KEM_Lore-L1.txt
        KAT_KEM_Lore-L2.txt
    SM3/
        KAT_KEM_Lore-L1.txt
        KAT_KEM_Lore-L2.txt
        KAT_KEM_Lore-L3.txt
        KAT_KEM_Lore-L4.txt
```

## Main Files in Each Algorithm Instance

`KEM_AlgorithmInstance.c` and `KEM_AlgorithmInstance.h` provide the API_PKC bridge layer for KEM key generation, encapsulation, and decapsulation.

`KAT_KEM.c` is the API_PKC KAT generation program.

`drng.c` and `drng.h` provide the deterministic random number generator used by KAT programs.

`auxfunc.c` and `auxfunc.h` provide API_PKC auxiliary hash/XOF functions.

`fips202.c`, `fips202.h`, `symmetric-shake.c`, and `symmetric.h` adapt the backend hash/XOF interface used by Lore. SM3 instances additionally include `sm3.c` and `sm3.h`.

`kem.c`, `pke.c`, `indcpa.c`, `poly.c`, `polyvec.c`, `ntt.c`, `sampler.c`, `reduce.c`, `toomcook.c`, `verify.c`, and `bch_codec.c` contain the core Lore implementation.

## Build / Run Examples

Reference SHAKE:

```bash
cd Implementations/Reference_Implementation/SHAKE/Lore-L1
make clean
make KAT_KEM_1
./KAT_KEM_1

cd ../Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2
```

Reference SM3:

```bash
cd Implementations/Reference_Implementation/SM3/Lore-L1
make clean
make KAT_KEM_1
./KAT_KEM_1

cd ../Lore-L4
make clean
make KAT_KEM_4
./KAT_KEM_4
```

Optimized examples:

```bash
cd Implementations/Optimized_Implementation/SHAKE/Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2

cd ../../SM3/Lore-L2
make clean
make KAT_KEM_2
./KAT_KEM_2
```

## Test Vectors

KAT files are stored under backend-specific directories in `Test_Vectors/`.

Cycle-count result tables are intentionally not committed. Measure cycles on the target platform with `bench/run_lore_measurements.sh`.

## Notes

- Generated binaries such as `KAT_KEM_*`, object files, `obj/`, `output/`, generated benchmark sources, and benchmark result files are ignored by `.gitignore`.
- SHAKE and SM3 sources are intentionally separated by backend to avoid mixing files with the same names but different hash/XOF behavior.
