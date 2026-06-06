#!/usr/bin/env bash
set -euo pipefail

REPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_DIR="$REPORT_DIR/performance_tests"
OUT_DIR="$REPORT_DIR/performance"

ITERS="${1:-1000}"
WARMUP="${2:-100}"

mkdir -p "$OUT_DIR"

echo "[INFO] Running performance: iters=$ITERS warmup=$WARMUP"

cd "$TEST_DIR"
bash run_perf.sh "$ITERS" "$WARMUP"

latest_csv="$(find "$TEST_DIR/results" "$OUT_DIR" -type f -name "*.csv" ! -name "perf_latest.csv" -printf "%T@ %p\n" 2>/dev/null | sort -nr | head -n 1 | cut -d' ' -f2-)"

if [ -n "${latest_csv:-}" ]; then
  cp "$latest_csv" "$OUT_DIR/perf_latest.csv"
  echo "[INFO] Updated: $OUT_DIR/perf_latest.csv"
else
  echo "[WARN] No CSV found; check performance_tests/run_perf.sh output"
fi

echo "[DONE]"
