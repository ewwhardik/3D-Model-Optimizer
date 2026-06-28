#!/usr/bin/env bash
# ============================================================
#  poop3D Compiler — Build Script (Linux / macOS)
# ============================================================

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$BUILD_DIR/bin"

echo ""
echo "=========================================="
echo "  poop3D Compiler — Build"
echo "=========================================="

# Check deps are downloaded
if [ ! -f "$SCRIPT_DIR/src/imgui/imgui.h" ]; then
  echo "ERROR: Dependencies missing. Run ./setup.sh first!"
  exit 1
fi

if [ ! -f "$SCRIPT_DIR/src/glad/glad.c" ]; then
  echo "ERROR: GLAD missing. Run ./setup.sh first!"
  exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Detect build type
BUILD_TYPE="${1:-Release}"
echo "Build type: $BUILD_TYPE"

# CMake configure
cmake "$SCRIPT_DIR" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build (use all cores)
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo "Building with $CORES cores..."
cmake --build . --config "$BUILD_TYPE" -- -j"$CORES"

echo ""
echo "=========================================="
echo "  Build SUCCESS!"
echo "  Binary: $BIN_DIR/poop3D"
echo ""
echo "  Run with:  $BIN_DIR/poop3D"
echo "  or simply: ./run.sh"
echo "=========================================="
