# Lore-SHAKE Minimal KEM Benchmark Package

This repository is a minimal benchmark package for testing Lore-SHAKE KEM API performance.

The benchmark measures only the KEM-level API:

- `crypto_kem_keypair`
- `crypto_kem_enc`
- `crypto_kem_dec`

It reports both:

- CPU cycles
- wall-clock time in microseconds

This package does **not** benchmark PKE-level `KeyGen / Enc / Dec`.

---

## 1. Repository Layout

```text
zj-cuc-lore/
├── bench/
│   ├── config/
│   │   ├── lore_shake_bench_config.sh
│   │   └── lore_shake_levels.csv
│   ├── scripts/
│   │   ├── 00_capture_environment.sh
│   │   ├── 01_check_sources.sh
│   │   ├── 02_run_kat_all_levels.sh
│   │   ├── 03_run_kem_cycles_time_all_levels.sh
│   │   └── 04_summarize_results.py
│   ├── run_full_lore_shake_kem_bench.sh
│   ├── package_lore_shake_bench_bundle.sh
│   └── README_RUN_ON_NEW_SERVER.md
├── Implementations/
│   └── Reference_Implementation/
│       └── Lore-L4/
├── report/
│   ├── README_SUMMARY.md
│   ├── LORE_SHAKE_ALL_LEVELS_PERFORMANCE.md
│   ├── performance/
│   │   └── lore_shake_kem_cycles_time_all_levels.csv
│   └── performance_tests/
│       └── bench_lore_shake_kem_cycles_time.c
└── Test_Vectors/
    └── KAT_KEM_Lore-L4.txt
```

The formal repository keeps only the minimal submitted implementation snapshot.
All-level benchmark sources are expected to be available through the benchmark worktree or the packaged bundle.

---

## 2. Security-Level Mapping

| Level   | Paper name | Classical security | kappa |   n |  k |  t |
| ------- | ---------- | -----------------: | ----: | --: | -: | -: |
| Lore-L1 | Lore-128   |            128-bit |   128 | 512 |  1 |  2 |
| Lore-L2 | Lore-256   |            256-bit |   256 | 512 |  2 |  2 |
| Lore-L3 | Lore-384   |            384-bit |   384 | 512 |  3 |  4 |
| Lore-L4 | Lore-512   |            512-bit |   512 | 768 |  3 |  4 |

---

## 3. What This Benchmark Measures

| Operation          | Function             |
| ------------------ | -------------------- |
| KEM key generation | `crypto_kem_keypair` |
| KEM encapsulation  | `crypto_kem_enc`     |
| KEM decapsulation  | `crypto_kem_dec`     |

Each operation reports: median cycles, average cycles, median microseconds, average microseconds.

Default configuration:

```bash
ITERATIONS=10000
WARMUP=1000
CFLAGS="-O3 -DNDEBUG"
BUILD_MODE=generic
```

The benchmark uses generic C flags by default. It does not use `-march=native`, AVX, AES, or SHA-specific instruction flags.

---

## 4. Quick Start

```bash
bash bench/run_full_lore_shake_kem_bench.sh
```

Optional CPU pinning:

```bash
CPU_PIN=0 bash bench/run_full_lore_shake_kem_bench.sh
```

The script will: record environment, check sources and SHAKE purity, run KAT for L1-L4, run KEM cycles/time benchmark, generate CSV and Markdown summary.

---

## 5. Output Locations

```
bench/results/latest/              (symlink to latest run)
bench/results/<timestamp>/
  environment/                     CPU, compiler, OS info
  logs/                            build and run logs
  validation/                      KAT output + kat_summary.csv
  performance/                     lore_shake_kem_cycles_time_all_levels.csv
  summary/                         LORE_SHAKE_KEM_CYCLES_TIME_SUMMARY.md
report/performance/lore_shake_kem_cycles_time_all_levels.csv
report/LORE_SHAKE_ALL_LEVELS_PERFORMANCE.md
```

---

## 6. Packaging for a New Server

```bash
bash bench/package_lore_shake_bench_bundle.sh
```

Output: `/home/syh/Work/lore_shake_bench_bundle_<timestamp>.tar.gz`

The bundle contains: formal repo, all-levels worktree, manifest, and a `run_on_new_server.sh` entry script.

---

## 7. Running on a New Server

```bash
tar -xzf lore_shake_bench_bundle_<timestamp>.tar.gz
cd lore_shake_bench_bundle_<timestamp>
bash run_on_new_server.sh
```

---

## 8. Dependencies

Required: `bash`, `gcc`, `make`, `python3`, `coreutils`
Optional: `taskset` (CPU pinning), `lscpu` (environment capture)

---

## 9. Notes

This package is intentionally minimal. It does not include: historical ML-KEM comparison reports, PKE-level benchmarks, liboqs artifacts, profiling artifacts, old optimization logs, or old validation outputs.

Benchmark results are machine-dependent. When comparing across servers, use the recorded environment files under `bench/results/latest/environment/`.

Do not commit runtime artifacts: `bench/results/`, `*.o`, `build/`, `*/output/`, `KAT_KEM_*` binaries.
