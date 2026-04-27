# Future Enhancement: Multiple Timbres

**Date:** 2026-04-28  
**Status:** Parked for future implementation  
**Priority:** Medium  
**Estimated Effort:** 2-4 hours for basic implementation

---

## Overview

The current v3.0.0 neural engine uses a single ONNX model (`engineMain`, 8.7KB). We can expand this to support multiple piano timbres for greater sonic variety.

---

## Option 1: Multiple Pre-trained Models (RECOMMENDED)

**Approach:** Add 3-5 different piano models and switch between them

**Sources:**
- PianoForte repository has multiple trained models
- Models available: Steinway D, Steinway B, Yamaha C5, vintage pianos

**Implementation:**
1. Download additional ONNX models from PianoForte releases
2. Embed in `BinaryData` (add to CMakeLists.txt `ASSET_FILES`)
3. Create selector in UI (dropdown or buttons)
4. Load selected model on user choice

**Code Changes:**
```cpp
// PluginProcessor.h
enum PianoModel { STEINWAY_D, STEINWAY_B, YAMAHA_C5, VINTAGE };
std::map<PianoModel, std::unique_ptr<NeuralModel>> models;
PianoModel currentModel = STEINWAY_D;

// Load all models at startup
models[STEINWAY_D] = std::make_unique<NeuralModel>("steinway_d");
models[YAMAHA_C5] = std::make_unique<NeuralModel>("yamaha_c5");

// Switch models
void setModel(PianoModel model);
```

**Storage Impact:** 5 models × 8KB = ~40KB total (negligible)  
**CPU Impact:** None (only one model active at a time)  
**User Experience:** Instant switching, professional variety

**Estimated Time:** 2-3 hours

---

## Option 2: Train Custom Models

**Approach:** Use PianoForte's training pipeline to create custom models

**Requirements:**
- High-quality piano recordings (WAV/FLAC)
- Python + PyTorch environment
- 2-4 hours training time per model
- Access to PianoForte training scripts

**Possibilities:**
- Custom piano recordings
- Processed/effected piano sounds
- Hybrid piano/synth timbres
- Vintage/prepared piano sounds

**Estimated Time:** 4-8 hours (including training)

---

## Option 3: Real-time Timbre Control

**Approach:** Add parameters that modify neural output harmonics

**Implementation:**
```cpp
// In MatildaNeuralVoice::computeNextSample()
float brightness = *brightnessParam; // 0-1 from UI
float warmth = *warmthParam;         // 0-1 from UI

for (size_t i = 0; i < currentAmplitudes.size(); ++i) {
    // Brightness: decay high harmonics
    float harmonicDecay = std::pow(1.0f - brightness, float(i));
    
    // Warmth: boost low harmonics
    float lowBoost = (i < 5) ? (1.0f + warmth * 0.5f) : 1.0f;
    
    currentAmplitudes[i] *= harmonicDecay * lowBoost;
}
```

**UI Changes:**
- Add "Brightness" knob (0-100%)
- Add "Warmth" knob (0-100%)
- Optionally add "Character" knob for mid-range emphasis

**Estimated Time:** 3-4 hours (including UI)

---

## Option 4: Model Blending (Advanced)

**Approach:** Run multiple models and blend outputs

**Implementation:**
```cpp
// Run two models
std::vector<float> modelA_amps, modelB_amps;
modelA->eval(input, modelA_amps);
modelB->eval(input, modelB_amps);

// Blend with user control
float blend = *blendParam; // 0-1
for (size_t i = 0; i < currentAmplitudes.size(); ++i) {
    currentAmplitudes[i] = modelA_amps[i] * blend + 
                           modelB_amps[i] * (1.0f - blend);
}
```

**CPU Impact:** ~2× inference cost (running two models)  
**User Experience:** Smooth morphing between piano types

**Estimated Time:** 4-6 hours

---

## Recommended Implementation Order

### Phase 1: Quick Win (2-3 hours)
1. Download 2-3 PianoForte models (Steinway, Yamaha, Vintage)
2. Embed in BinaryData
3. Add simple dropdown selector in UI
4. Test and verify

### Phase 2: Polish (2-3 hours)
5. Add more models (5-6 total)
6. Create better UI (buttons with piano names/icons)
7. Save model selection in preset
8. Document new models

### Phase 3: Advanced (Optional, 4-6 hours)
9. Add brightness/warmth controls
10. Experiment with model blending
11. Train custom models for unique sounds

---

## Resources Needed

**PianoForte Models:**
- Check releases: https://github.com/tesserato/PianoForte/releases
- Look for `.onnx` model files
- Typical size: 5-15KB each

**Documentation:**
- PianoForte training pipeline
- ONNX model format documentation
- Our integration docs in `v3-integration/`

**Testing:**
- Test each model loads correctly
- Verify audio quality
- Check CPU usage with multiple models
- Test model switching doesn't cause glitches

---

## Technical Notes

### Model Loading Strategy
```cpp
// Lazy loading (load on demand)
void PluginProcessor::setModel(PianoModel model) {
    if (!models[model]) {
        const char* modelName = getModelName(model);
        models[model] = std::make_unique<NeuralModel>(modelName);
    }
    
    // Update all voices to use new model
    for (int i = 0; i < synth.getNumVoices(); ++i) {
        if (auto* voice = dynamic_cast<MatildaNeuralVoice*>(synth.getVoice(i))) {
            voice->setModel(models[model].get());
        }
    }
}
```

### Voice Update Required
`MatildaNeuralVoice` needs a `setModel()` method:
```cpp
void MatildaNeuralVoice::setModel(NeuralModel* newModel) {
    model = newModel;
    // Resize amplitude vectors if new model has different output size
    targetAmplitudes.resize(model->outputShape[0]);
    currentAmplitudes.resize(model->outputShape[0]);
    // ... resize phases too
}
```

### UI Integration
Add to parameter layout:
```cpp
// In Parameters.cpp
static const juce::StringArray MODEL_CHOICES = {
    "Steinway D (Concert)",
    "Steinway B (Grand)", 
    "Yamaha C5 (Studio)",
    "Vintage Upright"
};

layout.add(std::make_unique<juce::AudioParameterChoice>(
    "MODEL", "Piano Model", MODEL_CHOICES, 0));
```

---

## Known Considerations

**Pros:**
- Small file size (models are tiny)
- No performance impact (one model at a time)
- Professional variety
- Easy to add more models later

**Cons:**
- Need to source/train models
- UI space for selector
- Testing each model
- Documentation for each piano type

**Risk Assessment:** Low - This is a well-proven approach used by PianoForte itself

---

## Success Metrics

- [ ] 3-5 distinct piano timbres available
- [ ] Instant model switching (no audio glitches)
- [ ] <5% CPU increase
- [ ] Model selection saved in presets
- [ ] Clear UI labeling of each piano type

---

**Status:** Documented, ready for implementation when desired  
**Next Steps:** Focus on XY pad fix (current priority)

---

## Reference Links

- PianoForte: https://github.com/tesserato/PianoForte
- ONNX Runtime: https://onnxruntime.ai/
- Our integration: `v3-integration/ARCHITECTURE-ANALYSIS.md`
