#!/bin/bash
# Full clean + build for Matilda Piano (for designers / when you get build errors)
# Run from the project folder: ./clean-and-build.sh

set -e

# ——— Edit this path if your JUCE folder is somewhere else ———
JUCE_PATH='/Users/udai.deori/Desktop/CursorAI/JUCE 2'
# —————————————————————————————————————————————————————————————

echo "=========================================="
echo "  Matilda Piano — full clean + build"
echo "=========================================="
echo ""

# 0. Use Xcode's SDK if Xcode is installed (fixes 'algorithm' not found)
CMAKE_EXTRA=""
if [ -d "/Applications/Xcode.app" ]; then
  XCODE_SDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
  if [ -d "$XCODE_SDK" ]; then
    CMAKE_EXTRA="-DCMAKE_OSX_SYSROOT=$XCODE_SDK"
    echo "Using Xcode SDK for system headers."
  fi
  # Remind to select Xcode if build still fails with 'algorithm' not found
  CURRENT_SELECT=$(xcode-select -p 2>/dev/null || true)
  if [ "$CURRENT_SELECT" != "/Applications/Xcode.app/Contents/Developer" ]; then
    echo "Tip: If you see 'algorithm' not found, run once: sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
    echo ""
  fi
fi
echo ""

# 1. Full clean: remove old build folder so we start fresh
if [ -d build ]; then
  echo "Removing old build folder (full clean)..."
  rm -rf build
  echo "Done."
else
  echo "No existing build folder; starting fresh."
fi
echo ""

# 2. Set JUCE path and build (JUCE_BUILD_EXTRAS=OFF skips juceaide to avoid toolchain issues)
export JUCE_DIR="$JUCE_PATH"
echo "Using JUCE at: $JUCE_DIR"
echo ""
echo "Configuring and building (this may take a few minutes)..."
echo ""

cmake -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="$JUCE_DIR" -DJUCE_BUILD_EXTRAS=OFF $CMAKE_EXTRA
cmake --build build --config Release

echo ""
echo "=========================================="
echo "  Build finished successfully"
echo "=========================================="
echo ""
echo "Standalone app: build/MatildaPiano_artefacts/Release/Matilda Piano.app"
echo "AU plugin:      ~/Library/Audio/Plug-Ins/Components/MatildaPiano.component"
echo ""
echo "To open the app:"
echo "  open build/MatildaPiano_artefacts/Release/Matilda\\ Piano.app"
echo ""
