#!/usr/bin/env bash
# ============================================================
#  poop3D Compiler — Run Script
# ============================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$SCRIPT_DIR/build/bin/poop3D"

if [ ! -f "$BIN" ]; then
  echo "Binary not found. Run ./build.sh first!"
  exit 1
fi

cd "$SCRIPT_DIR"
echo "Launching poop3D Compiler..."
exec "$BIN" "$@"
