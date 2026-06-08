# Lore SM3 Report

This report summarizes the official-submission-aligned SM3 version.

## Branch Role

- Branch: `SM3`
- Role: official-submission-aligned implementation
- Fixed hash: `sm3hash(256, ...)`
- Variable-length XOF/KDF: `pseudoXOF(...)`
- DRNG: official `drng.c/h`
- KAT: official `KAT_KEM.c` and `KEM_AlgorithmInstance.c/h`

The SHAKE256 branch is retained only as a comparison / legacy branch.

## Files

| Path | Description |
|---|---|
| `VALIDATION_REPORT.md` | Full validation report |
| `FINAL_SIZE_TABLE.md` | Actual serialized size statistics |
| `performance/perf_generic.csv` | Generic baseline performance |
| `performance/perf_native_avx2.csv` | Native / AVX2 performance |
| `run_performance.sh` | One-click performance test |
| `performance_tests/` | Benchmark source and helper scripts |
| `../Test_Vectors/` | Official-driver-generated KAT vectors |

## Main Status

| Check | Status |
|---|---|
| Official auxfunc alignment | PASS |
| Official KAT/API alignment | PASS |
| Reference pure C | PASS |
| Optimized AVX2 backend | PASS |
| Ref/Opt L1-L4 KAT | PASS |
| Ref/Opt vectors bit-identical | PASS |
| Performance tests | PASS |
| Size analysis | PASS with L4 CT caveat |

## Known Caveat

L4 ciphertext actual size differs from the paper table:

- measured actual average: 2882.68 bytes
- paper value: 2886 bytes

This is documented in `FINAL_SIZE_TABLE.md`.

## Re-run Performance

Generic baseline:

```bash
cd /home/syh/Work/zj-cuc-lore-sm3/report
MODE=generic bash run_performance.sh 10000 1000
```

Native / AVX2:

```bash
MODE=native bash run_performance.sh 10000 1000
```

## Re-run KAT

```bash
cd /home/syh/Work/zj-cuc-lore-sm3
for impl in Reference_Implementation Optimized_Implementation; do
  for lvl in 1 2 3 4; do
    d="Implementations/$impl/Lore-L${lvl}"
    (cd "$d" && make clean && make KAT_KEM_${lvl} && ./KAT_KEM_${lvl})
  done
done
```
