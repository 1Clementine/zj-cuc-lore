# Lore SHAKE256 Report

SHAKE/SHA3-interface version. Branch: SHAKE256.

## Branch Role

This is a **comparison / legacy** branch. The SM3 branch is the official-submission-aligned version.

**KEM layer**: H = SHA3-256, G = SHAKE256, KDF = SHAKE256.

## Files

| Path | Description |
|------|-------------|
| `FINAL_SIZE_TABLE.md` | 10000-trial actual size analysis |
| `performance/perf_comparison.csv` | Performance data for SM3 comparison |
| `run_performance.sh` | One-click performance entry |
| `performance_tests/` | bench_kem.c + build/run scripts (identical to SM3) |
| `../Test_Vectors/` | KAT L1-L4 |

## Performance Build Modes

| Mode | Flags | Purpose |
|------|-------|---------|
| generic | `-O3 -DNDEBUG -std=gnu11 -Wall -Wextra`, no native/SIMD | fair baseline comparison |
| native | generic + `-march=native` (x86_64) / `-mcpu=native` (aarch64) | local optimized |

Performance test scripts are identical to the SM3 branch.

## Usage

```bash
cd /home/syh/Work/zj-cuc-lore/report

# Generic (default, fair comparison)
bash run_performance.sh 10000 1000

# Native (optional)
MODE=native bash run_performance.sh 10000 1000
```

## Re-run KAT

```bash
cd /home/syh/Work/zj-cuc-lore
for impl in Reference_Implementation Optimized_Implementation; do
  for lvl in 1 2 3 4; do
    d="Implementations/$impl/Lore-L${lvl}"
    (cd "$d" && make clean && make KAT_KEM_${lvl} && ./KAT_KEM_${lvl})
  done
done
```

## Notes

- `CRYPTO_*BYTES` / `LORE_*BYTES` are buffer maxima. Actual = pack `bytes_written` before padding.
- L4 CT: measured avg 2882.68 vs PDF 2886 (see FINAL_SIZE_TABLE.md).
- SHAKE256 is not official-submission-aligned (does not use API_PKC auxfunc/drng/KAT driver).
