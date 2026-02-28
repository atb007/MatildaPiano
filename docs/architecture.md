## MatildaPiano — code-level architecture

This document captures how the plugin is structured in code, with emphasis on threading, voice management, parameter mapping, file scanning rules, error handling, and performance constraints.

**Project phase:** M1 complete (frozen). M2a + M2b + M2c complete (GUI, fonts, effect module + delay Off + XY). See `docs/MILESTONES.md`.

### Strategy: single path (JUCE only)

- **Deliverable:** AU plugin on macOS (e.g. GarageBand). This repo is the only codebase.
- **Iteration:** Use the **Standalone** target for fast UI/UX iteration without a DAW; treat it as the day-to-day prototype. Test in GarageBand when ready.
- **UI ↔ engine:** UI = `PluginEditor`, core = `PluginProcessor`; they connect via `AudioProcessorValueTreeState` and attachments. Layout follows Figma frame `4203:94317` (1074×483).

### High-level structure (current JUCE codebase)

- **Processor**: `Source/PluginProcessor.h/.cpp`
  - Owns parameters (`AudioProcessorValueTreeState`)
  - Owns `juce::MidiKeyboardState` (shared with editor for on-screen keyboard); injects keyboard state into MIDI in `processBlock()` so the synth responds to the GUI keyboard.
  - Owns `juce::Synthesiser` (voices + sounds)
  - Owns DSP chain modules: `TapeModule`, `DelayModule`, `ReverbModule`, `masterGain`
  - Pulls host tempo from `AudioPlayHead::getPosition()` → `PositionInfo::getBpm()` (not deprecated `getCurrentPosition`).
- **Editor/UI**: `Source/PluginEditor.h/.cpp`
  - Pure JUCE UI (sliders, labels, XY pad, MIDI keyboard)
  - Parameter binding via `AudioProcessorValueTreeState::SliderAttachment`
  - Uses pixel coordinates copied from Figma frame `4203:94317` (1074×483)
- **Sampler/Voices**
  - `Source/MatildaSamplerVoice.*`: voice + ADSR envelope
  - `Source/MatildaSamplerSound.*`: wrapper around JUCE `SamplerSound`
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

**Important note**: sample loading performs file scanning and decoding. It must not be moved into `processBlock()`; current implementation calls `loadSamples()` in the processor constructor, and should be considered “initialization only.”

### Voice management

- Voices are created once:
  - `MatildaPianoAudioProcessor::MatildaPianoAudioProcessor()` adds `numVoices = 32` instances of `MatildaSamplerVoice`.
- MIDI triggering:
  - `synth.renderNextBlock(buffer, midiMessages, ...)` handles note on/off and voice stealing internally.
- Envelope:
  - `MatildaSamplerVoice` uses `juce::ADSR` and updates ADSR params from processor parameters.
- **Polyphony gain:** The synthesiser **sums** all voices into the same buffer; many notes → clip → burst then flat “blank” sound. After `renderNextBlock()`: apply gain **1/numVoices (1/32)** so 32 voices peak at 1.0 (no clamp there); then safety clamp to [-1, 1]. Master gain uses make-up (×16) so a single note stays audible; a final clamp after the full chain prevents output > 1.0 (see `PluginProcessor::processBlock()`).

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

### Sample scanning rules (open-source libraries)

Sample loading **search order**:

1. **keySamples** — project folder used by the plugin:
   - **In app bundle:** `Contents/Resources/keySamples` (Standalone; CMake copies project `keySamples/` into the bundle when present).
   - **Next to the .app:** `keySamples` in the same directory as `Matilda Piano.app` (e.g. `build/MatildaPiano_artefacts/Release/Standalone/keySamples`).
   - **Naming:** note letter + optional `#` + octave index **0–7**, e.g. `c0.wav`, `c#5.wav`, `a6.wav`. Octave 0 = C1 = MIDI 24; octave 7 = C8 = MIDI 108. PRD: 7 octaves. Lowercase or uppercase.
2. **User folders** (if no keySamples found):
   - `~/Music/MatildaPiano/Samples`
   - `~/Documents/MatildaPiano/Samples`
   - **Naming:** note name (e.g. `Piano_C4.wav`) or MIDI number (e.g. `Piano_60.wav`); see below.

Supported formats (all locations): WAV (`.wav`, `.wave`), AIFF (`.aif`, `.aiff`).

**Sample duration (for upload / content):**
- **Recommended per-note length: 3–8 seconds.** Enough for natural decay; keeps load time and memory reasonable.
- **Maximum length used by the plugin: 30 seconds.** Samples are loaded with `maxSampleLengthSeconds = 30.0`; any extra is not played.

**Filename parsing (user folders only when keySamples not used):**
1. **Note name tokens**: `C4`, `F#3`, `Bb2` (case-insensitive)
2. **MIDI number tokens**: `0..127`
3. **Fallback**: if no note can be parsed, map the sample across all notes (debug-friendly but not musically correct)

### Keyboard range and GUI labels (PRD §2.4)

The on-screen keyboard displays **C0–C7** (MIDI 12–96). Implemented via `setAvailableRange(12, 96)`, `setLowestVisibleKey(12)`, and **`setOctaveForMiddleC(4)`** so white keys are labelled C0, C1, … C7. **Sample mapping** is unchanged (keySamples c0→C1 … c7→C8); keys C1–C7 have samples, C0 has none by default. Host MIDI outside the displayed range is still processed if samples exist.

### Error handling strategy

Current strategy:
- If sample folders don’t exist or no files found: **silent no-sound** (plugin loads, but plays nothing), and a **status message** is set for the UI.
- If a file can’t be decoded: it is skipped.
- **Status message:** The processor stores a `sampleLoadStatus_` string (e.g. “No samples found — add WAV/AIFF to …”). The editor reads it on the message thread and draws it in `paint()` when non-empty (bottom-left, amber text). Updated only in `loadSamples()` (constructor / init), so safe to read from the UI thread.

### Performance constraints / rules of thumb

- **Audio thread**
  - Avoid allocations, file I/O, logging
  - Prefer per-block updates over per-sample where possible
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

