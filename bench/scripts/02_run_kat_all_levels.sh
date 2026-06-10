#!/usr/bin/env bash
set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RUN_ID="${1:-$(date +%Y%m%d_%H%M%S)}"
LOG_DIR="$BENCH_DIR/results/$RUN_ID/logs"
VAL_DIR="$BENCH_DIR/results/$RUN_ID/validation"
KAT_CSV="$VAL_DIR/kat_summary.csv"

mkdir -p "$LOG_DIR" "$VAL_DIR"
echo "level,paper_name,kat_build,kat_run,kat_file,notes" > "$KAT_CSV"

for L in 1 2 3 4; do
    case $L in
        1) PAPER="Lore-128";;
        2) PAPER="Lore-256";;
        3) PAPER="Lore-384";;
        4) PAPER="Lore-512";;
    esac

    D="$LORE_ALL_LEVELS_REPO/Implementations/Reference_Implementation/Lore-L${L}"
    echo "===== KAT Lore-L${L} ($PAPER) ====="

    cd "$D"
    make clean 2>/dev/null || true

    if make KAT_KEM_${L} > "$LOG_DIR/kat_L${L}_build.log" 2>&1; then
        BUILD="PASS"
        if [ -x "./KAT_KEM_${L}" ]; then
            if ./KAT_KEM_${L} > "$LOG_DIR/kat_L${L}_run.log" 2>&1; then
                RUN="PASS"
                kat_file=$(ls output/KAT_KEM_*.txt 2>/dev/null | head -1)
                if [ -n "$kat_file" ]; then
                    cp "$kat_file" "$VAL_DIR/KAT_KEM_Lore-L${L}_pure_shake.txt"
                    lines=$(wc -l < "$kat_file")
                    echo "  KAT: $lines lines"
                fi
            else
                RUN="FAIL"
                echo "  KAT RUN FAILED for L${L}"
            fi
        else
            RUN="FAIL (no binary)"
        fi
    else
        BUILD="FAIL"
        RUN="N/A"
        echo "  KAT BUILD FAILED for L${L}"
    fi

    echo "$L,$PAPER,$BUILD,$RUN,$VAL_DIR/KAT_KEM_Lore-L${L}_pure_shake.txt,10000 iters" >> "$KAT_CSV"
done

echo ""
echo "=== KAT Summary ==="
cat "$KAT_CSV"
