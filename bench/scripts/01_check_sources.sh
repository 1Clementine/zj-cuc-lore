#!/usr/bin/env bash
set -euo pipefail

LORE_FORMAL_REPO="${LORE_FORMAL_REPO:-}"
LORE_ALL_LEVELS_REPO="${LORE_ALL_LEVELS_REPO:-}"

if [ -z "$LORE_FORMAL_REPO" ] || [ ! -d "$LORE_FORMAL_REPO" ]; then
    echo "[ERROR] LORE_FORMAL_REPO not set or not found: ${LORE_FORMAL_REPO:-unset}"
    exit 1
fi
if [ -z "$LORE_ALL_LEVELS_REPO" ] || [ ! -d "$LORE_ALL_LEVELS_REPO" ]; then
    echo "[ERROR] LORE_ALL_LEVELS_REPO not set or not found: ${LORE_ALL_LEVELS_REPO:-unset}"
    exit 1
fi

echo "[check] LORE_FORMAL_REPO=$LORE_FORMAL_REPO"
echo "[check] LORE_ALL_LEVELS_REPO=$LORE_ALL_LEVELS_REPO"

# Check all 4 levels exist
for L in 1 2 3 4; do
    D="$LORE_ALL_LEVELS_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    if [ ! -d "$D" ]; then
        echo "[ERROR] Missing: $D"
        exit 1
    fi
    echo "[OK] Lore-L${L} dir exists"
done

# Check pure SHAKE
echo ""
echo "=== SHAKE purity check ==="
ALL_PURE=1
for L in 1 2 3 4; do
    D="$LORE_ALL_LEVELS_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    hits=$(grep -RIn "pseudoXOF\|sm3hash\|pseudohash" "$D/fips202.c" "$D/fips202.h" 2>/dev/null | grep -v "^Binary" || true)
    if echo "$hits" | grep -q "pseudoXOF\|sm3hash\|pseudohash"; then
        echo "[FAIL] Lore-L${L}: contains SM3 wrapper calls in fips202"
        echo "$hits"
        ALL_PURE=0
    else
        echo "[OK] Lore-L${L}: pure SHAKE"
    fi
done
if [ "$ALL_PURE" -eq 0 ]; then
    echo "[ERROR] One or more levels contain SM3 wrapper. Aborting."
    exit 1
fi

# Check benchmark source
BENCH_SRC="$LORE_FORMAL_REPO/report/performance_tests/bench_lore_shake_kem_cycles_time.c"
if [ ! -f "$BENCH_SRC" ]; then
    echo "[ERROR] Benchmark source not found: $BENCH_SRC"
    exit 1
fi
echo "[OK] Benchmark source: $BENCH_SRC"

# Ensure output directories
mkdir -p "$LORE_FORMAL_REPO/report/performance"
echo "[OK] All checks passed"
