# Lore-SHAKE Minimal KEM Benchmark Summary

This repository snapshot is a minimal benchmark package for Lore-SHAKE KEM API performance.

## Benchmark Scope

This benchmark measures KEM API-level performance only:

- `crypto_kem_keypair`
- `crypto_kem_enc`
- `crypto_kem_dec`

Both CPU cycles and wall-clock time are reported.

This package does **not** measure PKE-level KeyGen / Enc / Dec.

## Security Mapping

| Level | Paper name | Classical security | κ | n | k | t |
|---|---:|---:|---:|---:|---:|---:|
| Lore-L1 | Lore-128 | 128-bit | 128 | 512 | 1 | 2 |
| Lore-L2 | Lore-256 | 256-bit | 256 | 512 | 2 | 2 |
| Lore-L3 | Lore-384 | 384-bit | 384 | 512 | 3 | 4 |
| Lore-L4 | Lore-512 | 512-bit | 512 | 768 | 3 | 4 |

## Main Files

- `bench/run_full_lore_shake_kem_bench.sh` — full benchmark entry
- `bench/package_lore_shake_bench_bundle.sh` — packaging script
- `bench/README_RUN_ON_NEW_SERVER.md` — new server instructions
- `report/LORE_SHAKE_ALL_LEVELS_PERFORMANCE.md` — results summary
- `report/performance/lore_shake_kem_cycles_time_all_levels.csv` — raw CSV
- `report/performance_tests/bench_lore_shake_kem_cycles_time.c` — benchmark source

## Run

```bash
bash bench/run_full_lore_shake_kem_bench.sh
```

Optional CPU pinning:

```bash
CPU_PIN=0 bash bench/run_full_lore_shake_kem_bench.sh
```

## Package

```bash
bash bench/package_lore_shake_bench_bundle.sh
```
