# PianoForte Architecture Analysis

**Date:** 2026-04-28  
**Source:** `/Users/udai.deori/Desktop/CursorAI/VST/Matilda/PianoForte-reference/`

---

## Key Files Copied

```
v3-integration/reference-source/
├── Voices.h          (18.6KB) - NeuralModel class, pianoVoice class
├── Voices.cpp        (5.4KB)  - Voice implementation
└── SopranoVoice.h    (109KB)  - Embedded model weights (large!)
```

---

## Architecture Overview

### Core Classes

#### 1. `NeuralModel` (Voices.h lines 160-227)
- Loads ONNX models via **ONNX Runtime C++ API**
- Models loaded from **BinaryData** (embedded in plugin binary)
- Input shape: `{0}` (dynamic), Output shape: `{0}` (dynamic)
- Key method: `eval(vector<float>& I, vector<float>& O)` - runs inference

**Dependencies:**
```cpp
#include "core/session/onnxruntime_cxx_api.h"
```

#### 2. `pianoVoice` : public `juce::SynthesiserVoice` (Voices.h lines 395-515)
- Inherits from JUCE's `SynthesiserVoice`
- Uses `NeuralModel*` pointer for neural synthesis
- **Hybrid approach**: Blends neural network + physical model
  - Neural: Frequency domain amplitudes from ONNX
  - Physical: Time domain synthesis with harmonics
- Async inference with `std::future` for non-blocking

**Key Features:**
- Polyphony: 10 voices (const POLYPHONY = 10)
- MIDI range: 21-108 (88 piano keys)
- Velocity-sensitive
- Sustain pedal support
- Attack/decay envelopes
- Tail-off for natural release

---

## Neural Network Details

### Model Format
- **ONNX** format (Open Neural Network Exchange)
- **Size**: ~8KB per model (Resources/engineMain)
- **Embedded**: Stored in BinaryData, no external files needed
- **Parameters**: ~1500 (estimated from research paper)

### Input/Output
- **Input**: `{pitch, velocity, periodCount}`
  - `pitch`: Normalized 0-1 (MIDI 21-108 → 0.0-1.0)
  - `velocity`: 0-1
  - `periodCount`: Progressive counter (tanh normalized)
  
- **Output**: Frequency domain amplitudes (spectrum)
  - Variable size based on model
  - Used as harmonic amplitudes for synthesis

### Inference Strategy
- **Async execution**: `std::async` to avoid blocking audio thread
- **Double buffering**: `targetAmps` / `currentAmps`
- **Smooth interpolation**: Blend between predictions at `2000.0f / fps` rate

---

## Synthesis Pipeline

### `startNote()` Flow
1. Set MIDI note and velocity
2. Normalize to `pitch` (0-1)
3. Initialize random phases for stereo spread
4. Launch async `forward()` to run ONNX inference
5. Start physical model (harmonics backup)
6. Wait for neural model ready (~100ms max)
7. Copy `targetAmps` → `currentAmps`

### `renderNextBlock()` Flow
1. Check if voice is sounding
2. For each sample:
   - Call `getNextSample()`
     - Update async inference if needed
     - Interpolate `currentAmps` toward `targetAmps`
     - Compute neural synthesis (freq domain)
     - Compute physical synthesis (time domain)
     - **Blend**: `alpha * neural + beta * physical`
   - Apply attack/decay envelope
   - Apply tail-off if key released
3. Add to output buffer

### Blending Strategy
```cpp
float alpha = 1.0f - (1.0f - pitch) * 0.95f;  // pitch-dependent
float beta = 1.0f - alpha;
W[0] = W[0] * 0.7f * alpha + beta * WD[0];  // neural * alpha + physical * beta
```

**Interpretation:**
- High notes (pitch → 1): More neural (alpha → 1)
- Low notes (pitch → 0): More physical (beta → 1)
- Neural scaled by 0.7 (reduce volume slightly)

---

## Physical Model Fallback

They include a **physical model** as a fallback/blend:

### Harmonic Groups
- **G1** (MIDI 21-30): Low bass (13 harmonics)
- **G2** (MIDI 31-50): Mid range (13 harmonics)
- **G3** (MIDI 51-108): High treble (13 harmonics)

Each group has:
- Frequency ratios (`G1F`, `G2F`, `G3F`)
- Amplitude ratios (`G1A`, `G2A`, `G3A`)

### Synthesis
```cpp
yL += a * std::sin(pL + h) * std::exp(-0.0003f * h);  // Exponential decay
```

- Random stereo phases (`phasesL`, `phasesR`)
- Natural decay via exponential
- Summed harmonics

---

## ONNX Runtime Integration

### Header Inclusion
```cpp
#include "core/session/onnxruntime_cxx_api.h"
```

This is the **ONNX Runtime C++ API**.

### Session Creation
```cpp
Ort::Env env{ ORT_LOGGING_LEVEL_WARNING, "Piano" };
const void* data = BinaryData::getNamedResource(name, dataSizeInBytes);
session = Ort::Session(env, data, dataSizeInBytes, Ort::SessionOptions{});
```

### Inference
```cpp
Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(...);
auto inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, I.data(), ...);
auto outputTensor = Ort::Value::CreateTensor<float>(memoryInfo, O.data(), ...);
session.Run(Ort::RunOptions{}, inputNames.data(), &inputTensor, 1, ...);
```

---

## Key Insights for Our Integration

### What We Need
1. **ONNX Runtime SDK**: C++ library (~300MB SDK, ~5MB linked)
2. **Model file**: `engineMain` (8KB) or train our own
3. **Adapter layer**: Convert their `pianoVoice` to our `MatildaNeuralVoice`

### What We Keep
✅ JUCE `SynthesiserVoice` interface (same as v2)  
✅ ADSR control (they have it, we can wire to ours)  
✅ Velocity sensitivity  
✅ Pitch wheel support (stubbed in theirs, we can add)  
✅ Polyphony (they use 10, we use 32)  

### What We Adapt
- **Effects chain**: Our tape/delay/reverb (they have none)
- **UI**: Our Figma design (theirs is different)
- **Parameters**: Our XY pad, ADSR knobs (theirs is minimal)
- **Branding**: Matilda Piano 3

### Challenges
1. **ONNX Runtime linking**: Need to find/install the SDK
2. **Model embedding**: Convert `engineMain` to BinaryData
3. **Thread safety**: Their async approach is good, adapt to our needs
4. **Blend ratio**: They blend neural+physical; we might go pure neural

---

## Simplified Approach

Instead of their hybrid, we can do **pure neural**:
1. Remove physical model fallback
2. Use only ONNX inference
3. Apply ADSR to neural output
4. Feed through our effects chain

This will be:
- Simpler to maintain
- More authentic piano sound
- No pitch-dependent blending needed

---

## Next Steps

1. **Install ONNX Runtime** (see ONNX-SETUP.md)
2. **Test minimal integration**: Load model, run inference
3. **Create `MatildaNeuralVoice`**: Adapt `pianoVoice` to our needs
4. **Embed model**: Use JUCE BinaryData for `engineMain`
5. **Wire to processor**: Replace `setupPhysicalEngine()`

---

## Resources

- **PianoForte repo**: https://github.com/tesserato/PianoForte
- **ONNX Runtime**: https://onnxruntime.ai/
- **ONNX Runtime C++ API**: https://onnxruntime.ai/docs/api/c/
- **Research paper**: "Efficient Algorithm for Quasi-Periodic Signals"

---

**Estimated Complexity**: Medium  
**Estimated Time**: 6-10 hours for full integration  
**Risk**: Low (proven architecture, permissive license)
