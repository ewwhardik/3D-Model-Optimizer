#!/usr/bin/env bash
# ============================================================
#  poop3D Compiler – Dependency Setup Script
#  Run this ONCE before building to fetch all libraries
# ============================================================

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="$SCRIPT_DIR/src"

echo ""
echo "=========================================="
echo "  poop3D Compiler — Dependency Fetcher"
echo "=========================================="
echo ""

# ─── Detect OS ───────────────────────────────────────────────────────────────
OS="$(uname -s)"
case "$OS" in
  Linux*)  PLATFORM="Linux" ;;
  Darwin*) PLATFORM="Mac" ;;
  MINGW*|MSYS*|CYGWIN*) PLATFORM="Windows" ;;
  *) PLATFORM="Unknown" ;;
esac
echo "Platform: $PLATFORM"

# ─── Check tools ─────────────────────────────────────────────────────────────
for cmd in git cmake; do
  if ! command -v "$cmd" &>/dev/null; then
    echo "ERROR: '$cmd' not found. Please install it first."
    exit 1
  fi
done

# ─── Dear ImGui ──────────────────────────────────────────────────────────────
IMGUI_DIR="$SRC/imgui"
if [ ! -f "$IMGUI_DIR/imgui.h" ]; then
  echo "[1/3] Downloading Dear ImGui v1.90.5..."
  mkdir -p "$IMGUI_DIR/backends"
  IMGUI_BASE="https://raw.githubusercontent.com/ocornut/imgui/v1.90.5"
  for f in imgui.h imgui.cpp imgui_internal.h imgui_draw.cpp \
            imgui_tables.cpp imgui_widgets.cpp imconfig.h imstb_rectpack.h \
            imstb_textedit.h imstb_truetype.h; do
    curl -sL "$IMGUI_BASE/$f" -o "$IMGUI_DIR/$f"
  done
  for f in imgui_impl_glfw.h imgui_impl_glfw.cpp \
            imgui_impl_opengl3.h imgui_impl_opengl3.cpp \
            imgui_impl_opengl3_loader.h; do
    curl -sL "$IMGUI_BASE/backends/$f" -o "$IMGUI_DIR/backends/$f"
  done
  echo "  ✔ ImGui downloaded"
else
  echo "[1/3] ImGui already present — skipping"
fi

# ─── GLAD ────────────────────────────────────────────────────────────────────
GLAD_DIR="$SRC/glad"
if [ ! -f "$GLAD_DIR/glad.c" ]; then
  echo "[2/3] Downloading GLAD (OpenGL 3.3 Core)..."
  mkdir -p "$GLAD_DIR/KHR"
  GLAD_BASE="https://raw.githubusercontent.com/Dav1dde/glad/v0.1.36/src"
  GLAD_INC="https://raw.githubusercontent.com/Dav1dde/glad/v0.1.36/include/glad"
  GLAD_KHR="https://raw.githubusercontent.com/Dav1dde/glad/v0.1.36/include/KHR"
  # Try the pre-generated GL 3.3 Core files from a known-good source
  curl -sL "https://raw.githubusercontent.com/Dav1dde/glad/c/src/glad.c" \
       -o "$GLAD_DIR/glad.c" 2>/dev/null || true
  curl -sL "https://raw.githubusercontent.com/Dav1dde/glad/c/include/glad/glad.h" \
       -o "$GLAD_DIR/glad.h" 2>/dev/null || true
  curl -sL "https://raw.githubusercontent.com/Dav1dde/glad/c/include/KHR/khrplatform.h" \
       -o "$GLAD_DIR/KHR/khrplatform.h" 2>/dev/null || true

  # Verify we got valid files
  if [ ! -s "$GLAD_DIR/glad.c" ] || [ ! -s "$GLAD_DIR/glad.h" ]; then
    echo "  Fallback: generating GLAD via pip..."
    pip3 install glad 2>/dev/null || pip install glad 2>/dev/null || true
    python3 -m glad --out-path "$GLAD_DIR" --api gl:core=3.3 c 2>/dev/null || true
    # Move if needed
    if [ -f "$GLAD_DIR/src/glad.c" ]; then
      cp "$GLAD_DIR/src/glad.c" "$GLAD_DIR/glad.c"
      cp "$GLAD_DIR/include/glad/glad.h" "$GLAD_DIR/glad.h"
      cp "$GLAD_DIR/include/KHR/khrplatform.h" "$GLAD_DIR/KHR/khrplatform.h"
    fi
  fi
  echo "  ✔ GLAD downloaded"
else
  echo "[2/3] GLAD already present — skipping"
fi

# ─── System packages for Linux ───────────────────────────────────────────────
if [ "$PLATFORM" = "Linux" ]; then
  echo "[3/3] Checking Linux system packages..."
  if command -v apt-get &>/dev/null; then
    sudo apt-get install -y \
      libglfw3-dev \
      libassimp-dev \
      libgl1-mesa-dev \
      libx11-dev \
      libxrandr-dev \
      libxinerama-dev \
      libxcursor-dev \
      libxi-dev \
      cmake \
      build-essential 2>/dev/null || true
  elif command -v pacman &>/dev/null; then
    sudo pacman -S --noconfirm glfw assimp mesa cmake gcc 2>/dev/null || true
  elif command -v dnf &>/dev/null; then
    sudo dnf install -y glfw-devel assimp-devel mesa-libGL-devel cmake gcc-c++ 2>/dev/null || true
  fi
  echo "  ✔ System packages installed"
elif [ "$PLATFORM" = "Mac" ]; then
  echo "[3/3] Checking Homebrew packages..."
  if command -v brew &>/dev/null; then
    brew install glfw assimp cmake 2>/dev/null || true
  else
    echo "  ⚠ Homebrew not found. Install from https://brew.sh then run:"
    echo "    brew install glfw assimp cmake"
  fi
  echo "  ✔ Mac packages checked"
else
  echo "[3/3] Windows: Use vcpkg or manual install (see README)"
fi

echo ""
echo "=========================================="
echo "  All dependencies ready!"
echo "  Now run: ./build.sh"
echo "=========================================="
