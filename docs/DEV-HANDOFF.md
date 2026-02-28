# Dev handoff — Matilda Piano (JUCE)

**For:** BMM Dev Agent (`/bmad-agent-bmm-dev`)  
**Use this file as the primary context when starting implementation. Keep documentation updated as you build.**

**GitHub:** `atb007` — use for remote URLs and releases (e.g. `https://github.com/atb007/MatildaPiano`).

**Milestone:** **M1** complete (frozen). **M2a** GUI alignment complete. **M2b** fonts fixed. **M2c** effect module + delay Off + XY enabled (2026-02-27). See `docs/MILESTONES.md`.

---

## 1. What you need from the user

- **JUCE:** The user must set **`JUCE_DIR`** to either:
  - A JUCE **install prefix** (directory containing `JUCEConfig.cmake`), or
  - The JUCE **source directory** (e.g. your `JUCE 2` folder); CMake will use `add_subdirectory` and build JUCE as part of the project.
- **User’s JUCE path (this machine):** `'/Users/udai.deori/Desktop/CursorAI/JUCE 2'` — use quoted in shell/CMake because of the space.

Nothing else is required to build; samples are loaded from user folders at runtime (see below).

---

## 2. Build and run

For designers / simple steps: **[BUILD-SIMPLE.md](../BUILD-SIMPLE.md)** and `./clean-and-build.sh` (full clean + build).

```bash
# Configure (set JUCE_DIR to the user’s JUCE path)
export JUCE_DIR=~/JUCE   # user replaces with actual path
cmake -B build -DJUCE_DIR="$JUCE_DIR" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

**Outputs:**
- **AU:** Copied to `~/Library/Audio/Plug-Ins/Components/MatildaPiano.component` (if COPY_PLUGIN_AFTER_BUILD is on).
- **Standalone:** `build/MatildaPiano_artefacts/Release/Matilda Piano.app` — use this for fast UI/UX iteration without a DAW.

**Run Standalone:**
```bash
open build/MatildaPiano_artefacts/Release/Matilda\ Piano.app
```

---

## 3. Repo state (what’s done vs next)

| Area | Status | Notes |
|------|--------|--------|
| CMake | Done | AU + Standalone; BinaryData optional when `Assets/` has PNGs. |
| Processor | Done | `PluginProcessor`: APVTS, synth, tape/delay/reverb/gain chain, `loadSamples()` from disk. Effect chain enabled (MATILDA_BYPASS_DSP_DEBUG=0); delay lowest position = Off; XY pad drives tape. |
| Parameters | Done | `Parameters.h/.cpp`: ADSR, reverb, delay time, master vol, XY X/Y. |
| Sampler | Done | `MatildaSamplerVoice` / `MatildaSamplerSound`; 32 voices; ADSR per voice. |
| DSP | Done | `TapeModule`, `DelayModule` (tempo-synced subdivisions), `ReverbModule`. |
| UI | M2a+M2b done | Figma 1074×483; gradient + left panel (BinaryData or ~/Documents/MatildaPiano/Assets); chickenhead knobs; XY pad; keyboard; dynamic scale. **Fonts:** Jacquard 24, Kode Mono, Inter load from BinaryData, bundle Resources (Standalone), user Assets, or project Assets (nested or flat paths). |
| Sample loading | Done | Scans `~/Music/MatildaPiano/Samples`, then `~/Documents/MatildaPiano/Samples`; WAV/AIFF; note name or MIDI number in filename. Recommended 3–8 s per note; max 30 s. |
| Keyboard range | Done | On-screen keyboard C0–C7 (MIDI 12–96). Sampled range per keySamples (C1–C8 in mapping; keys shown to C7). |
| Figma assets | Optional | Background: `Assets/background.png` (embed) or `~/Documents/MatildaPiano/Assets/background.png` (no rebuild). |
| Tests | Started | Unit: `Tests/MatildaPianoTests.cpp` + target `MatildaPianoTests`. Manual: `docs/TESTING-LOG.md` for Standalone/AU sessions and sound/GUI checklist. See `docs/testing.md`. |
| “No samples found” UI | Done | Processor exposes `getSampleLoadStatus()`; editor draws status in `paint()` when non-empty (bottom-left, amber). |
| Docs | Living | Update `docs/architecture.md`, `README.md`, `QUICKSTART.md`, `docs/testing.md`, `docs/TESTING-LOG.md` when you change behaviour or add features. |

**Build / toolchain (current):**
- CMake uses **C + CXX** (for JUCE subprojects). **JUCE_BUILD_EXTRAS=OFF** when using JUCE source to avoid juceaide; **juce_generate_juce_header** used for plugin and test target. Xcode SDK or Command Line Tools SDK set when using JUCE source. Scripts: `build.sh`, `clean-and-build.sh` (full clean + build; supports Xcode SDK).

**Suggested next steps:**
- **M2c (2026-02-27):** Effect module (tape, delay, reverb) fully enabled; delay knob lowest value = Off (mix 0, label "Off"); XY pad effects (wow/flutter, saturation, tone) active. See `docs/MILESTONES.md`.
- Add or extend tests when adding new behaviour; keep docs in sync (architecture, PRD, README, QUICKSTART, testing).

---

## 4. Key references (for implementation and docs)

| Doc | Path | Purpose |
|-----|------|--------|
| **Milestones** | `docs/MILESTONES.md` | M1 (functionality frozen), M2 (GUI fixes next). |
| PRD | `docs/PRD.md` | Product scope, features, platform (AU + Standalone only; no web). |
| Architecture | `docs/architecture.md` | Threading, voice management, parameters, sample scanning, error handling, performance. |
| Figma UI | `docs/figma-ui.md` | Figma link and node; UI size 1074×483; export paths for background. |
| Quick start | `QUICKSTART.md` | Build, run Standalone, add assets, add samples, test in GarageBand. |
| Testing | `docs/testing.md` | Unit tests: build and run MatildaPianoTests. |
| Testing log | `docs/TESTING-LOG.md` | Manual testing: checklist for audio/GUI keys, session notes, troubleshooting. |
| Main README | `README.md` | Project overview, structure, build, testing. |

**Figma design:** `https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317`

---

## 5. Documentation rules for the Dev Agent

- **When you change behaviour or add features:** update the relevant section in `docs/architecture.md` (and PRD/README/QUICKSTART if scope or user flow changes).
- **When you add or move files:** update project structure in `README.md` and, if applicable, `docs/architecture.md`.
- **When you fix or extend sample loading:** document any new paths, formats, or filename rules in `docs/architecture.md` (Sample scanning rules) and `QUICKSTART.md`.
- **When you add tests:** note them in the story file’s Dev Agent Record and in `docs/testing.md` (and README if useful). For manual testing findings, update `docs/TESTING-LOG.md`.

---

## 6. Strategy (no web prototype)

- **Single codebase:** JUCE only. AU on macOS (e.g. GarageBand) is the product; Standalone is for iteration.
- **No browser prototype.** All prior “Phase 0 web” references have been removed from `docs/architecture.md` and `docs/PRD.md`.
