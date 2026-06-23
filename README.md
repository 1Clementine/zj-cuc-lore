# Lore KEM Implementations

Lore is a post-quantum Key Encapsulation Mechanism (KEM) based on structured
lattices over Z_257 with sparse, fixed-weight secret keys, submitted to the NGCC
(Next-generation Commercial Cryptographic Algorithms) evaluation program.

- Hash backends: SHAKE (SHA3-256 / SHAKE256) and SM3 (Chinese national standard)
- Security levels: L1 (128-bit), L2 (256-bit), L3 (384-bit), L4 (512-bit)
- Platform variants: reference portable C, AVX2-optimized (x86-64), NEON-optimized (ARM64)
- Formal implementation files: `Implementations/`
- Known-Answer Test vectors: `Test_Vectors/`
- Benchmark scripts: `bench/`

## Included Levels

| Backend | Level | Security | κ (KAPPA) | N   | k | t | HWT (±) |
|---------|-------|----------|-----------|-----|---|---|---------|
| SHAKE   | L1    | 128-bit  | 128       | 512 | 1 | 2 | 50+50   |
| SHAKE   | L2    | 256-bit  | 256       | 512 | 2 | 2 | —       |
| SHAKE   | L3    | 384-bit  | 384       | 512 | 3 | 4 | —       |
| SHAKE   | L4    | 512-bit  | 512       | 768 | 3 | 4 | —       |
| SM3     | L1    | 128-bit  | 128       | 512 | 1 | 2 | 50+50   |
| SM3     | L2    | 256-bit  | 256       | 512 | 2 | 2 | —       |
| SM3     | L3    | 384-bit  | 384       | 512 | 3 | 4 | —       |
| SM3     | L4    | 512-bit  | 512       | 768 | 3 | 4 | —       |

L1 uses no BCH error correction. L2–L4 use BCH codes for message encoding.

## Directory Structure

```text
Implementations/
    Reference_Implementation/
        Lore-SHAKE/          # portable C, SHAKE backend
            Lore-L1/
            Lore-L2/
            Lore-L3/
            Lore-L4/
        Lore-SM3/            # portable C, SM3 backend
            Lore-L1/
            Lore-L2/
            Lore-L3/
            Lore-L4/
    Optimized_Implementation/
        AVX/                 # x86-64 AVX2 optimized
            Lore-SHAKE-AVX/
                Lore-L1/
                Lore-L2/
                Lore-L3/
                Lore-L4/
            Lore-SM3-AVX/
                Lore-L1/
                Lore-L2/
                Lore-L3/
                Lore-L4/
        NEON/                # ARM64 NEON optimized
            Lore-SHAKE-NEON/
                Lore-L1/
                Lore-L2/
                Lore-L3/
                Lore-L4/
            Lore-SM3-NEON/
                Lore-L1/
                Lore-L2/
                Lore-L3/
                Lore-L4/

Test_Vectors/
    Lore-SHAKE/
        KAT_KEM_Lore-L1.txt
        KAT_KEM_Lore-L2.txt
        KAT_KEM_Lore-L3.txt
        KAT_KEM_Lore-L4.txt
    Lore-SM3/
        KAT_KEM_Lore-L1.txt
        KAT_KEM_Lore-L2.txt
        KAT_KEM_Lore-L3.txt
        KAT_KEM_Lore-L4.txt
```

## Source File Overview

Each algorithm instance directory contains the following files:

| File | Description |
|------|-------------|
| `kem.c` / `kem.h` | Top-level KEM API: `crypto_kem_keypair`, `crypto_kem_enc`, `crypto_kem_dec` |
| `pke.c` / `pke.h` | Public-key encryption layer |
| `indcpa.c` / `indcpa.h` | IND-CPA core: key generation, encryption, decryption |
| `poly.c` / `poly.h` | Polynomial arithmetic over Z_q and Z_t; NTT wrapper |
| `polyvec.c` / `polyvec.h` | Polynomial vector operations |
| `ntt.c` / `ntt.h` | Number-Theoretic Transform over Z_257 (Fermat prime) |
| `toomcook.c` / `toomcook.h` | Toom-Cook-3 + Karatsuba polynomial multiplication |
| `reduce.c` / `reduce.h` | Barrett reduction, Montgomery reduction, mod-t/mod-q helpers |
| `sampler.c` / `sampler.h` | Fixed-weight sampling via Fisher-Yates shuffle using PRF |
| `bch_codec.c` / `bch_codec.h` | BCH error-correcting code for message encoding (L2–L4 only) |
| `verify.c` / `verify.h` | Constant-time comparison (`verify`) and conditional move (`cmov`) |
| `fips202.c` / `fips202.h` | Keccak / SHAKE reference (SHAKE variants only) |
| `symmetric-shake.c` / `symmetric.h` | SHAKE hash/XOF backend adapter |
| `sm3.c`, `sm3_xof.c`, `symmetric-sm3.c` | SM3 hash + XOF + backend adapter (SM3 variants only) |
| `auxfunc.c` / `auxfunc.h` | API_PKC auxiliary hash/XOF bridge (SM3 variants) |
| `KEM_AlgorithmInstance.c` / `.h` | API_PKC submission bridge for KEM |
| `KAT_KEM.c` | Known-Answer Test generation program |
| `drng.c` / `drng.h` | Deterministic RNG for reproducible KAT output |
| `randombytes.c` / `randombytes.h` | OS entropy source (production use) |
| `params.h` | Compile-time parameters selected by `-DLORE_LEVEL=N` |
| `api.h` | Public KEM API size constants |

Optimized implementations add a `simd/` subdirectory with platform-specific
accelerator files (see SIMD Optimization section below).

## SIMD Optimization Details

### AVX2 (x86-64) — `Optimized_Implementation/AVX/`

Enabled automatically on x86-64; controlled by `LORE_USE_AVX2=1` (default).

| File | Function(s) | Instruction set | Description |
|------|-------------|-----------------|-------------|
| `simd/shake_avx2.c` | `lore_shake128x4_absorb_once_squeezeblocks()` | AVX2 (`__m256i`) | 4-way parallel Keccak-f\[1600\] permutation; processes 4 SHAKE streams simultaneously using 256-bit SIMD lanes |
| `simd/poly_mul_ntt_avx2.c` | `poly_mul_ntt_avx2()` | AVX2 (`__m256i`) | 8-way NTT-domain polynomial multiplication; Montgomery reduction 8 coefficients at a time with `fqmul_8way` |
| `simd/sm3_avx2.c` | `lore_sm3_hash_avx2()`, `lore_sm3_pseudoXOF_avx2()` | AVX2 (`__m256i`) | 8-way parallel SM3 compression; processes 8 independent message blocks simultaneously |

**Compilation flags:**

| Macro | Default (x86-64) | Effect |
|-------|-----------------|--------|
| `LORE_USE_AVX2_SHAKE` | on | Enables 4-way SHAKE via `shake_avx2.c`; adds `-mavx2` |
| `LORE_USE_AVX2_SM3` | on | Enables 8-way SM3 via `sm3_avx2.c`; adds `-mavx2` |
| `LORE_USE_AVX2_POLYMUL_NTT` | off (opt-in) | Enables AVX2 NTT poly multiply via `poly_mul_ntt_avx2.c` |

Enable NTT poly multiply explicitly:
```bash
make kat_api LORE_USE_AVX2_POLYMUL_NTT=1
```

### NEON (ARM64) — `Optimized_Implementation/NEON/`

Enabled automatically on arm64/aarch64; controlled by `LORE_USE_NEON=1` (default).

| File | Function(s) | Instruction set | Description |
|------|-------------|-----------------|-------------|
| `simd/shake_neon.c` | `lore_shake128x2_absorb_once_squeezeblocks()` | NEON (`uint64x2_t`) | 2-way parallel Keccak-f\[1600\]; processes 2 SHAKE streams using 128-bit SIMD pairs |
| `simd/sm3_neon.c` | `lore_sm3_hash_neon()`, `lore_sm3_pseudoXOF_neon()` | NEON (`uint32x4_t`) | 4-way parallel SM3 compression; `p0`/`p1` permutations and message schedule vectorized |
| `poly.c` (inline, `#ifdef LORE_USE_NEON_CORE`) | `poly_add()` | NEON (`int16x8_t`) | 8-coefficient-wide vectorized polynomial addition with Barrett reduction via `lore_barrett_reduce_s16x8` |

**Compilation flags:**

| Macro | Default (arm64) | Effect |
|-------|----------------|--------|
| `LORE_USE_NEON` | on | Master NEON enable flag |
| `LORE_USE_NEON_SM3` | on | Enables 4-way SM3 via `sm3_neon.c` |
| `LORE_USE_NEON_SHAKE` | **off** (opt-in) | Enables 2-way SHAKE; **disabled by default** because scalar SHAKE is faster on tested ARM64 targets |
| `LORE_USE_NEON_CORE` | **off** (opt-in) | Enables vectorized `poly_add` in `poly.c` |

Enable all NEON paths:
```bash
make kat_api LORE_USE_NEON_SHAKE=1 LORE_USE_NEON_CORE=1
```

### Call Paths

```
crypto_kem_enc / crypto_kem_dec
  └─ indcpa_enc / indcpa_dec
       └─ polyvec_basemul_acc_montgomery
            └─ poly_basemul_montgomery
                 └─ poly_mul_ntt (NTT domain)
                      ├─ [AVX2]  poly_mul_ntt_avx2()       (8-way, simd/)
                      └─ [ref]   poly_mul_acc()             (Toom-Cook-3 / Karatsuba)

  └─ hash functions (keypair / enc / dec)
       ├─ [SHAKE-AVX2]  lore_shake128x4_absorb_once_squeezeblocks()
       ├─ [SHAKE-NEON]  lore_shake128x2_absorb_once_squeezeblocks()
       ├─ [SHAKE-ref]   scalar SHAKE256 via fips202.c
       ├─ [SM3-AVX2]    lore_sm3_hash_avx2() / lore_sm3_pseudoXOF_avx2()
       ├─ [SM3-NEON]    lore_sm3_hash_neon() / lore_sm3_pseudoXOF_neon()
       └─ [SM3-ref]     scalar SM3 via sm3.c / sm3_xof.c
```

## Build / Run Examples

### Reference implementations

```bash
# SHAKE, all 4 levels
for L in 1 2 3 4; do
  cd Implementations/Reference_Implementation/Lore-SHAKE/Lore-L$L
  make clean && make kat_api && ./KAT_KEM_$L
  cd -
done

# SM3, all 4 levels
for L in 1 2 3 4; do
  cd Implementations/Reference_Implementation/Lore-SM3/Lore-L$L
  make clean && make kat_api && ./KAT_KEM_$L
  cd -
done
```

### AVX2-optimized (x86-64)

```bash
# SHAKE-AVX (L1 only currently)
cd Implementations/Optimized_Implementation/AVX/Lore-SHAKE-AVX/Lore-L1
make clean && make kat_api && ./KAT_KEM_1

# SM3-AVX, all 4 levels (default: AVX2 SM3 enabled automatically on x86-64)
for L in 1 2 3 4; do
  cd Implementations/Optimized_Implementation/AVX/Lore-SM3-AVX/Lore-L$L
  make clean && make kat_api && ./KAT_KEM_$L
  cd -
done

# SM3-AVX with NTT poly multiply also enabled
make clean && make kat_api LORE_USE_AVX2_POLYMUL_NTT=1 && ./KAT_KEM_1
```

### NEON-optimized (ARM64)

```bash
# SHAKE-NEON, all 4 levels (scalar SHAKE is default; NEON SHAKE is opt-in)
for L in 1 2 3 4; do
  cd Implementations/Optimized_Implementation/NEON/Lore-SHAKE-NEON/Lore-L$L
  make clean && make kat_api && ./KAT_KEM_$L
  cd -
done

# SHAKE-NEON with vectorized SHAKE and poly_add enabled
make clean && make kat_api LORE_USE_NEON_SHAKE=1 LORE_USE_NEON_CORE=1 && ./KAT_KEM_1

# SM3-NEON, all 4 levels (NEON SM3 enabled automatically on arm64)
for L in 1 2 3 4; do
  cd Implementations/Optimized_Implementation/NEON/Lore-SM3-NEON/Lore-L$L
  make clean && make kat_api && ./KAT_KEM_$L
  cd -
done
```

### Benchmarks

```bash
./bench/run_lore_measurements.sh shake size
./bench/run_lore_measurements.sh shake cycles
./bench/run_lore_measurements.sh sm3 size
./bench/run_lore_measurements.sh sm3 cycles

# Full benchmark (all levels, 10 000 trials)
LEVELS="L1 L2 L3 L4" TRIALS=10000 ITERS=10000 WARMUP=1000 CORE=0 \
  ./bench/run_lore_measurements.sh shake all

LEVELS="L1 L2 L3 L4" TRIALS=10000 ITERS=10000 WARMUP=1000 CORE=0 \
  ./bench/run_lore_measurements.sh sm3 all
```

## Security Notes

The following hardening measures are applied across all implementations:

- **Key material zeroization** (`kem.c`): all intermediate secret buffers
  (`coins`, `kr`, `buf`, `kdf_buf`, `mu`, `ss_valid`, `ss_invalid`) are
  cleared with `secure_zero()` (volatile-pointer loop, immune to compiler
  Dead Store Elimination) before the function returns.
- **Implicit rejection** (`kem.c`): decapsulation copies `ss_invalid` to `ss`
  first, then conditionally overwrites it with `ss_valid` on success — this
  ensures the final assignment is not data-dependent on the decryption result
  from the compiler's perspective.
- **Sampler abort** (`sampler.c`): PRF-buffer exhaustion during fixed-weight
  sampling calls `abort()` rather than silently returning a partial result,
  which would produce a weaker secret key.
- **Polynomial initialization** (`poly.c`): `poly_getnoise_uniform` clears
  the output coefficient array with `memset` before the sampling loop so no
  uninitialized data can leak through early-exit paths.
- **Memory leak fixes** (`auxfunc.c`): all error paths in
  `pseudohash_512/768/1024` free previously allocated buffers before
  returning an error code.
- **Constant-time utilities** (`verify.c`): `cmov` uses a GCC/Clang inline
  assembly barrier `__asm__("" : "+r"(b) : )` to prevent the compiler from
  optimizing the conditional into a branch.

## Test Vectors

KAT files are stored under backend-specific directories in `Test_Vectors/`.
Generated output from `./KAT_KEM_N` must match the corresponding
`KAT_KEM_Lore-LN.txt` exactly for the implementation to be considered correct.

Cycle-count result tables are not committed. Measure on the target platform
with `bench/run_lore_measurements.sh`.

## Notes

- Generated binaries (`KAT_KEM_*`), object files, `obj/`, `output/`,
  and benchmark result files are excluded by `.gitignore`.
- SHAKE and SM3 sources are kept in separate directories because files such
  as `symmetric-shake.c` and `symmetric-sm3.c` share the same public API
  (`symmetric.h`) but have different implementations.
- The `LORE_NAMESPACE(s)` macro in `params.h` prefixes all exported symbols
  to allow linking multiple levels in the same binary during testing.
