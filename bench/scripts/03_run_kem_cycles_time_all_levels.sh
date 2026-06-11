#!/usr/bin/env bash
set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RUN_ID="${1:-$(date +%Y%m%d_%H%M%S)}"
LOG_DIR="$BENCH_DIR/results/$RUN_ID/logs"
PERF_DIR="$BENCH_DIR/results/$RUN_ID/performance"
BUILD_DIR="$BENCH_DIR/results/$RUN_ID/build"
mkdir -p "$LOG_DIR" "$PERF_DIR" "$BUILD_DIR"

ITERATIONS="${ITERATIONS:-10000}"
WARMUP="${WARMUP:-1000}"
CFLAGS="${CFLAGS:--O3 -DNDEBUG}"
CPU_PIN="${CPU_PIN:-}"

BENCH_SRC="$LORE_SM3_REPO/report/performance_tests/bench_lore_sm3_kem_cycles_time.c"
CSV_OUT="$PERF_DIR/lore_sm3_kem_cycles_time_all_levels.csv"
echo "scheme,level,paper_name,classical_security,operation,iterations,warmup,avg_cycles,median_cycles,avg_us,median_us,pk_bytes,ct_bytes,sk_bytes,kappa,n,k,t,sm3_path,opt_status,failures" > "$CSV_OUT"

SRC_FILES="kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c symmetric-shake.c fips202.c toomcook.c verify.c bch_codec.c randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c sm3.c"

for L in 1 2 3 4; do
    D="$LORE_SM3_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    echo "===== KEM Benchmark L${L} ====="
    cd "$D"

    BIN="$BUILD_DIR/bench_kem_L${L}"
    gcc -O3 -DNDEBUG -std=gnu11 -Wall -Wextra -D_POSIX_C_SOURCE=199309L \
        -I. -DLORE_LEVEL=${L} -DLORE_USE_API_PKC_DRNG \
        "$BENCH_SRC" $SRC_FILES -o "$BIN" -lm \
        > "$LOG_DIR/bench_L${L}_build.log" 2>&1

    run_cmd="$BIN"
    if [ -n "$CPU_PIN" ]; then
        run_cmd="taskset -c $CPU_PIN $BIN"
    fi

    $run_cmd > "$LOG_DIR/bench_L${L}_run.log" 2>&1
    tail -3 "$LOG_DIR/bench_L${L}_run.log" >> "$CSV_OUT"
    echo "  Done."
done

# Also copy CSV to formal report directory
cp "$CSV_OUT" "$LORE_SM3_REPO/report/performance/lore_sm3_kem_cycles_time_all_levels.csv"

echo ""
echo "=== Benchmark CSV ==="
cat "$CSV_OUT"
