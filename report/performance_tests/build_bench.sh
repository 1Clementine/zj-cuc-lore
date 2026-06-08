#!/bin/bash
set -euo pipefail

# Usage: ./build_bench.sh <repo_path> <version_name> <impl> <level>
# MODE=generic (default) or MODE=native

REPO="$1"
VERSION="$2"
IMPL="$3"
LEVEL="$4"

SRC_DIR="$REPO/Implementations/${IMPL}_Implementation/Lore-L${LEVEL}"
BUILD_DIR="$(dirname "$0")/build/${VERSION}_${IMPL}_L${LEVEL}"
BENCH_C="$(dirname "$0")/bench_kem.c"

if [ ! -d "$SRC_DIR" ]; then
    echo "ERROR: Source directory not found: $SRC_DIR"
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# --- MODE selection ---
MODE="${MODE:-generic}"

COMMON_CFLAGS="-O3 -DNDEBUG -std=gnu11 -Wall -Wextra"
ARCH_CFLAGS=""

case "$MODE" in
  generic)
    ARCH_CFLAGS=""
    ;;
  native)
    arch="$(uname -m)"
    if [ "$arch" = "x86_64" ]; then
      ARCH_CFLAGS="-march=native"
    elif [ "$arch" = "aarch64" ] || [ "$arch" = "arm64" ]; then
      ARCH_CFLAGS="-mcpu=native"
    fi
    ;;
  *)
    echo "[ERROR] Unknown MODE=$MODE, use MODE=generic or MODE=native"
    exit 1
    ;;
esac

CFLAGS="$COMMON_CFLAGS $ARCH_CFLAGS -I$SRC_DIR -DLORE_LEVEL=$LEVEL -DLORE_USE_API_PKC_DRNG"

# Enable AVX2 if Optimized implementation has avx2_utils.h
if [ -f "$SRC_DIR/avx2_utils.h" ]; then
    CFLAGS="$CFLAGS -DLORE_USE_AVX2 -mavx2"
fi

echo "=== Building $VERSION $IMPL L$LEVEL [MODE=$MODE] ==="
echo "CFLAGS: $CFLAGS"

# Compile all source files into objects
SRC_FILES=(
    kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c
    symmetric-shake.c fips202.c toomcook.c verify.c bch_codec.c
    randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c
)

if [ -f "$SRC_DIR/sm3.c" ]; then
    SRC_FILES+=(sm3.c)
fi

OBJS=""
for f in "${SRC_FILES[@]}"; do
    if [ -f "$SRC_DIR/$f" ]; then
        obj="$BUILD_DIR/${f%.c}.o"
        gcc $CFLAGS -c "$SRC_DIR/$f" -o "$obj"
        OBJS="$OBJS $obj"
    fi
done

gcc $CFLAGS -c "$BENCH_C" -o "$BUILD_DIR/bench_kem.o"
gcc $CFLAGS "$BUILD_DIR/bench_kem.o" $OBJS -o "$BUILD_DIR/bench_kem" -lm

echo "Binary: $BUILD_DIR/bench_kem"
echo "DONE"
