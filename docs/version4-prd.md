# Matilda Piano v4 — Product Requirements

**Status:** In development (branch `v4-schuck-young-hybrid`)  
**Branch target:** `v4-schuck-young-hybrid`  
**Prior art:** `docs/V3-RETROSPECTIVE.md`, `v3-integration/reference-source/Voices.h`

---

## 1. Strategic shift

Move from v3's **pure neural** framework (integer harmonics + ONNX amplitudes) to a **hybrid acoustic engine**:

- **Schuck–Young frequencies** for partial placement (stiff-string stretching)
- **ONNX for amplitudes only** (30 partial weights from embedded model)
- **Low-note physical blend** from PianoForte reference (`G1F` / `G2F` / `G3F` tables) — optional resonance layer when sustain pedal is engaged
- **Non-blocking inference** — single background worker; audio thread never waits on ONNX

This directly addresses the v3 bet that ONNX amplitudes alone were sufficient for piano character, and incorporates lessons from the GarageBand piano-roll stability work documented in `docs/V3-RETROSPECTIVE.md`.

---

## 2. Controls

### Row 1 — Expressive shape (white knobs)

Map ADSR to **physics of decay**, not generic synth envelope:

| Control | Behaviour |
|---------|-----------|
| **Attack** | Onset speed. 0% = crisp hammer strike; higher = soft ambient/cinematic onset |
| **Decay** | Core sustain length while held. High = 9-foot grand ring; low = damp upright |
| **Velocity** *(replaces Sustain)* | Dynamic sensitivity. Low = intimate/felt-like regardless of key force; high = full soft-to-growl range |
| **Release** | Damper behaviour on key-up — short = instant choke; long = natural string fade |

### Row 2 — Acoustic workspace (black knobs)

| Control | Behaviour |
|---------|-----------|
| **Inharmonicity** | Drives stiffness coefficient **B** in Schuck–Young formula. 0% = clean/electric; ~50% = grand stretch-tuning; 100% = rustic detune |
| **Reverb** | Internal space wet/dry mix |
| **Master Vol** | Overall output level |

### XY pad — performance morph

Because amplitudes come from ONNX, the XY pad morphs **piano character**, not static EQ:

| Axis | Range | Effect |
|------|-------|--------|
| **X — Hardness / timbre** | Soft/Felt ← → Bright/Pop | ONNX input conditioning; boosts higher partials toward the right |
| **Y — Cabinet resonance** | Dry ← → Sympathetic wash | Volume of low-note physical blend / sympathetic string model when sustain pedal is down |

---

## 3. Engine architecture

### 3.1 Partial frequency (Schuck–Young)

For MIDI note number `n`, compute fundamental:

\[
f_1 = 440 \cdot 2^{\frac{n - 69}{12}}
\]

Partial `k` uses stiff-string stretching (Schuck–Young):

\[
f_k = f_1 \cdot k \sqrt{1 + B \cdot k^2}
\]

where **B** is mapped from the Inharmonicity knob.

**Requirement:** Continuous function over full MIDI range — no hard-coded note zones. Octave shifts in the DAW must recalculate the entire partial framework instantly.

### 3.2 Partial amplitudes (ONNX only)

- Input: `[pitch, velocity, periodCount, XY features…]` (exact feature vector TBD)
- Output: up to 30 amplitude weights applied to the Schuck–Young partials
- **Octave-based culling:** for `n > 80` (threshold TBD), cap active partial count to avoid ultrasonic waste and keep inference fast during rapid octave changes

### 3.3 Low-note physical blend

- Port ratio tables from `v3-integration/reference-source/Voices.h` (`G1F`, `G2F`, `G3F`)
- Blend weight: note-height + Y-axis (cabinet resonance) + sustain pedal
- Synthesis on audio thread only; no blocking waits for ONNX

### 3.4 Inference threading

- Single `NeuralInferenceScheduler` worker (pattern from local v3 fixes)
- Thread-safe handoff: generation counter + mutex; audio thread reads only committed results
- **Forbidden:** `std::async` per voice, `wait_for` on audio thread, direct cross-thread amplitude writes

### 3.5 Gain staging

- Polyphony compensation **before** FX chain, with smoothed rise/fall (fast drop when voices increase, slow rise when they release)
- Per-voice `tanh` limiter; buffer `sanitizeBuffer()` for NaN/Inf
- Delay **Off by default**; smoothed delay length on tempo/BPM changes

---

## 4. MIDI & DAW guard rails

GarageBand and other hosts shift MIDI by ±12 when the user changes octave. Matilda v4 must handle this cleanly.

### 4.1 Continuous pitch map

Never hard-code coefficients to fixed MIDI zones. Every note-on recomputes `f_1` → Schuck–Young partials from the incoming note number.

### 4.2 Voice allocation — prevent stuck notes

**Problem:** User holds C (60), shifts octave up in DAW, releases key → host sends Note-Off for 72; voice on 60 never stops → buffer overload → screech.

**Requirements:**

- Track voices by internal voice ID, not only raw MIDI note number where host pairing can break
- Implement All Notes Off / panic on detected Note-On/Off sequence anomalies
- `allNotesOff()` on transport play/stop edges

### 4.3 ONNX scaling by register

Dynamic partial count cap for high notes (see §3.2). Background thread stays bounded under frantic octave button use.

### 4.4 Parameter smoothing on active voices

When fundamental or blend coefficients change mid-note, interpolate over a few milliseconds inside the audio block — no instantaneous frequency snaps during erratic MIDI.

### 4.5 Host vs on-screen keyboard

- **Host MIDI:** concert pitch, no transpose
- **On-screen keyboard / Musical Typing:** +12 semitones (preserve v3 fix)

---

## 5. Success criteria

| Criterion | Target |
|-----------|--------|
| GarageBand piano roll | No screech/silence on first note, dense chords, or clip boundaries |
| Musical Typing | Middle C at concert pitch; no stuck notes on octave shift |
| Offline tests | Existing + new render tests pass (`Tests/MatildaPianoTests.cpp`) |
| CPU | ≤ v3 typical (~5–10%) at 32 voices, 128-sample buffer |
| Side-by-side install | New `PLUGIN_CODE` **MtP4**, bundle ID `com.matildaaudio.matildapiano4` |

---

## 6. Out of scope (v4.0)

- Windows VST3 (may follow on separate milestone)
- Preset browser (factory presets)
- Multiple ONNX timbre models (future; XY morph is v4 scope)

---

## 7. References

- `docs/V3-RETROSPECTIVE.md` — why Schuck–Young was skipped in v3, GarageBand debugging log
- `CASE-STUDY.md` — Part 10 (post-release reality)
- `v3-integration/reference-source/Voices.h` — `partialFromMidiKey`, physical tables
- `v3-integration/PHASE-1-SUMMARY.md` — original pure-neural decision


## 8. Latest figma link to updated design(primarily for knobs, and label changes):
https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5033-96191&t=PWZt0nNnToYRvHvy-4