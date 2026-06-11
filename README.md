# Lore-SM3 KEM Benchmark Package

Official-submission-aligned SM3 interface version. This is a minimal benchmark package for Lore-SM3 KEM API performance.

The benchmark measures:
- `crypto_kem_keypair`
- `crypto_kem_enc`
- `crypto_kem_dec`

It reports both CPU cycles and wall-clock time. Does not benchmark PKE-level KeyGen/Enc/Dec.

## Run

```bash
bash bench/run_full_lore_sm3_kem_bench.sh
```

## Output

```
bench/results/latest/
report/performance/lore_sm3_kem_cycles_time_all_levels.csv
report/SM3_KEM_ALL_LEVELS_PERFORMANCE.md
```

## Security Mapping

| Level   | Paper name | Classical security |   κ |   n |  k |  t |
| ------- | ---------- | -----------------: | --: | --: | -: | -: |
| Lore-L1 | Lore-128   |            128-bit | 128 | 512 |  1 |  2 |
| Lore-L2 | Lore-256   |            256-bit | 256 | 512 |  2 |  2 |
| Lore-L3 | Lore-384   |            384-bit | 384 | 512 |  3 |  4 |
| Lore-L4 | Lore-512   |            512-bit | 512 | 768 |  3 |  4 |

## Package for New Server

```bash
bash bench/package_lore_sm3_bench_bundle.sh
```
