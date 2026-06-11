# How to run Lore-SM3 KEM benchmark on a new server

## 1. Unpack

```bash
tar -xzf lore_sm3_bench_bundle_YYYYMMDD_HHMMSS.tar.gz
cd lore_sm3_bench_bundle_YYYYMMDD_HHMMSS
```

## 2. Check dependencies

Required:
- gcc
- make
- python3
- bash
- coreutils

Optional:
- taskset (for CPU pinning)

## 3. Run full benchmark

```bash
bash run_on_new_server.sh
```

This executes the full pipeline:
1. Capture environment info
2. Check source purity (pure SM3, no SM3 wrapper)
3. Run KAT for all 4 levels (Lore-L1 to Lore-L4)
4. Run KEM cycles+time benchmark (10000 iters, 1000 warmup)
5. Generate summary reports

## 4. Optional: CPU pinning

```bash
cd zj-cuc-lore
CPU_PIN=0 bash bench/run_full_lore_sm3_kem_bench.sh
```

## 5. Optional: Custom iterations

```bash
ITERATIONS=50000 WARMUP=5000 bash run_on_new_server.sh
```

## 6. Outputs

- `zj-cuc-lore/bench/results/latest/` — full results for the latest run
  - `environment/` — CPU, compiler, OS info
  - `validation/` — KAT output files + kat_summary.csv
  - `performance/` — lore_sm3_kem_cycles_time_all_levels.csv
  - `summary/` — LORE_SM3_KEM_CYCLES_TIME_SUMMARY.md
  - `logs/` — build and run logs
- `zj-cuc-lore/report/performance/lore_sm3_kem_cycles_time_all_levels.csv`
- `zj-cuc-lore/report/LORE_SM3_ALL_LEVELS_PERFORMANCE.md`

## 7. Notes

- This benchmark measures **KEM API-level** performance only.
- It calls `crypto_kem_keypair` / `crypto_kem_enc` / `crypto_kem_dec`.
- It does **not** measure PKE-level KeyGen/Enc/Dec.
- Results across machines should be compared using the recorded environment info.
- The SM3256 formal branch only retains Lore-L4. The all-levels worktree (`zj-cuc-lore-sm3-all-levels-bench/`) contains Lore-L1 through Lore-L4.
