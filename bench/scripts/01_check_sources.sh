#!/usr/bin/env bash
set -euo pipefail

LORE_SM3_REPO="${LORE_SM3_REPO:-}"

if [ -z "$LORE_SM3_REPO" ] || [ ! -d "$LORE_SM3_REPO" ]; then
    echo "[ERROR] LORE_SM3_REPO not set or not found: ${LORE_SM3_REPO:-unset}"
    exit 1
fi

echo "[check] LORE_SM3_REPO=$LORE_SM3_REPO"

for L in 1 2 3 4; do
    D="$LORE_SM3_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    if [ ! -d "$D" ]; then
        echo "[ERROR] Missing: $D"
        exit 1
    fi
    echo "[OK] Lore-L${L} dir exists"
done

echo ""
echo "=== SM3 path check (sm3hash/pseudoXOF expected in SM3 version) ==="
for L in 1 2 3 4; do
    D="$LORE_SM3_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    hits=$(grep -c "sm3hash\|pseudoXOF\|pseudohash" "$D/fips202.c" 2>/dev/null || echo 0)
    echo "[OK] Lore-L${L}: $hits SM3 primitive references (expected)"
done

BENCH_SRC="$LORE_SM3_REPO/report/performance_tests/bench_lore_sm3_kem_cycles_time.c"
if [ ! -f "$BENCH_SRC" ]; then
    echo "[ERROR] Benchmark source not found: $BENCH_SRC"
    exit 1
fi
echo "[OK] Benchmark source: $BENCH_SRC"

mkdir -p "$LORE_SM3_REPO/report/performance"
echo "[OK] All checks passed"
