# Matilda Piano v3 — Retrospective & v4 Rationale

**Date:** June 2026  
**Status:** v3 shipped; GarageBand piano-roll stability unresolved; v4 planning underway  
**Next:** See **[version4-prd.md](version4-prd.md)**

---

## Executive Summary

Matilda Piano v3 achieved authentic timbre via a **pure neural** ONNX engine (harmonic partials, learned amplitudes). Post-release testing in **GarageBand** exposed serious stability issues on **piano-roll MIDI** that live play and Musical Typing did not fully reveal. Separately, we documented why the **Schuck–Young / inharmonic additive** path from PianoForte was **intentionally not ported** in v3 — and why v4 reverses that decision.

---

## Why Schuck–Young Was Not Pursued in v3

### What PianoForte had (reference only)

The MIT-licensed PianoForte engine in `v3-integration/reference-source/` is **hybrid**, not pure neural:

| Layer | Role |
|-------|------|
| **ONNX model** | Predicts harmonic amplitudes from `[pitch, velocity, periodCount]` |
| **Schuck–Young stretching** | `partialFromMidiKey()` — partial frequency \(f_n \propto n\sqrt{1 + \beta n^2}\) |
| **Physical additive tables** | `G1F`, `G2F`, `G3F` — inharmonic ratio sets for low notes |
| **Alpha/beta blend** | Low notes → more physical; high notes → more neural |

None of the frequency-stretching or physical blend code was ported into `Source/`. Matilda v3 uses **integer harmonics** `(i+1) * step` and ONNX amplitudes only.

### Documented v3 decision (Phase 1, April 2026)

From `v3-integration/PHASE-1-SUMMARY.md` and `ARCHITECTURE-ANALYSIS.md`:

> **Use pure neural (no hybrid)** — simpler, easier to maintain; neural model should learn timbre.

**Rationale at integration time:**

1. **v2 physical modeling had failed the timbre bar** — Karplus-Strong sounded more dulcimer/harpsichord than grand piano; constant tuning battle. Bias against adding another physical layer.
2. **Scope pressure** — v3 integration was scoped at ~6–10 hours. ONNX wiring + ADSR + FX was the critical path; hybrid blend, queues, and `partialFromMidiKey` were cut.
3. **Bet on the model** — 30 ONNX partial amplitudes were assumed to encode piano character (including implicit inharmonicity); explicit stretched frequencies deemed unnecessary.
4. **Reference hybrid was risky** — PianoForte's `startNote` can spin on the audio thread waiting for inference while feeding a physical queue. Matilda already needed async inference; a second synthesis path added complexity.
5. **No UI surface** — Inharmonicity stayed engine-internal in v2; restoring Schuck–Young did not map to the existing knob layout.

### What we did *not* do

- We did **not** A/B test Schuck–Young additive in Matilda v3 and reject it on listening grounds.
- We **cut it at design time** when simplifying PianoForte → Matilda.

### v4 reversal

v4 moves to **Schuck–Young frequencies for partials, ONNX for amplitudes only**, plus an optional **low-note physical blend** from the reference — on a **non-blocking** inference path. See `docs/version4-prd.md`.

---

## Post-Release: GarageBand Piano-Roll Debugging (June 2026)

**Baseline reference commit (last known good live play):** `72a9237` — *Add VST3 plugin format alongside AU for macOS builds.*

Further fixes exist as **local/uncommitted** changes on `v3-neural-network` (not fully validated in GarageBand).

### Symptoms

| Symptom | Context |
|---------|---------|
| Screech then silence on Matilda track | Piano roll playback; sometimes entire GarageBand session mutes |
| Screech on empty clip at transport start | Delay default ON + BPM/playhead jump (partially fixed: delay default → Off) |
| Screech at **first note** in clip | ONNX inference completes → amplitudes jump ~10× above seeds in one block |
| Piano roll barely audible / silent | Per-block inference cap left most voices on whisper-quiet seeds |
| End-of-clip screech | `masterGain = 16 / activeVoices` spiked when voices dropped, amplifying FX tails |
| Single-note crash | Async thread wrote `targetAmplitudes` while audio thread read (data race) |

Classic Electric Piano on the same MIDI region worked — issue was **Matilda-specific** (neural engine + gain/FX staging).

### Root causes identified

| Cause | Mechanism |
|-------|-----------|
| CPU/thread storm | 32× `std::async` ONNX at clip start; audio thread blocking; clipping → host mutes all tracks |
| Delay length jump | Default delay ON + host tempo arrival caused instant delay-length change |
| ONNX handoff spike | Fast smoothing (800 Hz) converged in one block when amplitudes jumped |
| Inference starvation | Cap of 1 job/block when >8 note-ons left most voices on seed amplitudes |
| Post-FX gain spike | Voice-count compensation applied after FX without smoothing |
| Note pairing | Host MIDI at concert pitch; on-screen keyboard only +12 (correct split) |

### Fixes attempted (local, partial success)

**`MatildaNeuralVoice`:** single background inference worker + queue; thread-safe `acceptInferenceResult()`; no blocking `wait_for` in `startNote`; seed amplitudes; slower smoothing (120 Hz); per-voice `tanh` limiter; energy normalization; harvest blend toward ONNX (not instant snap).

**`PluginProcessor`:** combined synth bus gain with smoothed polyphony compensation; `sanitizeBuffer()`; transport-edge FX/voice reset; host MIDI untransposed, GUI keyboard +12 only.

**`DelayModule` / `Parameters`:** delay default Off; smoothed delay length; BPM updates only on meaningful change.

**Tests:** `runMidiRenderTest`, `runDenseMidiRenderTest`, `runOutOfRangeNoteTest` added — passed offline; GarageBand piano roll still failed at first note.

### What still worked

- Other tracks no longer died when Matilda misbehaved (session crash partially fixed)
- Empty clip + delay Off: no screech at transport start
- Musical Typing at various points during iteration

### What we must not reintroduce

- Blocking `wait_for(100ms)` on the audio thread in `startNote`
- Direct async writes to `targetAmplitudes` without mutex/generation handoff
- 32 simultaneous `std::async` inference threads
- Aggressive per-block inference cap of 1 job (caused silent piano roll)
- `masterGain = 16 / activeVoices` post-FX without smoothing
- Removing +12 transpose from **on-screen keyboard only** (Musical Typing regression)

---

## v3 vs v4 Engine Comparison

| Aspect | v3 (shipped) | v4 (planned) |
|--------|--------------|--------------|
| Partial frequencies | Integer harmonics `(n+1)·f₁` | Schuck–Young stretched partials |
| Partial amplitudes | ONNX (30 outputs) | ONNX only |
| Low-note body | Neural seeds + FX | Optional physical blend (reference `G1F`/`G2F`/`G3F`) |
| Inharmonicity control | Hidden / none | User knob → stiffness β |
| Inference | Async (evolved to worker queue locally) | Single worker; never block audio thread |
| MIDI octave shifts | Partial handling | Voice ID tracking, continuous f₁ map, partial culling |

---

## References

- `v3-integration/reference-source/Voices.h` — `partialFromMidiKey`, `G1F`/`G2F`/`G3F`
- `v3-integration/PHASE-1-SUMMARY.md` — pure neural decision
- `v3-integration/ARCHITECTURE-ANALYSIS.md` — hybrid architecture analysis
- `CASE-STUDY.md` — Part 2 (v3) and Part 10 (post-release)
- `docs/version4-prd.md` — v4 product requirements
