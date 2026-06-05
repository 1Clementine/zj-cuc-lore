# Run Tests - SHAKE256 Version

This report belongs to the SHAKE/SHA3 version only.

## 1. View Existing Results

```bash
cd /home/syh/Work/zj-cuc-lore

cat report/README_SUMMARY.md
ls -lh report/KAT_SHAKE
ls -lh report/performance
cat report/code_checks/kat_shake_sizes.txt
```

KAT: `report/KAT_SHAKE/`
Performance CSV: `report/performance/`

## 2. Re-run KAT

Do not run plain `make`. Use only `KAT_KEM_${lvl}`.

```bash
cd /home/syh/Work/zj-cuc-lore

rm -rf /tmp/lore_shake_kat_outputs
mkdir -p /tmp/lore_shake_kat_outputs

for impl in Reference_Implementation Optimized_Implementation; do
  for lvl in 1 2 3 4; do
    d="Implementations/$impl/Lore-L${lvl}"
    cd "/home/syh/Work/zj-cuc-lore/$d" || exit 1

    make clean || true
    make KAT_KEM_${lvl}
    ./KAT_KEM_${lvl}

    f=$(find . -maxdepth 3 -name "KAT_KEM_Lore-L${lvl}.txt" | head -1)
    cp "$f" /tmp/lore_shake_kat_outputs/${impl}_L${lvl}_KAT_KEM_Lore-L${lvl}.txt
  done
done
```

Compare Reference and Optimized:

```bash
cd /tmp/lore_shake_kat_outputs

for lvl in 1 2 3 4; do
  diff -q Reference_Implementation_L${lvl}_KAT_KEM_Lore-L${lvl}.txt \
          Optimized_Implementation_L${lvl}_KAT_KEM_Lore-L${lvl}.txt \
    && echo "L$lvl PASS" || echo "L$lvl FAIL"
done
```

## 3. Re-run Performance

```bash
cd /home/syh/Work/zj-cuc-lore/report/performance_tests

bash run_perf.sh 1000 100   # full
bash run_perf.sh 200 20     # quick
```

After running, check failures:

```bash
for f in results/*.csv; do
  echo "===== $f ====="
  awk -F, 'NR==1 || $14 != 0 {print}' "$f"
done
```

If only the header is printed, failures are zero.
