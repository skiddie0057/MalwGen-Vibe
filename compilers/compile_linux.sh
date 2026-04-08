#!/bin/bash

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_DIR="$PROJECT_ROOT/output/linux"

echo "=== Malw Generator - Linux ==="
echo ""

if ! command -v gcc &> /dev/null; then
    echo "[ERROR] gcc not found. Install with: sudo apt install gcc"
    exit 1
fi
echo "[OK] gcc found: $(which gcc)"
echo "     Version: $(gcc --version | head -n1)"

if command -v upx &> /dev/null; then
    UPX_AVAILABLE=1
    UPX_VERSION=$(upx --version 2>&1 | head -n1)
    echo "[OK] UPX found: $UPX_VERSION"
else
    UPX_AVAILABLE=0
    echo "[--] UPX not found (optional - binaries will not be packed)"
fi

echo ""
mkdir -p "$OUTPUT_DIR"

CFLAGS="-O2 -fno-stack-protector -no-pie -z execstack -I$PROJECT_ROOT"
LDFLAGS="-pthread"

echo "--- Compiling ---"
echo ""

echo "[1/3] Building simple_malw..."
gcc $CFLAGS "$PROJECT_ROOT/malw/simple/simple_malw.c" \
    -o "$OUTPUT_DIR/simple_malw" \
    $LDFLAGS
echo "  -> simple_malw built ($(stat -c%s "$OUTPUT_DIR/simple_malw") bytes)"

if [ $UPX_AVAILABLE -eq 1 ]; then
    cp "$OUTPUT_DIR/simple_malw" "$OUTPUT_DIR/simple_malw.packed"
    upx --best "$OUTPUT_DIR/simple_malw.packed" 2>/dev/null
    echo "  -> simple_malw.packed ($(stat -c%s "$OUTPUT_DIR/simple_malw.packed") bytes)"
fi

echo ""
echo "[2/3] Building medium_malw..."
gcc $CFLAGS "$PROJECT_ROOT/malw/medium/medium_malw.c" \
    -o "$OUTPUT_DIR/medium_malw" \
    $LDFLAGS
echo "  -> medium_malw built ($(stat -c%s "$OUTPUT_DIR/medium_malw") bytes)"

if [ $UPX_AVAILABLE -eq 1 ]; then
    cp "$OUTPUT_DIR/medium_malw" "$OUTPUT_DIR/medium_malw.packed"
    upx --best "$OUTPUT_DIR/medium_malw.packed" 2>/dev/null
    echo "  -> medium_malw.packed ($(stat -c%s "$OUTPUT_DIR/medium_malw.packed") bytes)"
fi

echo ""
echo "[3/3] Building complex_malw..."
gcc $CFLAGS "$PROJECT_ROOT/malw/complex/complex_malw.c" \
    -o "$OUTPUT_DIR/complex_malw" \
    $LDFLAGS
echo "  -> complex_malw built ($(stat -c%s "$OUTPUT_DIR/complex_malw") bytes)"

if [ $UPX_AVAILABLE -eq 1 ]; then
    cp "$OUTPUT_DIR/complex_malw" "$OUTPUT_DIR/complex_malw.packed"
    upx --best "$OUTPUT_DIR/complex_malw.packed" 2>/dev/null
    echo "  -> complex_malw.packed ($(stat -c%s "$OUTPUT_DIR/complex_malw.packed") bytes)"
fi

echo ""
echo "=== Build Complete ==="
ls -la "$OUTPUT_DIR"
