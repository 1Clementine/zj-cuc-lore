# Lore SM3 Report

SM3-interface version. Branch: SM3.

**KEM layer**: H = SM3, G = SM3-KDF, KDF = SM3-KDF.
SM3-KDF = SM3(input || counter_be), counter from 0x00000001.

## Files

| Path | Description |
|------|-------------|
| `FINAL_SIZE_TABLE.md` | 10000-trial actual size analysis |
| `performance/perf_latest.csv` | Latest performance CSV |
| `run_performance.sh` | One-click performance entry |
| `performance_tests/` | bench_kem.c + build/run scripts |
| `../Test_Vectors/` | KAT L1-L4 |

## Main Results

- KAT: L1-L4 Ref=Opt PASS.
- Size: `CRYPTO_*BYTES` / `LORE_*BYTES` are buffer maxima. Actual = pack `bytes_written` before padding.
  L4 CT: measured avg 2882.68 vs PDF 2886 (see FINAL_SIZE_TABLE.md).

## Re-run Performance

```bash
cd /home/syh/Work/zj-cuc-lore-sm3/report
bash run_performance.sh            # default 1000/100
bash run_performance.sh 200 20     # quick test
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
