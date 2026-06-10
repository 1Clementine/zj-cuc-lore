#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BENCH_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
RUN_ID="${1:-$(date +%Y%m%d_%H%M%S)}"
ENV_DIR="$BENCH_DIR/results/$RUN_ID/environment"

mkdir -p "$ENV_DIR"

{
  echo "=== Benchmark run: $RUN_ID ==="
  echo "date: $(date -Iseconds)"
  echo "hostname: $(hostname)"
  echo "uname: $(uname -a)"
  echo ""
  echo "=== CPU ==="
  lscpu 2>/dev/null || cat /proc/cpuinfo 2>/dev/null | head -40
  echo ""
  echo "=== Compiler ==="
  gcc --version 2>/dev/null | head -1 || echo "gcc not found"
  echo ""
  echo "=== Tools ==="
  echo "make: $(make --version 2>/dev/null | head -1 || echo 'not found')"
  echo "python3: $(python3 --version 2>/dev/null || echo 'not found')"
  echo "bash: $(bash --version 2>/dev/null | head -1)"
  echo "nproc: $(nproc)"
  echo ""
  echo "=== CPU governor (if available) ==="
  for f in /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor; do
    test -r "$f" && echo "$f: $(cat $f)" || true
  done
  echo ""
  echo "=== Turbo boost (if available) ==="
  for f in /sys/devices/system/cpu/intel_pstate/no_turbo /sys/devices/system/cpu/cpufreq/boost; do
    test -r "$f" && echo "$f: $(cat $f)" || true
  done
} > "$ENV_DIR/env.txt"

{
  echo "=== Benchmark config ==="
  echo "ITERATIONS=${ITERATIONS:-10000}"
  echo "WARMUP=${WARMUP:-1000}"
  echo "BUILD_MODE=${BUILD_MODE:-generic}"
  echo "CFLAGS=${CFLAGS:--O3 -DNDEBUG}"
  echo "CPU_PIN=${CPU_PIN:-}"
} > "$ENV_DIR/config.txt"

if command -v git &>/dev/null && [ -d "$LORE_FORMAL_REPO/.git" ]; then
  cd "$LORE_FORMAL_REPO"
  {
    echo "branch: $(git branch --show-current)"
    echo "commit: $(git rev-parse HEAD)"
    echo "status:"
    git status --short
  } > "$ENV_DIR/git_status.txt"
fi

echo "[environment] captured to $ENV_DIR/"
