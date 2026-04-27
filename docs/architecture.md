## MatildaPiano — code-level architecture

This document captures how the plugin is structured in code, with emphasis on threading, voice management, parameter mapping, error handling, and performance constraints.

**Project phase:** **v2.0.0** — physical string engine (Karplus–Strong style) replaces the v1 sampler. Milestones M1–M2c describe the original sampler + GUI work; see `docs/MILESTONES.md` for history and the v2 milestone.

### Strategy: single path (JUCE only)

- **Deliverable:** **Matilda Piano 2** — AU plugin on macOS (e.g. GarageBand), installable **side-by-side** with v1 (`PLUGIN_CODE` / bundle ID differ). This repo is the only codebase.
- **Iteration:** Use the **Standalone** target for fast UI/UX iteration without a DAW; treat it as the day-to-day prototype. Test in GarageBand when ready.
- **UI ↔ engine:** UI = `PluginEditor`, core = `PluginProcessor`; they connect via `AudioProcessorValueTreeState` and attachments. Layout follows Figma frame `4203:94317` (1074×483). **v2 keeps the same controls and parameter behaviour** as v1.

### High-level structure (current JUCE codebase)

- **Processor**: `Source/PluginProcessor.h/.cpp`
  - Owns parameters (`AudioProcessorValueTreeState`)
  - Owns `juce::MidiKeyboardState` (shared with editor for on-screen keyboard); injects keyboard state into MIDI in `processBlock()` so the synth responds to the GUI keyboard.
  - Owns `juce::Synthesiser` (voices + sounds)
  - Owns DSP chain modules: `TapeModule`, `DelayModule`, `ReverbModule`, `masterGain`
  - Pulls host tempo from `AudioPlayHead::getPosition()` → `PositionInfo::getBpm()` (not deprecated `getCurrentPosition`).
  - Calls `setupPhysicalEngine()` from the constructor: registers one `MatildaPhysicalSound` covering MIDI 0–127 (no disk I/O).
- **Editor/UI**: `Source/PluginEditor.h/.cpp`
  - Pure JUCE UI (sliders, labels, XY pad, MIDI keyboard)
  - Parameter binding via `AudioProcessorValueTreeState::SliderAttachment`
  - Uses pixel coordinates copied from Figma frame `4203:94317` (1074×483)
- **Physical engine / voices**
  - `Source/MatildaPhysicalVoice.*`: `juce::SynthesiserVoice` implementation — delay-line string loop, noise excitation “hammer”, per-voice `juce::ADSR` (same role as v1: shapes output; parameters from APVTS).
  - `Source/MatildaPhysicalSound.*`: `juce::SynthesiserSound` — `appliesToNote` for full MIDI range.
- **DSP modules**
  - `Source/TapeModule.*`: wow/flutter modulation + saturation + tone filter. IIR filter coefficients set via `toneFilter.coefficients = IIR::Coefficients<float>::makeLowPass(...)` (assign Ptr).
  - `Source/DelayModule.*`: tempo-synced delay using `dsp::DelayLine`. Subdivision table uses `const char*` for display (literal type for `static constexpr`).
  - `Source/ReverbModule.*`: `dsp::Reverb` with correct wet/dry mixing

### Threading model (critical)

JUCE plugins primarily run on two threads:

- **Audio thread**
  - Calls `MatildaPianoAudioProcessor::processBlock()`
  - Must be real-time safe:
    - No allocations
    - No file I/O
    - No locks
    - Avoid heavy per-sample work when possible
- **Message/UI thread**
  - Painting and UI input
  - Parameter changes sent to the processor via attachments and atomics

**Current behavior**:
- Parameter reads in `updateParameters()` use `getRawParameterValue(...)->load()` which is safe for the audio thread.
- `ReverbModule` preallocates a wet buffer during `prepare()` and reuses it (no per-block allocations under normal conditions).

**Engine init:** `setupPhysicalEngine()` runs on the message thread in the processor constructor (adds sounds only). `MatildaPhysicalVoice::setSampleRate()` runs in `prepareToPlay()` and **allocates/resizes** the per-voice delay line there — not on the audio thread during playback.

### Voice management

- Voices are created once:
  - `MatildaPianoAudioProcessor::MatildaPianoAudioProcessor()` adds `numVoices = 32` instances of `MatildaPhysicalVoice`.
- MIDI triggering:
  - `synth.renderNextBlock(buffer, midiMessages, ...)` handles note on/off and voice stealing internally.
- Envelope:
  - `MatildaPhysicalVoice` uses `juce::ADSR` and updates ADSR params from processor parameters (same IDs/ranges as v1).
- **Polyphony gain:** The synthesiser **sums** all voices into the same buffer; many notes → clip. After `renderNextBlock()`: apply gain **1/numVoices (1/32)** so 32 voices peak at 1.0 (no clamp there); then safety clamp to [-1, 1]. Master gain uses make-up (×16) so a single note stays audible; a final clamp after the full chain prevents output > 1.0 (see `PluginProcessor::processBlock()`).

### Parameter mapping

Defined in:
- `Source/Parameters.h/.cpp`

Bound in UI via:
- `SliderAttachment` for each knob
- `XYPadComponent` uses hidden sliders + attachments for `xyX` and `xyY`

Applied in audio thread:
- `MatildaPianoAudioProcessor::updateParameters()`
  - ADSR -> each voice
  - XY -> tape wow/flutter + saturation + tone cutoff; XY also adds reverb wash (reverbMix += xyY*0.5 + xyX*0.3) for watery, washed-out vibe
  - Delay knob:
    - Lowest ~5% of knob = **Off** (mix 0, label "Off"); remainder maps to musical subdivision
    - `delayTime` selects subdivision (1/64…1); mix derived from same knob: `mix = clamp(t * 0.6)` for normalized `t` above threshold
  - Reverb -> mix
  - Master -> gain

### Delay timing rules

Subdivision list is in:
- `Source/DelayModule.h` `subdivisions[9]`

Mapping:
- knob \(\le 0.05\) = Off (mix 0); above that, normalized range maps to index \(0..8\) for subdivisions 1/64…1
- host tempo (BPM) used to convert beats -> seconds -> samples
- **Max delay time capped at 1.0 s** so 1/2 bar and 1 bar do not repeat a full phrase (avoids “whole sequence” echo).

Performance note:
- `DelayModule::process()` currently calls `updateDelayTime()` each block. This is acceptable for a first version but can be optimized by caching the last subdivision index and last BPM.

### v1 sample-based engine (historical)

v1 loaded WAV/AIFF from `keySamples` or `~/Music|Documents/MatildaPiano/Samples`. That implementation lived in `MatildaSamplerVoice` / `MatildaSamplerSound` and `loadSamples()`. **v2 removes runtime sample dependency**; an archived copy of the v1 tree remains under `version-1/` for reference.

### Keyboard range and GUI labels (PRD §2.4)

The on-screen keyboard displays **C0–C7** (MIDI 12–96). Implemented via `setAvailableRange(12, 96)`, `setLowestVisibleKey(12)`, and **`setOctaveForMiddleC(4)`** so white keys are labelled C0, C1, … C7. The physical engine responds to the full MIDI range supported by `MatildaPhysicalSound` (0–127). Host MIDI outside the displayed range is still processed.

### Error handling strategy

- **v2:** No sample folders required. `sampleLoadStatus_` is usually **empty**; the editor’s amber status line appears only if a future engine sets a non-empty message.
- If a file can’t be decoded (N/A for core v2 engine): N/A.
- **Status hook:** `getSampleLoadStatus()` remains for UI compatibility.

### Performance constraints / rules of thumb

- **Audio thread**
  - Avoid allocations, file I/O, logging
  - Prefer per-block updates over per-sample where possible
- **Physical voices**
  - Per-sample work includes delay read/write and one-pole loop filter; CPU scales with polyphony (32 voices max).
- **DSP**
  - Reverb uses a preallocated wet buffer
  - Tape module is currently simple and may be CPU-heavy per sample; optimize later by:
    - processing modulation at lower rate
    - using vectorized blocks for saturation/filter when possible

### UI constraints

The UI is designed for a fixed frame size (matching Figma):
- Editor: **1074×483**
- Control placement: pixel-perfect bounds (see `MatildaPianoAudioProcessorEditor::resized()`)
- All text/labels are JUCE-rendered (not baked into the PNG), so they can be swapped later for custom fonts or localization.
- Fonts use `juce::Font(juce::FontOptions(...))` for JUCE 7/8 compatibility (deprecated `Font(float)` replaced). Custom fonts (Jacquard 24, Kode Mono, Inter) load from: (1) BinaryData (if embedded by CMake), (2) app bundle `Contents/Resources/Assets` when running Standalone, (3) `~/Documents/MatildaPiano/Assets`, (4) project `Assets/` from CWD. Both nested paths (`Fonts/Jacquard_24/Jacquard24-Regular.ttf`, `Fonts/Kode_Mono/static/KodeMono-Bold.ttf`) and flat paths (`Fonts/Jacquard24-Regular.ttf`, `Fonts/KodeMono-VariableFont_wght.ttf`) are tried so repo or bundle layout works.
