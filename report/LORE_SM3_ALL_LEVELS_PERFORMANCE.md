# Lore-SM3 KEM Cycles and Time Benchmark

## Scope

- **Measured functions**: `crypto_kem_keypair`, `crypto_kem_enc`, `crypto_kem_dec`
- **Metrics**: CPU cycles (rdtsc) + wall-clock time (clock_gettime)
- **Build**: generic C, `-O3 -DNDEBUG`
- **Iterations**: 10000, **Warmup**: 1000
- **This benchmark does NOT measure PKE-level KeyGen / Enc / Dec.**

## Parameter Mapping

| Level | Paper name | Classical security | κ | n | k | t |
|---|---:|---:|---:|---:|---:|---:|
| Lore-L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| Lore-L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| Lore-L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| Lore-L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## KEM Cycles and Time

| Level | Paper | keygen med cycles | encaps med cycles | decaps med cycles | keygen us | encaps us | decaps us |
|---|---:|---:|---:|---:|---:|---:|---:|
| L1 | Lore-128 | 188260 | 305730 | 385274 | 65.08 | 105.67 | 133.16 |
| L2 | Lore-256 | 636468 | 801234 | 922050 | 219.97 | 276.91 | 318.66 |
| L3 | Lore-384 | 1377194 | 1608642 | 1866594 | 475.95 | 555.93 | 645.08 |
| L4 | Lore-512 | 2938994 | 3502896 | 3811132 | 1015.71 | 1210.57 | 1317.11 |

## Validation

- **KAT**: see `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/validation/kat_summary.csv`
- **SM3 path (sm3hash/pseudoXOF)**: YES
- **Failures**: ?

## Output Files

- `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/performance/lore_sm3_kem_cycles_time_all_levels.csv`
- `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/summary/`
- `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/logs/`
- `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/environment/`
- `/home/syh/Work/zj-cuc-lore-sm3/bench/results/20260611_180811/validation/`
