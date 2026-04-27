# Matilda Piano — Milestones

Track major project phases. Update this file when a milestone is completed or a new phase starts.

---

## Milestone 1: Functionality complete (frozen) — 2026-02-26

**Status:** ✅ Complete

**Scope:** Core instrument and plugin behaviour implemented and stable. No further functional changes until after GUI phase.

**Delivered:**
- **Processor:** APVTS, synth, tape/delay/reverb/gain chain; sample loading from keySamples (bundle) or user folders (~/Music/~Documents/MatildaPiano/Samples).
- **Sampler:** MatildaSamplerVoice / MatildaSamplerSound; 32 voices; ADSR per voice; 3 ms sample attack to reduce clicks.
- **DSP:** TapeModule, DelayModule (tempo-synced subdivisions), ReverbModule. Effect chain enabled by default (MATILDA_BYPASS_DSP_DEBUG=0); set to 1 only for debug bypass.
- **Parameters:** ADSR, reverb mix, delay time, master vol, XY X/Y — all host-automatable.
- **On-screen keyboard:** C0–C7 (MIDI 12–96); labels C0–C7 (setOctaveForMiddleC(4)); sample mapping 7 octaves (C1–C8 in keySamples; keys shown to C7).
- **Sample loading:** keySamples (octave 0–7 → C1–C8) or user folders; WAV/AIFF; note name or MIDI in filename. Status message when no samples found.
- **Build:** AU + Standalone; keySamples copied into Standalone bundle at build time.
- **Testing:** Unit tests (MatildaPianoTests); manual log (TESTING-LOG.md) with checklist and troubleshooting.

**References:** PRD §2, docs/architecture.md, docs/DEV-HANDOFF.md (repo state table).

---

## Milestone 2a: GUI alignment (visual) — 2026-02-26

**Status:** ✅ Complete

**Scope:** Visual alignment with Figma — layout, assets, knobs, XY pad. No change to frozen functionality.

**Delivered:**
- **Left panel:** Background image loads from BinaryData or `~/Documents/MatildaPiano/Assets/Images/` (copy via `scripts/copy-assets-to-documents.sh` for AU in GarageBand).
- **Knobs:** Chickenhead style in code; full outer ring, track + fill arc, line + centre dot; zero at 7 o'clock. ADSR: white ring, purple marker; FX: black base, orange marker.
- **XY pad:** Background image with aspect ratio preserved; rounded-rect clip; sliders are non-painted (`addChildComponent`) so no sharp rectangular frame; dot indicator on top.
- **Layout:** Figma 1074×483; dynamic font/layout scaling via `getFigmaScale()`; keybed, gradient, inner shadow.

**Reference:** Figma [AdMaker-CMS — frame 4203:94317](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317), docs/figma-ui.md, docs/FIGMA-SPEC.md.

---

## Milestone 2b: Fonts — 2026-02-27

**Status:** ✅ Complete

**Scope:** Fonts not rendering; troubleshoot and fix (Jacquard 24, Kode Mono, Inter) so title, labels, and “GRAND PIANO” match Figma. Assets in `Assets/Fonts/` or `~/Documents/MatildaPiano/Assets/Fonts/`; BinaryData or disk load.

**Reference:** docs/FIGMA-SPEC.md (§8 Fonts), docs/figma-ui.md (Testing the AU in GarageBand).

---

## Milestone 2c: Effect module + Delay Off + XY — 2026-02-27

**Status:** ✅ Complete

**Scope:** Enable full effect chain (tape, delay, reverb); delay knob lowest value = Off; XY pad effects active.

**Delivered:**
- **Effect chain:** MATILDA_BYPASS_DSP_DEBUG set to 0 — tape, delay, reverb, and master gain all run in processBlock.
- **Delay:** Lowest knob position (≤5%) = Off (mix 0, label "Delay (Off)"); remainder maps to subdivisions 1/64…1.
- **XY pad:** Already wired in updateParameters(); tape module now processes, so XY controls wow/flutter (X), saturation and tone (Y).

**Reference:** PluginProcessor.cpp (bypass define, updateParameters delay/XY), PluginEditor.cpp (updateDelayTimeLabel).

---

## Changelog

| Date       | Change                                                                 |
|------------|------------------------------------------------------------------------|
| 2026-02-26 | M1 functionality frozen; M2 GUI fixes planned.                         |
| 2026-02-26 | M2a GUI alignment complete (panel, knobs, XY pad, layout). M2b fonts next. |
| 2026-02-27 | M2b fonts: bundle Resources + flat/nested paths; BinaryData glob; deprecation fix. |
| 2026-02-27 | M2c: Effect module enabled (bypass=0); delay Off at minimum; XY pad effects active. |
| 2026-02-27 | **v1.0.0** tagged — milestone release; polyphony fix, delay/XY improvements; push to GitHub. |