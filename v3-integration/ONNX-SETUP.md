# ONNX Runtime Setup Guide

**Date:** 2026-04-28  
**Platform:** macOS (Apple Silicon / Intel)  
**Version Needed:** 1.14.0 or later

---

## What is ONNX Runtime?

ONNX Runtime is a **cross-platform inference engine** for ONNX (Open Neural Network Exchange) models. It's required to run the neural piano synthesis in v3.

- **Website**: https://onnxruntime.ai/
- **GitHub**: https://github.com/microsoft/onnxruntime
- **License**: MIT (same as PianoForte)

---

## Installation Options

### Option 1: Manual Download (RECOMMENDED)

Since Homebrew has SSL/network issues in the sandbox, we'll download manually.

#### Step 1: Download SDK

Visit: https://github.com/microsoft/onnxruntime/releases

Download the appropriate package:
- **macOS ARM64 (M1/M2/M3)**: `onnxruntime-osx-arm64-*.tgz`
- **macOS x86_64 (Intel)**: `onnxruntime-osx-x86_64-*.tgz`

**Latest stable** (as of 2026): v1.17.0

Direct links:
```bash
# ARM64 (M1/M2/M3)
curl -L https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-osx-arm64-1.17.0.tgz -o ~/Downloads/onnxruntime.tgz

# Intel x86_64
curl -L https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-osx-x86_64-1.17.0.tgz -o ~/Downloads/onnxruntime.tgz
```

#### Step 2: Extract

```bash
cd ~/Downloads
tar -xzf onnxruntime.tgz
```

This creates a folder like `onnxruntime-osx-arm64-1.17.0/` with:
```
include/        # Header files (onnxruntime_cxx_api.h, etc.)
lib/           # Dynamic library (libonnxruntime.dylib)
```

#### Step 3: Move to Standard Location

```bash
sudo mkdir -p /usr/local/onnxruntime
sudo mv onnxruntime-osx-*/include /usr/local/onnxruntime/
sudo mv onnxruntime-osx-*/lib /usr/local/onnxruntime/
```

**Result:**
```
/usr/local/onnxruntime/
├── include/
│   ├── onnxruntime_cxx_api.h
│   ├── onnxruntime_c_api.h
│   └── core/session/...
└── lib/
    ├── libonnxruntime.dylib
    └── libonnxruntime.1.17.0.dylib
```

#### Step 4: Verify Installation

```bash
ls /usr/local/onnxruntime/include/onnxruntime_cxx_api.h
ls /usr/local/onnxruntime/lib/libonnxruntime.dylib
```

Both should exist.

---

### Option 2: Homebrew (if network available)

```bash
brew install onnxruntime
```

**Caveat**: Currently failing in Cursor sandbox due to SSL certificate issues.

If this works, find the installation path:
```bash
brew --prefix onnxruntime
```

Typical path: `/opt/homebrew/opt/onnxruntime/` (ARM) or `/usr/local/opt/onnxruntime/` (Intel)

---

### Option 3: Build from Source (Advanced)

Only needed for custom optimizations or debugging.

```bash
git clone --recursive https://github.com/microsoft/onnxruntime
cd onnxruntime
./build.sh --config Release --build_shared_lib --parallel
```

Takes ~1 hour. Output in `build/MacOS/Release/`.

---

## CMake Integration

Once installed, update our `CMakeLists.txt`:

```cmake
# Find ONNX Runtime
set(ONNXRUNTIME_ROOT "/usr/local/onnxruntime" CACHE PATH "Path to ONNX Runtime")

# Option 1: Manual paths
include_directories(${ONNXRUNTIME_ROOT}/include)
link_directories(${ONNXRUNTIME_ROOT}/lib)

target_link_libraries(MatildaPiano PRIVATE onnxruntime)

# Option 2: Use find_package (if onnxruntime provides CMake config)
# find_package(onnxruntime REQUIRED)
# target_link_libraries(MatildaPiano PRIVATE onnxruntime::onnxruntime)
```

---

## Testing ONNX Runtime

### Minimal Test Program

Create `test_onnx.cpp`:

```cpp
#include <iostream>
#include "onnxruntime_cxx_api.h"

int main() {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
    std::cout << "ONNX Runtime initialized successfully!" << std::endl;
    return 0;
}
```

Compile:
```bash
clang++ -std=c++17 \
  -I/usr/local/onnxruntime/include \
  -L/usr/local/onnxruntime/lib \
  -lonnxruntime \
  test_onnx.cpp -o test_onnx
  
./test_onnx
```

**Expected output:**
```
ONNX Runtime initialized successfully!
```

---

## Troubleshooting

### Error: "dyld: Library not loaded: @rpath/libonnxruntime.dylib"

**Solution 1**: Add to `DYLD_LIBRARY_PATH`:
```bash
export DYLD_LIBRARY_PATH=/usr/local/onnxruntime/lib:$DYLD_LIBRARY_PATH
```

**Solution 2**: Use absolute path in CMake:
```cmake
set_target_properties(MatildaPiano PROPERTIES
    INSTALL_RPATH "/usr/local/onnxruntime/lib"
    BUILD_RPATH "/usr/local/onnxruntime/lib"
)
```

**Solution 3**: Copy dylib to plugin bundle (done automatically by JUCE if configured):
```cmake
# JUCE will copy dylibs to .component/Contents/MacOS/
```

### Error: "onnxruntime_cxx_api.h: No such file"

Check include path:
```bash
ls /usr/local/onnxruntime/include/onnxruntime_cxx_api.h
```

Update CMakeLists.txt include path if different.

### Error: "Undefined symbols for architecture arm64"

Wrong architecture. Download the ARM64 version, not x86_64.

---

## File Sizes

- **SDK Download**: ~300MB compressed, ~500MB extracted
- **Linked Library**: ~5-8MB added to plugin binary
- **Headers**: ~50MB (not included in final binary)

---

## Model Files

The neural network model is **8KB** (`engineMain` in PianoForte):
- Format: ONNX binary
- Embedded: via JUCE `BinaryData`
- No external files needed at runtime

Already copied to: `v3-integration/engineMain`

---

## License Compatibility

✅ **ONNX Runtime**: MIT License  
✅ **PianoForte models**: MIT License  
✅ **Our plugin**: Compatible (MIT allows commercial use with attribution)

---

## Next Steps

1. Download and extract ONNX Runtime
2. Verify installation with test program
3. Update `CMakeLists.txt` with paths
4. Test build with stub integration

---

## Reference

- **ONNX Runtime Docs**: https://onnxruntime.ai/docs/
- **C++ API Reference**: https://onnxruntime.ai/docs/api/c/
- **GitHub Releases**: https://github.com/microsoft/onnxruntime/releases
- **PianoForte Integration**: See `ARCHITECTURE-ANALYSIS.md`

---

**Status**: Ready for manual download and installation.  
**Estimated setup time**: 15-30 minutes
