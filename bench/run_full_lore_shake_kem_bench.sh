#!/usr/bin/env bash
set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
SCRIPT_DIR="$BENCH_DIR/scripts"

# Load config (allow overrides from environment)
if [ -f "$BENCH_DIR/config/lore_shake_bench_config.sh" ]; then
    source "$BENCH_DIR/config/lore_shake_bench_config.sh"
fi

# Set default paths if not already set
export LORE_FORMAL_REPO="${LORE_FORMAL_REPO:-$(cd "$BENCH_DIR/.." && pwd)}"
export LORE_ALL_LEVELS_REPO="${LORE_ALL_LEVELS_REPO:-${LORE_FORMAL_REPO}/../zj-cuc-lore-shake-all-levels-bench}"
export BENCH_DIR="$BENCH_DIR"

RUN_ID="${1:-$(date +%Y%m%d_%H%M%S)}"

echo "============================================"
echo " Lore-SHAKE KEM Benchmark"
echo " RUN_ID: $RUN_ID"
echo " LORE_FORMAL_REPO: $LORE_FORMAL_REPO"
echo " LORE_ALL_LEVELS_REPO: $LORE_ALL_LEVELS_REPO"
echo " ITERATIONS: ${ITERATIONS:-10000}"
echo " WARMUP: ${WARMUP:-1000}"
echo "============================================"

# Create symlink
mkdir -p "$BENCH_DIR/results"
ln -sfn "$RUN_ID" "$BENCH_DIR/results/latest"

export ITERATIONS WARMUP CFLAGS CPU_PIN RUN_KAT RUN_BENCH

echo ""
echo "=== Step 00: Capture environment ==="
bash "$SCRIPT_DIR/00_capture_environment.sh" "$RUN_ID"

echo ""
echo "=== Step 01: Check sources ==="
bash "$SCRIPT_DIR/01_check_sources.sh"

if [ "${RUN_KAT:-1}" = "1" ]; then
    echo ""
    echo "=== Step 02: Run KAT ==="
    bash "$SCRIPT_DIR/02_run_kat_all_levels.sh" "$RUN_ID"
fi

if [ "${RUN_BENCH:-1}" = "1" ]; then
    echo ""
    echo "=== Step 03: Run KEM cycles+time benchmark ==="
    bash "$SCRIPT_DIR/03_run_kem_cycles_time_all_levels.sh" "$RUN_ID"

    echo ""
    echo "=== Step 04: Summarize results ==="
    python3 "$SCRIPT_DIR/04_summarize_results.py" "$RUN_ID"
fi

echo ""
echo "============================================"
echo " DONE"
echo " RUN_ID: $RUN_ID"
echo " Results: $BENCH_DIR/results/$RUN_ID/"
echo " CSV:     $BENCH_DIR/results/$RUN_ID/performance/lore_shake_kem_cycles_time_all_levels.csv"
echo " Summary: $BENCH_DIR/results/$RUN_ID/summary/"
echo " Logs:    $BENCH_DIR/results/$RUN_ID/logs/"
echo "============================================"
