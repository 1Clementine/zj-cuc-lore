#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "Usage: bash scripts/install_kat_template_to_instance.sh <AlgorithmInstanceDir>"
  exit 1
fi

TARGET="$1"
TEMPLATE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

mkdir -p "$TARGET"

for f in \
  KAT_KEM.c \
  KEM_AlgorithmInstance.c \
  KEM_AlgorithmInstance.h \
  auxfunc.c \
  auxfunc.h \
  drng.c \
  drng.h
do
  if [ ! -f "$TEMPLATE_ROOT/official_api/$f" ]; then
    echo "[ERROR] Missing official_api/$f"
    exit 1
  fi
  cp "$TEMPLATE_ROOT/official_api/$f" "$TARGET/$f"
done

echo "[DONE] Official KAT/API template installed to: $TARGET"
