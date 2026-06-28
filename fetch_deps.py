#!/usr/bin/env python3
"""
poop3D Compiler — Automatic Dependency Downloader
Run this script once before building to download ImGui and GLAD.

Usage:
    python3 fetch_deps.py        # Linux / macOS
    python fetch_deps.py         # Windows (cmd or PowerShell)
"""

import os
import sys
import urllib.request
import urllib.error
import platform

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR    = os.path.join(SCRIPT_DIR, "src")

IMGUI_TAG  = "v1.90.5"
IMGUI_DIR  = os.path.join(SRC_DIR, "imgui")
GLAD_DIR   = os.path.join(SRC_DIR, "glad")
GLAD_KHR   = os.path.join(GLAD_DIR, "KHR")

IMGUI_BASE = f"https://raw.githubusercontent.com/ocornut/imgui/{IMGUI_TAG}"

IMGUI_FILES = [
    "imconfig.h",
    "imgui.h",
    "imgui.cpp",
    "imgui_internal.h",
    "imgui_draw.cpp",
    "imgui_tables.cpp",
    "imgui_widgets.cpp",
    "imstb_rectpack.h",
    "imstb_textedit.h",
    "imstb_truetype.h",
]

IMGUI_BACKEND_FILES = [
    "backends/imgui_impl_glfw.h",
    "backends/imgui_impl_glfw.cpp",
    "backends/imgui_impl_opengl3.h",
    "backends/imgui_impl_opengl3.cpp",
    "backends/imgui_impl_opengl3_loader.h",
]

# NOTE: glad.c and glad.h are already bundled in src/glad/ —
# these fallback URLs are only used if those files are missing or corrupted.
GLAD_FILES = {
    "glad.h":             "https://raw.githubusercontent.com/Dav1dde/glad/c/include/glad/glad.h",
    "glad.c":             "https://raw.githubusercontent.com/Dav1dde/glad/c/src/glad.c",
    "KHR/khrplatform.h":  "https://raw.githubusercontent.com/Dav1dde/glad/c/include/KHR/khrplatform.h",
}


def fetch(url, dest_path):
    os.makedirs(os.path.dirname(dest_path), exist_ok=True)
    name = os.path.basename(dest_path)
    print(f"  Downloading {name}...", end=" ", flush=True)
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "poop3D-fetch/1.0"})
        with urllib.request.urlopen(req, timeout=30) as resp:
            data = resp.read()
        if len(data) < 50:
            print(f"FAILED (suspiciously small: {len(data)} bytes)")
            return False
        with open(dest_path, "wb") as f:
            f.write(data)
        print(f"OK ({len(data):,} bytes)")
        return True
    except urllib.error.HTTPError as e:
        print(f"FAILED: HTTP {e.code} — {e.reason}")
        return False
    except urllib.error.URLError as e:
        print(f"FAILED: {e.reason}")
        return False
    except Exception as e:
        print(f"FAILED: {e}")
        return False


def check_file(path, min_bytes=100):
    return os.path.exists(path) and os.path.getsize(path) >= min_bytes


def main():
    print()
    print("=" * 52)
    print("   poop3D Compiler — Dependency Downloader")
    print("=" * 52)
    print(f"   Platform : {platform.system()} {platform.machine()}")
    print(f"   Python   : {sys.version.split()[0]}")
    print()

    all_ok = True

    # ─── Dear ImGui ─────────────────────────────────────────────────────────────
    print("[1/2] Dear ImGui (tag: v1.90.5):")
    os.makedirs(os.path.join(IMGUI_DIR, "backends"), exist_ok=True)

    for fname in IMGUI_FILES:
        dest = os.path.join(IMGUI_DIR, fname)
        if check_file(dest):
            print(f"  {fname} — already present")
            continue
        if not fetch(f"{IMGUI_BASE}/{fname}", dest):
            all_ok = False

    for fname in IMGUI_BACKEND_FILES:
        dest = os.path.join(IMGUI_DIR, fname)
        if check_file(dest):
            print(f"  {fname} — already present")
            continue
        if not fetch(f"{IMGUI_BASE}/{fname}", dest):
            all_ok = False

    print()

    # ─── GLAD ────────────────────────────────────────────────────────────────────
    print("[2/2] GLAD (OpenGL 3.3 Core):")
    os.makedirs(GLAD_KHR, exist_ok=True)

    for fname, url in GLAD_FILES.items():
        dest = os.path.join(GLAD_DIR, fname)
        if check_file(dest, 500):
            print(f"  {fname} — already present")
            continue
        if not fetch(url, dest):
            print(f"  WARNING: Could not download {fname}")
            all_ok = False

    print()

    # ─── Summary ─────────────────────────────────────────────────────────────────
    if all_ok:
        print("=" * 52)
        print("  ✔  All dependencies ready!")
        print()
        if platform.system() == "Windows":
            print("  Next step:  build_windows.bat")
            print("  (Or open the folder in Visual Studio and build)")
        else:
            print("  Next step:  chmod +x build.sh && ./build.sh")
            if platform.system() == "Linux":
                print()
                print("  If GLFW/Assimp are missing:")
                print("    Ubuntu/Debian: sudo apt-get install libglfw3-dev libassimp-dev")
                print("    Arch:          sudo pacman -S glfw assimp")
                print("    Fedora:        sudo dnf install glfw-devel assimp-devel")
            elif platform.system() == "Darwin":
                print()
                print("  If GLFW/Assimp are missing:")
                print("    brew install glfw assimp")
        print("=" * 52)
    else:
        print("=" * 52)
        print("  ⚠  Some downloads failed.")
        print()
        print("  Check your internet connection and try again.")
        print("  Alternatively, CMake will auto-download ImGui")
        print("  via FetchContent when you run the build.")
        print("=" * 52)
        sys.exit(1)

    print()


if __name__ == "__main__":
    main()
