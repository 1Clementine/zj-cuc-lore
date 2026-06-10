# Lore-SHAKE KEM Cycles and Time Benchmark

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
| L1 | Lore-128 | 87010 | 156578 | 210314 | 30.10 | 54.15 | 72.73 |
| L2 | Lore-256 | 245526 | 367888 | 435686 | 84.90 | 127.20 | 150.64 |
| L3 | Lore-384 | 556432 | 684582 | 847796 | 192.37 | 236.67 | 293.08 |
| L4 | Lore-512 | 949204 | 1242682 | 1504752 | 328.13 | 429.57 | 520.16 |

## Validation

- **KAT**: see `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/validation/kat_summary.csv`
- **Pure SHAKE**: YES
- **Failures**: ?

## Output Files

- `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/performance/lore_shake_kem_cycles_time_all_levels.csv`
- `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/summary/`
- `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/logs/`
- `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/environment/`
- `/home/syh/Work/zj-cuc-lore/bench/results/20260610_180833/validation/`
