#!/bin/bash
set -euo pipefail

# Usage: ./build_bench.sh <repo_path> <version_name> <impl> <level>
# Example: ./build_bench.sh /home/syh/Work/zj-cuc-lore SHAKE Ref 1

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

# Determine CFLAGS based on impl
if [ "$IMPL" = "Reference" ]; then
    CFLAGS="-O3 -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls -Wshadow -Wpointer-arith -fomit-frame-pointer"
else
    CFLAGS="-O3 -DNDEBUG -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls -Wshadow -Wpointer-arith -fomit-frame-pointer"
fi

CFLAGS="$CFLAGS -I$SRC_DIR -DLORE_LEVEL=$LEVEL -DLORE_USE_API_PKC_DRNG"

echo "=== Building $VERSION $IMPL L$LEVEL ==="
echo "SRC: $SRC_DIR"
echo "CFLAGS: $CFLAGS"

# Compile all source files into objects
SRC_FILES=(
    kem.c pke.c indcpa.c polyvec.c poly.c ntt.c sampler.c reduce.c
    symmetric-shake.c fips202.c toomcook.c verify.c bch_codec.c
    randombytes.c auxfunc.c KEM_AlgorithmInstance.c drng.c
)

# Check for sm3.c
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

# Compile and link bench_kem
gcc $CFLAGS -c "$BENCH_C" -o "$BUILD_DIR/bench_kem.o"
gcc $CFLAGS "$BUILD_DIR/bench_kem.o" $OBJS -o "$BUILD_DIR/bench_kem" -lm

echo "Binary: $BUILD_DIR/bench_kem"
echo "DONE"
