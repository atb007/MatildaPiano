#!/bin/bash

# Build script for Matilda Piano VST Plugin

set -e

echo "Building Matilda Piano Plugin..."

# Check if JUCE_DIR is set; otherwise try default path from DEV-HANDOFF (this machine)
if [ -z "$JUCE_DIR" ]; then
    DEFAULT_JUCE="/Users/udai.deori/Desktop/CursorAI/JUCE 2"
    if [ -d "$DEFAULT_JUCE" ]; then
        export JUCE_DIR="$DEFAULT_JUCE"
        echo "Using JUCE from: $JUCE_DIR"
    else
        echo "Error: JUCE_DIR environment variable is not set."
        echo "Set it to either:"
        echo "  - JUCE install prefix (contains JUCEConfig.cmake), or"
        echo "  - JUCE source directory (e.g. your 'JUCE 2' folder)"
        echo ""
        echo "  export JUCE_DIR='/path/to/JUCE 2'"
        echo "  ./build.sh"
        exit 1
    fi
fi

# Use Xcode SDK if available (fixes 'algorithm' not found)
CMAKE_EXTRA=""
if [ -d "/Applications/Xcode.app" ]; then
  XCODE_SDK="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
  [ -d "$XCODE_SDK" ] && CMAKE_EXTRA="-DCMAKE_OSX_SYSROOT=$XCODE_SDK"
fi

# Configure CMake (JUCE_BUILD_EXTRAS=OFF skips juceaide)
echo "Configuring CMake..."
cmake -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="$JUCE_DIR" -DJUCE_BUILD_EXTRAS=OFF $CMAKE_EXTRA

# Build
echo "Building plugin..."
cmake --build build --config Release

echo ""
echo "Build complete! Plugin installed to:"
echo "  ~/Library/Audio/Plug-Ins/Components/MatildaPiano.component"
echo ""
echo "To test in GarageBand:"
echo "  1. Open GarageBand"
echo "  2. Create a Software Instrument track"
echo "  3. Select 'Matilda Piano' from the Audio Units list"
