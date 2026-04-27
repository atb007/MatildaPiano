# v3.0.0 Neural Network Integration Plan

**Date:** 2026-04-28  
**Approach:** Integrate PianoForte neural engine (MIT licensed)  
**Branch:** `v3-neural-network`

---

## Overview

Replace physical string model with neural network synthesis for authentic grand piano timbre.

**Key Decision:** Use proven PianoForte engine rather than building from scratch.

---

## Architecture

### What Changes
- **Voice engine only**: `MatildaPhysicalVoice` → `MatildaNeuralVoice` (PianoForte-based)
- **Add dependency**: ONNX Runtime for neural network inference

### What Stays (No Changes)
✅ UI (Figma design, knobs, XY pad, keyboard)  
✅ Effects chain (tape, delay, reverb, master gain)  
✅ Parameters (ADSR, reverb, delay, XY, master)  
✅ Processor architecture  
✅ Build system (CMake, JUCE 7+)  
✅ Branding ("Matilda Piano 2" → "Matilda Piano 3")

---

## Integration Steps

### Phase 1: Setup Dependencies (~2 hours)
- [ ] Download ONNX Runtime SDK (or use system package)
- [ ] Update CMakeLists.txt to find and link ONNX Runtime
- [ ] Verify build with stub integration

### Phase 2: Extract PianoForte Engine (~2 hours)
Files to copy from `/Users/udai.deori/Desktop/CursorAI/VST/Matilda/PianoForte-reference/`:
- [ ] `Source/Voices.h` → `Source/MatildaNeuralVoice.h`
- [ ] `Source/Voices.cpp` → `Source/MatildaNeuralVoice.cpp`
- [ ] `Source/SopranoVoice.h` → `Source/NeuralModels.h` (model weights)
- [ ] Trained ONNX models from their Resources/

### Phase 3: Create Adapter Layer (~2 hours)
- [ ] Rename classes to avoid conflicts (`pianoVoice` → `MatildaNeuralVoice`)
- [ ] Adapt to our `juce::SynthesiserVoice` interface
- [ ] Connect ADSR parameters (they have similar system)
- [ ] Handle model loading (from Resources/)

### Phase 4: Update Processor (~1 hour)
- [ ] Replace `setupPhysicalEngine()` with `setupNeuralEngine()`
- [ ] Initialize ONNX session
- [ ] Create 32 neural voices
- [ ] Test note triggering

### Phase 5: Testing & Polish (~2 hours)
- [ ] Test all MIDI notes, velocities
- [ ] Verify ADSR control works
- [ ] Test effects chain (should work unchanged)
- [ ] Check XY pad integration
- [ ] Performance profiling

### Phase 6: Documentation (~1 hour)
- [ ] Update README with neural engine details
- [ ] Add PianoForte attribution
- [ ] Update architecture.md
- [ ] Create v3.0.0 milestone in MILESTONES.md

---

## File Mapping

| PianoForte Source | Our Target | Purpose |
|-------------------|------------|---------|
| `Voices.h` (line 8) | Headers | ONNX includes, model class |
| `Voices.h` (NeuralModel class) | `MatildaNeuralEngine.h` | ONNX session wrapper |
| `Voices.h` (pianoVoice class) | `MatildaNeuralVoice.h` | Voice implementation |
| `Voices.cpp` | `MatildaNeuralVoice.cpp` | Voice logic |
| `SopranoVoice.h` | `NeuralModels.h` | Embedded model weights |
| `.onnx model files` | `Resources/models/` | Trained models |

---

## Dependencies

### ONNX Runtime
- **Version**: 1.14+ recommended
- **Size**: ~300MB SDK, ~5MB linked
- **Install**: Homebrew (`brew install onnxruntime`) or manual download
- **CMake**: `find_package(onnxruntime REQUIRED)`

### Trained Models
- **Source**: PianoForte releases (MIT licensed)
- **Size**: ~8KB per model
- **Format**: ONNX format
- **Pianos**: Steinway B/D, Yamaha C5

---

## Build Changes

### CMakeLists.txt additions:
```cmake
# Find ONNX Runtime
find_package(onnxruntime REQUIRED)

# Link to our plugin
target_link_libraries(MatildaPiano PRIVATE onnxruntime)

# Include ONNX headers
target_include_directories(MatildaPiano PRIVATE 
    ${ONNXRUNTIME_INCLUDE_DIRS})

# Copy .onnx models to bundle
file(GLOB ONNX_MODELS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/models/*.onnx")
# ... copy logic
```

---

## Attribution (MIT License)

Add to README.md:
```markdown
## Neural Engine Credits
The piano synthesis engine in v3.0.0+ is based on 
[PianoForte](https://github.com/tesserato/PianoForte) 
by Carlos Tarjano, licensed under MIT.

Original research: "An Efficient Algorithm For Segmenting 
Quasi-Periodic Digital Signals Into Pseudo Cycles" 
(IEEE/ACM Transactions on Audio Speech and Language Processing, 2022)
```

Include their LICENSE file: `v3-integration/LICENSE-PianoForte`

---

## Testing Checklist

- [ ] Builds without errors
- [ ] Loads ONNX models successfully
- [ ] All 88 piano keys play (MIDI 21-108)
- [ ] Velocity sensitivity works (soft to loud)
- [ ] ADSR controls shape the sound
- [ ] Effects chain sounds correct
- [ ] XY pad modulates effects
- [ ] No audio dropouts at 128 sample buffer
- [ ] Musical Typing keyboard works
- [ ] Side-by-side with v1/v2 (different plugin code)

---

## Timeline Estimate

| Phase | Hours | Dependencies |
|-------|-------|--------------|
| Setup | 2 | ONNX Runtime SDK |
| Extract | 2 | PianoForte code |
| Adapter | 2 | - |
| Integration | 1 | - |
| Testing | 2 | - |
| Docs | 1 | - |
| **Total** | **10 hours** | ~1.5 days |

---

## Risk Mitigation

**Risk**: ONNX Runtime integration complexity  
**Mitigation**: PianoForte already solved this; copy their approach

**Risk**: Real-time performance  
**Mitigation**: Their models are tiny (8KB, 1500 params); should be fine

**Risk**: License compliance  
**Mitigation**: MIT is permissive; just include attribution

**Risk**: Model quality  
**Mitigation**: Test with their pre-trained models first; proven to work

---

## Success Criteria

✅ Authentic grand piano timbre (not synthetic string sound)  
✅ All v2 features preserved (UI, effects, parameters)  
✅ Real-time performance (no dropouts)  
✅ Professional build quality  
✅ Proper attribution to PianoForte

---

## Next Steps

1. Download ONNX Runtime SDK
2. Copy PianoForte source files to `v3-integration/`
3. Start Phase 1 (Setup Dependencies)

See `v3-integration/README.md` for current status.
