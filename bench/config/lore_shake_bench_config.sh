# Lore-SHAKE KEM benchmark configuration
# Source this file before running benchmarks, or edit defaults here.

ITERATIONS="${ITERATIONS:-10000}"
WARMUP="${WARMUP:-1000}"
BUILD_MODE="${BUILD_MODE:-generic}"
CFLAGS="${CFLAGS:--O3 -DNDEBUG}"
CPU_PIN="${CPU_PIN:-}"
RUN_KAT="${RUN_KAT:-1}"
RUN_BENCH="${RUN_BENCH:-1}"

# Repo paths (override via environment on new server)
LORE_FORMAL_REPO="${LORE_FORMAL_REPO:-$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)}"
LORE_ALL_LEVELS_REPO="${LORE_ALL_LEVELS_REPO:-${LORE_FORMAL_REPO}/../zj-cuc-lore-shake-all-levels-bench}"
