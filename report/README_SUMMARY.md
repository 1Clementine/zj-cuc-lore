# Lore-SM3 Benchmark Summary

## Scope

Measures KEM API performance: `crypto_kem_keypair` / `crypto_kem_enc` / `crypto_kem_dec`.
Reports CPU cycles + wall-clock time. Generic build, `-O3 -DNDEBUG`, 10000 iters, 1000 warmup.

## Files

- `bench/run_full_lore_sm3_kem_bench.sh` — full benchmark entry
- `report/SM3_KEM_ALL_LEVELS_PERFORMANCE.md` — results summary
- `report/performance/lore_sm3_kem_cycles_time_all_levels.csv` — raw CSV
- `report/performance_tests/bench_lore_sm3_kem_cycles_time.c` — benchmark source

## Run

```bash
bash bench/run_full_lore_sm3_kem_bench.sh
```
