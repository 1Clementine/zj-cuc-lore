#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
RESULTS_DIR="$SCRIPT_DIR/results"

SHAKE_REPO="/home/syh/Work/zj-cuc-lore"
SM3_REPO="/home/syh/Work/zj-cuc-lore-sm3"

ITERATIONS="${1:-1000}"
WARMUP="${2:-100}"
MODE="${MODE:-generic}"

mkdir -p "$RESULTS_DIR"

echo "============================================"
echo "Lore KEM Performance Benchmark [MODE=$MODE]"
echo "iterations=$ITERATIONS  warmup=$WARMUP"
echo "SHAKE repo: $SHAKE_REPO"
echo "SM3   repo: $SM3_REPO"
echo "============================================"

CSV_FILE="$RESULTS_DIR/perf_$(date +%Y%m%d_%H%M%S).csv"
echo "version,impl,level,op,iterations,warmup,total_ns,avg_ns,ops_per_sec,pk_bytes,ct_bytes,sk_bytes,ss_bytes,failures" > "$CSV_FILE"

run_one() {
    local repo="$1" version="$2" impl="$3" level="$4"
    local bin="$SCRIPT_DIR/build/${version}_${impl}_L${level}/bench_kem"

    if [ ! -x "$bin" ]; then
        echo "SKIP: $bin not found" >&2
        return 0
    fi

    echo "Running: $version $impl L$level ..."
    "$bin" "$version" "$impl" "$level" "$ITERATIONS" "$WARMUP" >> "$CSV_FILE"
    local rc=$?
    if [ $rc -ne 0 ]; then
        echo "WARNING: $version $impl L$level exit=$rc (failures or mismatch)" >&2
    fi
}

for version in SHAKE SM3; do
    if [ "$version" = "SHAKE" ]; then repo="$SHAKE_REPO"; else repo="$SM3_REPO"; fi

    for impl in Reference Optimized; do
        for level in 1 2 3 4; do
            MODE="$MODE" bash "$SCRIPT_DIR/build_bench.sh" "$repo" "$version" "$impl" "$level"
            run_one "$repo" "$version" "$impl" "$level"
        done
    done
done

echo ""
echo "===== RESULTS ====="
cat "$CSV_FILE"
echo ""
echo "CSV saved to: $CSV_FILE"

echo ""
echo "===== SUMMARY ====="
awk -F, 'NR>1 {
    key=$1","$2","$3","$4;
    if(!(key in min)){min[key]=$9;max[key]=$9;sum[key]=0;cnt[key]=0}
    sum[key]+=$9; cnt[key]++;
    if($9<min[key])min[key]=$9; if($9>max[key])max[key]=$9
}
END{
    printf "%-40s %12s %12s %12s %12s\n", "version,impl,level,op", "avg_ns", "min_ns", "max_ns", "ops/sec"
    for(k in cnt) printf "%-40s %12.2f %12.2f %12.2f %12.2f\n", k, sum[k]/cnt[k], min[k], max[k], 1000000000/(sum[k]/cnt[k])
}' "$CSV_FILE" | sort

echo ""
echo "===== DECAPS CORRECTNESS ====="
grep -c "DECAPS MISMATCH" /dev/stderr 2>/dev/null || true
fail_lines=$(awk -F, 'NR>1 && $14>0 {print $1,$2,$3,$4,"failures="$14}' "$CSV_FILE")
if [ -z "$fail_lines" ]; then
    echo "ALL PASS: no decaps mismatches, no failures"
else
    echo "FAILURES DETECTED:"
    echo "$fail_lines"
fi
