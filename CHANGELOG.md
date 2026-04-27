# Changelog

All notable changes to Matilda Piano are documented here.

## [2.0.0] - 2026-04-24

### Matilda Piano 2 — physical string model

Major version: **sample-based engine replaced** by a **Karplus–Strong–style** string voice (`MatildaPhysicalVoice`) and full-range `MatildaPhysicalSound`. **UI, parameters, and effect chain behaviour are unchanged** from v1 so projects and muscle memory transfer cleanly.

### Added
- **Physical engine:** Per-voice delay-line string with noise excitation; pitch wheel; velocity-sensitive hammer level; note-dependent loop damping for brighter high notes vs warmer lows.
- **Side-by-side installs:** New **four-character** AU code **`MtP2`**, product name **Matilda Piano 2**, bundle ID **`com.matildaaudio.matildapiano2`**, CMake **`VERSION` 2.0.0**.

### Removed
- **`MatildaSamplerVoice` / `MatildaSamplerSound`** and **`loadSamples()`** from the main `Source/` tree (v1 snapshot remains under **`version-1/`** and tag **`v1.0.0`**).

### Documentation
- README, PRD, architecture, milestones, dev handoff updated for v2.

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
