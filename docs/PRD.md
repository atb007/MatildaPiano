# Product Requirements Document (PRD) — Matilda Piano

**Version:** 2.0 (PRD addendum for physical engine)  
**Status:** Draft  
**Last updated:** 2026-04-24

**Milestone:** **M1–M2c** (v1 sampler + GUI) complete. **M3 / v2.0.0** — physical string engine; product **Matilda Piano 2** (side-by-side AU). See `docs/MILESTONES.md`.

---

## 1. Product overview

### 1.1 Name and tagline

- **Product name (v2):** **Matilda Piano 2** (AU display name; distinct plugin code from v1).  
- **Product name (v1):** Matilda Piano (sampler).  
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

**v2.0 (current product — Matilda Piano 2)**

- **Piano sound:** One main **physically inspired** tone: **Karplus–Strong / waveguide-style** string loop per voice with noise excitation (hammer) and velocity-sensitive level. **No sample files**; no disk scanning for WAV/AIFF in the core engine.
- **Pitch:** Equal temperament (A4 = 440 Hz); pitch wheel ±2 semitones (same JUCE convention as v1 voices).
- **Envelope:** Same **ADSR** parameters and ranges as v1 — applied as a **gain envelope on the physical voice output** so knob behaviour matches the previous release.
- **Polyphony:** 32 voices; voice stealing when exceeded.
- **Memory:** Per-voice delay buffers allocated in `prepareToPlay()` only (not per audio block).

**v1.0 (historical — sampler)**

- **Piano sound:** Driven by **sampled audio** from `keySamples` or user folders (see `version-1/` and `docs/architecture.md` historical section).
- **Sample source:** User-provided WAV/AIFF; loaded at init only.
- **Depth:** Simple velocity via sample playback; no sustain pedal in v1.
- **Polyphony / RAM:** Same 32-voice cap; samples in RAM.

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
  - On-screen piano keyboard at bottom for click-to-play and visual feedback. **Keyboard range:** **C0–C7** (MIDI 12–96). **v2:** engine responds to full MIDI note range 0–127 from the host; display unchanged.
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
- **Voice management:** Fixed number of `MatildaPhysicalVoice` instances (v2); JUCE `Synthesiser` handles note allocation and stealing.
- **File scanning (v2):** None for the core instrument. Optional `keySamples` copy in CMake is legacy / harmless for Standalone.
- **Error handling (v2):** Engine always registers one `MatildaPhysicalSound`; status line usually empty.
- **Performance:** Real-time safe audio path; reverb uses preallocated wet buffer; delay time updated per block (can be optimized with cached subdivision/BPM).

Detailed notes: `docs/architecture.md`.

---

## 5. Success criteria

**v2.0**

- Builds as **AU** and **Standalone** on macOS; installs as **Matilda Piano 2** without replacing v1.
- Loads in **GarageBand** and plays MIDI **without** sample folders.
- All controls (ADSR, Reverb, Delay, Master, XY) behave as in v1 (same parameter IDs and mapping).
- UI unchanged (Figma 1074×483).
- No crashes or audio dropouts under normal use (32-voice polyphony, typical buffer sizes).

**v1.0 (historical)**

- Plays when samples are present in configured folders; see archived milestone notes.

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
| 2.0     | 2026-04-24 | **M3:** Physical string engine; **Matilda Piano 2**; `PLUGIN_CODE` MtP2; side-by-side with v1. |
