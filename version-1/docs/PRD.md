# Product Requirements Document (PRD) — Matilda Piano

**Version:** 1.0 (first version)  
**Status:** Draft  
**Last updated:** 2026-02-26

**Milestone:** **M1** complete (frozen). **M2a** GUI alignment complete. **M2b** fonts fixed. **M2c** effect module + delay Off + XY enabled (2026-02-27). See `docs/MILESTONES.md`.

---

## 1. Product overview

### 1.1 Name and tagline

- **Product name:** Matilda Piano  
- **Tagline:** A beautiful, responsive piano instrument with tape character and simple effects.

### 1.2 Vision

Matilda Piano is a **software instrument (synth)** that provides a playable piano sound with a single-screen, visually polished UI. It targets producers and composers who want a “load and play” piano with subtle tape/flutter character and basic time-based effects, without menu diving.

### 1.3 Target users (initial)

- **Primary:** Bedroom producers and composers using **GarageBand** or **Ableton Live** on **macOS**.
- **Skill level:** Beginner to intermediate; no deep sound-design required.
- **Use case:** Quick, musical piano parts with optional vibe (tape, reverb, delay).

### 1.4 Platform and format (v1)

- **Product deliverable:** **AU plugin on macOS** (e.g. GarageBand). This repo is the single codebase; use **Standalone** for fast UI/UX iteration.
- **Platform:** macOS (latest; e.g. 12.0+).
- **Plugin format:** **AU** (Audio Unit) for GarageBand and other AU hosts.
- **Standalone:** Yes — build includes a **Standalone** app for fast UI/UX iteration without opening a DAW.
- **Future (out of scope for v1):** Windows, VST3, CLAP, AAX.

---

## 2. Core features and scope

### 2.1 Instrument engine

- **Piano sound:** One main acoustic piano, driven by **sampled audio** (no physical modelling in v1).
- **Sample source:** **Open-source or user-provided** sample libraries only; no bundled commercial samples. Samples are **not** embedded in the binary; they are loaded from user-configurable folders at runtime.
- **Sample loading rules:**
  - Search paths: `~/Music/MatildaPiano/Samples`, then `~/Documents/MatildaPiano/Samples`.
  - Formats: WAV, AIFF.
  - Filename parsing: note names (e.g. `C4`, `F#3`, `Bb2`) or MIDI note numbers (e.g. `60`). Unparseable files are mapped across all notes as a fallback.
- **Depth (v1):** Simple — 1–2 velocity layers per note, no round-robins, **no sustain-pedal support**.
- **Polyphony:** Fixed cap (e.g. 32 voices); voice stealing when exceeded.
- **Memory model:** Samples **preloaded into RAM** (no disk streaming in v1).

### 2.2 Envelope and tone shaping

- **ADSR:** One row of four knobs (chicken-head style): **Attack, Decay, Sustain, Release** — applied to the amplitude envelope of each voice.
- **Ranges:** Attack/Decay/Release in seconds (e.g. 0–5 s); Sustain 0–1.

### 2.3 Effects chain

- **Tape / Flutter (XY pad):**
  - **X-axis:** Wow/flutter rate (0 = none, 1 = max).
  - **Y-axis:** Saturation and tone (0 = clean, 1 = saturated + darker).
  - Cursor/mouse controllable; parameters automatable.
- **Delay:**
  - Single knob controlling **delay time** as **musical subdivisions** (e.g. 1/64, 1/32, 1/16, 1/8, 1/8T, 1/4, 1/4T, 1/2, 1 bar). **Lowest knob position = Off** (mix 0; label shows "Delay (Off)").
  - **Display:** Label updates in real time as the user rotates the knob (e.g. "Delay (Off)" at minimum, then subdivision) (e.g. “Delay (1/4)”).
  - Tempo taken from host when available; otherwise default BPM.
  - Mix derived from the same control (no separate “delay mix” knob in v1).
- **Reverb:** One knob — reverb mix (0–100%).
- **Master volume:** One knob — output gain (0–100%).

### 2.4 User interface

- **Design source:** UI layout and look are defined in **Figma**; implementation in JUCE follows the same frame (reference: Figma file `AdMaker-CMS`, frame node `4203:94317`, 1074×483 px).
- **Text and labels:** All on-screen text (branding, instrument name, knob labels, delay subdivision) is **rendered by JUCE** (not baked into background art). This allows future font embedding and localization.
- **Controls:**
  - ADSR row (white chicken-head knobs).
  - FX row: Reverb, Delay (with live subdivision label), Master vol. (green chicken-head knobs).
  - XY pad for tape/flutter.
  - On-screen piano keyboard at bottom for click-to-play and visual feedback. **Keyboard range:** **C0–C7** (MIDI 12–96). Sampled range 7 octaves (C1–C8 in mapping; keys shown C0–C7).
- **Background art:** Loaded from embedded BinaryData (if `Assets/` has PNGs at build time) or from disk at `~/Documents/MatildaPiano/Assets/background.png` for quick iteration without rebuild.

### 2.5 Parameter mapping and automation

- All knobs and XY axes are **host-automatable** via `AudioProcessorValueTreeState`.
- Preset/state: Full state (all parameters) saved and restored with the host project.

---

## 3. Out of scope (v1)

- Sustain pedal / repedal behaviour.
- Round-robin or release samples.
- Multiple instruments or preset browser (beyond “Grand Piano” display).
- Windows build; VST3/CLAP/AAX.
- Disk streaming; sample sets requiring >RAM.
- Custom font embedding (optional later).
- Localization (structure is ready; no translated strings in v1).

---

## 4. Technical context

### 4.1 Stack

- **Framework:** JUCE 6+ / 7.
- **Build:** CMake; targets: **AU** and **Standalone**.
- **IDE:** VS Code + CMake (optional Xcode for debugging).
- **Design:** Figma (design shared via Figma MCP / export); assets optional (BinaryData or user folder).

### 4.2 Architecture (summary)

- **Processor:** Single `AudioProcessor` subclass; owns synthesiser, DSP chain (Tape → Delay → Reverb → Gain), and parameter state.
- **Threading:** Audio thread (processBlock only; no alloc/I/O/locks); UI thread for painting and controls.
- **Voice management:** Fixed number of `MatildaSamplerVoice` instances; JUCE `Synthesiser` handles note allocation and stealing.
- **File scanning:** Sample load at **initialization only** (e.g. constructor or first prepare); never on the audio thread.
- **Error handling (v1):** Missing/invalid samples: plugin loads, no sound; failed files skipped. Optional: status message for “no samples found” in a later iteration.
- **Performance:** Real-time safe audio path; reverb uses preallocated wet buffer; delay time updated per block (can be optimized with cached subdivision/BPM).

Detailed notes: `docs/architecture.md`.

---

## 5. Success criteria (v1)

- Builds as **AU** and **Standalone** on macOS from CMake.
- Loads in **GarageBand** and plays MIDI notes when samples are present in the configured folders.
- All controls (ADSR, Reverb, Delay, Master, XY) affect sound as specified; delay label reflects current subdivision (including "Off" at minimum). Effect chain (tape, delay, reverb) and XY pad are enabled by default.
- UI matches Figma layout (1074×483); all labels and branding rendered by JUCE.
- No crashes or audio dropouts under normal use (e.g. 32-voice polyphony, typical buffer sizes).

---

## 6. References

- **Figma UI:** [AdMaker-CMS — frame 4203:94317](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317)
- **Codebase layout:** `README.md`, `QUICKSTART.md`
- **Architecture:** `docs/architecture.md`
- **Figma integration:** `docs/figma-ui.md`

---

## 7. Changelog

| Version | Date       | Changes                                                  |
|---------|------------|----------------------------------------------------------|
| 1.0     | (current)  | First PRD from prior discussion.                         |
| 1.0     | 2026-02-26 | §2.4: Keyboard range C0–C7 (MIDI 12–96); 7 octaves sampled (C1–C8 in mapping). |
| 1.0     | 2026-02-26 | **Milestone 1:** Functionality frozen. Next: GUI fixes (see `docs/MILESTONES.md`). |
| 1.0     | 2026-02-27 | **M2c:** Effect module (tape, delay, reverb) and XY pad enabled; delay lowest position = Off. |
