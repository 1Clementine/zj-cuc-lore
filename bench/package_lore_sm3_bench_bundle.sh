#!/usr/bin/env bash
set -euo pipefail

BENCH_DIR="$(cd "$(dirname "$0")" && pwd)"
FORMAL_REPO="$(cd "$BENCH_DIR/.." && pwd)"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BUNDLE_NAME="lore_sm3_bench_bundle_${TIMESTAMP}"
OUTDIR="/home/syh/Work/${BUNDLE_NAME}"
TARBALL="/home/syh/Work/${BUNDLE_NAME}.tar.gz"

    exit 1
fi

echo "=== Packaging: $BUNDLE_NAME ==="

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR/zj-cuc-lore"
mkdir -p "$OUTDIR/zj-cuc-lore-sm3-all-levels-bench"

# Copy formal repo, excluding build artifacts
cd "$FORMAL_REPO"
tar -cf - --exclude='.git' --exclude='*.o' --exclude='*/output/*' \
    --exclude='*/build/*' --exclude='bench/results/*' \
    --exclude='report/performance_tests/build/*' \
    --exclude='bench_lore_sm3_kem_cycles_time' \
    --exclude='KAT_KEM_1' --exclude='KAT_KEM_2' \
    --exclude='KAT_KEM_3' --exclude='KAT_KEM_4' \
    --exclude='*/obj/*' . | tar -xf - -C "$OUTDIR/zj-cuc-lore"

tar -cf - --exclude='.git' --exclude='*.o' --exclude='*/output/*' \
    --exclude='*/build/*' --exclude='*/obj/*' \
    --exclude='KAT_KEM_1' --exclude='KAT_KEM_2' \
    --exclude='KAT_KEM_3' --exclude='KAT_KEM_4' \
    . | tar -xf - -C "$OUTDIR/zj-cuc-lore-sm3-all-levels-bench"

# Copy README
cp "$BENCH_DIR/README_RUN_ON_NEW_SERVER.md" "$OUTDIR/"

# Create MANIFEST
cd "$OUTDIR"
find . -type f | sort | xargs sha256sum > MANIFEST.sha256
rm -f MANIFEST.sha256  # exclude self from manifest
find . -type f ! -name MANIFEST.sha256 | sort | xargs sha256sum > MANIFEST.sha256

# Create run_on_new_server.sh
cat > run_on_new_server.sh << 'EOF'
#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/zj-cuc-lore"
export LORE_FORMAL_REPO="$(pwd)"
bash bench/run_full_lore_sm3_kem_bench.sh
EOF
chmod +x run_on_new_server.sh

# Create tarball
cd /home/syh/Work
tar -czf "$TARBALL" "$BUNDLE_NAME"
rm -rf "$OUTDIR"

echo ""
echo "=== Bundle created ==="
echo "  $TARBALL"
echo ""
echo "  Size: $(ls -lh "$TARBALL" | awk '{print $5}')"
echo ""
echo "  To run on new server:"
echo "    scp $TARBALL user@server:/path/"
echo "    ssh user@server"
echo "    tar -xzf $TARBALL"
echo "    cd $BUNDLE_NAME"
echo "    bash run_on_new_server.sh"
