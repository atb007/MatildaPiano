# Changelog

All notable changes to Matilda Piano are documented here.

## [Unreleased] — v3 stability work & v4 planning

### Documentation
- **`docs/V3-RETROSPECTIVE.md`** — Schuck–Young / hybrid cut rationale; GarageBand piano-roll debugging log; v4 direction
- **`docs/version4-prd.md`** — v4 product requirements (Schuck–Young + ONNX amplitudes + MIDI guard rails)
- **`CASE-STUDY.md`** — Part 2 (v3 hybrid omission), Part 10 (post-release GarageBand reality)

### v3 stability fixes (local, branch `v3-neural-network`)
- Single background ONNX inference worker (replaces per-voice `std::async`)
- Thread-safe amplitude handoff; no audio-thread blocking in `startNote`
- Smoothed synth bus gain; polyphony compensation before FX
- Delay default Off; smoothed delay length on tempo changes
- Host MIDI at concert pitch; on-screen keyboard +12 only
- Additional render tests in `MatildaPianoTests.cpp`

**Known issue:** GarageBand piano-roll first-note screech not fully resolved — v4 planned.

See **`docs/V3-RETROSPECTIVE.md`** for full detail.

---

## [3.0.0] - 2026-04-28

### Matilda Piano 3 — Neural Network Synthesis

**✅ Status**: Current release. Authentic piano sound via ONNX-based neural network.

### Added
- **Neural synthesis engine:** ONNX Runtime integration for authentic piano timbre
- **PianoForte-based:** Adapted from MIT-licensed PianoForte project by Carlos Tarjano
- **Async inference:** Non-blocking neural network inference with `std::future` for real-time performance
- **Tiny model:** 8.7KB ONNX model embedded in plugin binary (no external files)
- **32-voice polyphony:** Full polyphony with efficient neural synthesis
- **Side-by-side installs:** New plugin code **`MtP3`**, product name **Matilda Piano 3**, bundle ID **`com.matildaaudio.matildapiano3`**

### Technical Details
- ONNX Runtime 1.17.0 (ARM64/x86_64)
- Model: ~1500 parameters, ~8KB size
- Input: [pitch, velocity, periodCount] → Output: harmonic amplitudes
- Stereo synthesis with random phase spread
- ADSR envelope integration
- Effects: Tape → Delay → Reverb → Master (unchanged from v1/v2)

### Performance
- Real-time capable on Apple Silicon and Intel Macs
- No audio dropouts with 128-sample buffer size
- Low CPU usage (~5-10% typical)
- Instant note response

### Credits
- Neural engine based on PianoForte by Carlos Tarjano (MIT License)
- Research: "An Efficient Algorithm For Segmenting Quasi-Periodic Digital Signals Into Pseudo Cycles"

### Documentation
- Complete integration docs in `v3-integration/`
- Phase 1-2 summaries with architecture details
- ONNX Runtime setup guide
- README, PRD, architecture docs updated

---

## [2.0.0] - 2026-04-28 (Frozen - Physical Model)

### Matilda Piano 2 — Physical String Model (Frozen)

**Status**: Frozen. This version uses Karplus-Strong physical modeling. Development continues with v3 (neural network approach).

### Added
- **Physical engine:** Per-voice delay-line string with noise excitation, pitch wheel, velocity-sensitive hammer level
- **Side-by-side installs:** New four-character AU code **`MtP2`**, product name **Matilda Piano 2**, bundle ID **`com.matildaaudio.matildapiano2`**, CMake **`VERSION` 2.0.0**
- **Stability features:** Auto-recovery from signal explosions, energy monitoring, soft clipping with tanh()
- **Timbre improvements:** Inharmonicity modeling, two-pole damping, velocity-dependent brightness, percussive body thump

### Known Limitations
- **Timbre:** Physical model sounds more like plucked string than grand piano
- **Tuning:** Extensive parameter tuning needed for realistic piano timbre
- **Volume:** Requires careful gain staging due to feedback-based synthesis

### Technical Details
- Feedback gain: 0.983-0.990 (stable but limited sustain)
- Hammer excitation: 0.5-2ms contact time with Gaussian envelope
- Output gain: 2.2-3.7× with velocity scaling
- Safety: Energy tracking, explosion detection, auto-reset after 20 bad samples

### Removed
- **`MatildaSamplerVoice` / `MatildaSamplerSound`** and **`loadSamples()`** from the main `Source/` tree (v1 snapshot remains under **`version-1/`** and tag **`v1.0.0`**)

### Documentation
- README, PRD, architecture, milestones, dev handoff updated for v2

---

## [1.0.0] - 2026-02-27

### Milestone release — Matilda Piano v1.0

First versioned release. Core piano plugin with Figma-aligned UI, full effect chain, and sample-based synthesis.

### Added
- **Piano sampler:** 32 voices, ADSR per voice, 7-octave sample mapping (C1–C8)
- **Effect chain:** Tape (wow/flutter, saturation, tone) → Delay (tempo-synced, Off at min) → Reverb → Master
- **XY pad:** Drives tape effect (X = wow/flutter, Y = saturation + tone) and reverb wash for watery, washed-out vibe
- **UI:** Figma 1074×483; chickenhead knobs; custom keyboard; Jacquard 24, Kode Mono, Inter fonts
- **Build:** AU + Standalone for macOS; `./build.sh` with JUCE path fallback

### Fixed
- Polyphony gain (1/32) + master make-up to prevent burst when many keys held
- Delay: `popSample(-1)` to use configured delay time; reset in prepare; higher mix when on
- XY pad dot sync from params; reverb wash from XY for watery vibe

### Known issues
- Delay and XY pad may still need further tuning for desired "washed-out, fluttery" character

---

[3.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v3.0.0
[2.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v2.0.0
[1.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v1.0.0
