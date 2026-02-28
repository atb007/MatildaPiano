# Changelog

All notable changes to Matilda Piano are documented here.

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

[1.0.0]: https://github.com/atb007/MatildaPiano/releases/tag/v1.0.0
