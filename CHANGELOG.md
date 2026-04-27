# Changelog

All notable changes to Matilda Piano are documented here.

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

[2.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v2.0.0
[1.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v1.0.0
