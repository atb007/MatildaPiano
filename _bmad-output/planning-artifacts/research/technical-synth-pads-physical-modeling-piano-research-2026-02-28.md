---
stepsCompleted: [1, 2, 3, 4, 5, 6]
inputDocuments: []
workflowType: 'research'
lastStep: 6
research_type: 'technical'
research_topic: 'Synth pads + physical modeling piano for Matilda AU plugin'
research_goals: 'UI impact, architecture changes, implementation approaches'
user_name: 'jeremiah007'
date: '2026-02-28'
web_research_enabled: true
source_verification: true
---

# Technical Research: Synth Pads + Physical Modeling Piano for Matilda

**Date:** 2026-02-28  
**Author:** jeremiah007  
**Research Type:** Technical

---

## Executive Summary

Adding **template synth pads** and switching the piano engine from **sample-based to physical modeling** will require substantial changes across the Matilda plugin: a new multi-engine architecture, new DSP paths, and significant **UI redesign**. The current single-instrument, single-screen layout (1074×483 px) is not designed for multiple sound engines.

**Key findings:**

- **UI impact is high:** Instrument selector (tabs or dropdown), pad-specific controls (filter, LFO, waveform), and possibly a larger or resizable frame. The existing Figma spec assumes one instrument ("Grand Piano").
- **Physical modeling:** OpenPiano, JUCE delay-line tutorials, and Karplus-Strong/waveguide algorithms provide viable paths. CPU cost is higher than sampling; expect 2–5× more CPU per voice.
- **Synth pads:** Wavetable or subtractive synthesis fits well with JUCE; reference implementations exist (Fiore, Subsynth, JUCE wavetable tutorial).
- **Architecture:** Move from a single `juce::Synthesiser` to multiple engines (or a polymorphic voice system). Each engine needs its own parameters and routing.

**Recommendations:**

1. **Prioritize:** Implement synth pads first (lower risk, faster to ship); physical modeling second.
2. **UI:** Add an instrument selector and conditional control panels; update Figma before coding.
3. **Physical modeling:** Consider OpenPianoCore as a reference or dependency; evaluate waveguide vs finite-difference approaches.

---

## Table of Contents

1. Technical Research Scope Confirmation
2. UI Impact Analysis (Primary Focus)
3. Technology Stack and Synthesis Approaches
4. Architectural Patterns and Design
5. Implementation Approaches and Migration
6. Risk Assessment and Recommendations
7. Sources and References

---

## 1. Technical Research Scope Confirmation

**Research Topic:** Synth pads + physical modeling piano for Matilda AU plugin  
**Research Goals:** UI impact, architecture changes, implementation approaches

**Technical Scope:**

- Architecture: multi-engine design, voice routing, parameter management
- Implementation: JUCE synthesis patterns, DSP modules, migration from sampler
- Technology stack: JUCE, C++, physical modeling algorithms, wavetable/subtractive synthesis
- Integration: APVTS, host automation, preset/state handling
- Performance: CPU, memory, real-time safety

**Scope Confirmed:** 2026-02-28

---

## 2. UI Impact Analysis (Primary Focus)

### 2.1 Current UI State (Matilda v1.0)

From `docs/FIGMA-SPEC.md` and `Source/PluginEditor.h`:

| Element | Current Design | Constraint |
|---------|----------------|------------|
| Frame size | 1074 × 483 px | Fixed; Figma spec |
| Instrument label | "&lt; GRAND PIANO &gt;" | Single instrument only |
| Left panel | 409 × 483 px | Art/background |
| Knob console | ADSR (4) + FX (3) | Shared across instrument |
| XY pad | 198 × 192 px | Tape effect only |
| Keyboard | Full width, bottom | Shared |

**Implication:** The UI is built for one instrument. There is no instrument selector, no per-instrument control set, and no preset browser.

### 2.2 UI Changes Required for Synth Pads

| Change | Description | Figma/Code Impact |
|--------|-------------|-------------------|
| **Instrument selector** | Switch between Piano and Pad | New component: tabs, dropdown, or segmented control. Position: near "GRAND PIANO" or top-center. |
| **Pad-specific controls** | Filter cutoff/resonance, LFO rate/depth, waveform | New row or panel. Either: (a) swap ADSR for pad controls when Pad selected, or (b) add collapsible panel. |
| **Label update** | "&lt; GRAND PIANO &gt;" vs "&lt; PAD &gt;" | Dynamic label based on selected instrument. |
| **XY pad repurpose** | Pad: filter + LFO? Tape: keep for piano? | Decide: shared XY or per-instrument mapping. |
| **Layout** | More controls in same 1074×483 | Risk of clutter. Consider: resizable frame, tabbed panels, or reduced knob size. |

**Reference patterns:** Cherry Audio Quadra (preset toolbar), Audiofier Seven (layer icons, key switches), HISE ModulationMatrix (search/filter). Tabs or a segmented control are common for 2–4 instruments.

### 2.3 UI Changes Required for Physical Modeling Piano

| Change | Description | Figma/Code Impact |
|--------|-------------|-------------------|
| **Piano-specific controls** | Hammer hardness, string decay, body resonance (if modeled) | New knobs or replace some ADSR semantics. ADSR still applies to amplitude envelope. |
| **Sample status** | "No samples found" becomes irrelevant if PM-only | Remove or repurpose status message; e.g. "Physical model active". |
| **Optional hybrid** | PM + samples (layered) | More complex: two engines + mix control. |

Physical modeling alone needs fewer sample-related UI elements but may need new physical parameters.

### 2.4 Combined UI Impact (Pads + Physical Modeling)

If both features are added:

1. **Instrument selector:** Piano (PM) | Pad — 2–3 options.
2. **Conditional panels:**
   - **Piano (PM):** ADSR + physical params (hammer, decay) + FX + XY (tape).
   - **Pad:** Filter + LFO + waveform + ADSR (or simplified) + FX + XY (optional).
3. **Frame size:** 1074×483 may be tight. Options:
   - Increase height (e.g. 1074×600) for stacked panels.
   - Use tabs: "Piano" | "Pad" with different layouts per tab.
   - Collapsible sections to keep current size.
4. **Figma:** New frames for each instrument view; document layout and components in FIGMA-SPEC.md.

### 2.5 UI Implementation Checklist (JUCE)

- [ ] Add `instrumentType` parameter to APVTS (e.g. 0=Piano, 1=Pad).
- [ ] Create `InstrumentSelectorComponent` (tabs or `ComboBox`).
- [ ] Create `PadControlsPanel` (filter, LFO, waveform).
- [ ] Create `PianoPMControlsPanel` (hammer, decay, etc.) if PM is used.
- [ ] Show/hide panels based on `instrumentType` in `resized()`.
- [ ] Update "GRAND PIANO" label to reflect selection.
- [ ] Ensure all new parameters are host-automatable.
- [ ] Update `getStateInformation` / `setStateInformation` for new params.

---

## 3. Technology Stack and Synthesis Approaches

### 3.1 Physical Modeling for Piano

**Algorithms:**

- **Karplus-Strong:** Simple plucked string; delay line + lowpass. Good for bass/pluck; less realistic for full piano.
- **Waveguide synthesis:** Bidirectional delay lines; better for strings. JUCE has `dsp::DelayLine` and a [delay-line tutorial](https://docs.juce.com/master/tutorial_dsp_delay_line.html).
- **Finite difference (FD):** OpenPiano uses FD for the stiff string equation. More CPU, more realistic. [OpenPiano](https://github.com/michele-perrone/OpenPiano) (AGPL-3.0) separates core (OpenPianoCore) and JUCE (OpenPianoJUCE).
- **Modal synthesis:** Alternative to FD; discretizes modes instead of the PDE. Can be more efficient at low energy.

**References:**

- CCRMA: [Efficient Waveguide Synthesis of Nonlinear Piano Strings](https://ccrma.stanford.edu/~jos/pasp/Efficient_Waveguide_Synthesis_Nonlinear.html)
- Wikipedia: [Physical modelling synthesis](https://en.wikipedia.org/wiki/Physical_modelling_synthesis)
- JUCE: [Create a string model with delay lines](https://docs.juce.com/master/tutorial_dsp_delay_line.html)

**Implementation options:**

1. **Build from JUCE DSP:** Use `dsp::DelayLine`, `dsp::ProcessorChain`, filters. Start with Karplus-Strong or simple waveguide.
2. **Integrate OpenPianoCore:** Use as library; license is AGPL-3.0 (copyleft).
3. **Hybrid:** Keep sampler for v1 compatibility; add PM as alternative engine.

### 3.2 Synth Pads

**Approaches:**

- **Wavetable:** Pre-filled lookup tables; efficient. [JUCE wavetable tutorial](https://docs.juce.com/master/tutorial_wavetable_synth.html).
- **Subtractive:** Oscillators (sine, saw, square, triangle) + filter + ADSR. Common for pads.
- **Reference projects:** Fiore (subtractive), Subsynth (polyphonic subtractive with filter, ADSR, waveforms), [Wavetable Synth Tutorial](https://thewolfsound.com/sound-synthesis/wavetable-synth-plugin-in-juce/).

**Typical pad controls:** Oscillator waveform, filter cutoff, filter resonance, LFO rate/depth, ADSR (or simplified envelope).

### 3.3 JUCE Synthesis Architecture

- **`juce::Synthesiser`:** Manages `SynthesiserSound` + `SynthesiserVoice`. Add voices via `addVoice()`, sounds via `addSound()`.
- **Multi-engine:** Use separate `Synthesiser` instances per engine, or one `Synthesiser` with polymorphic voices (`MatildaSamplerVoice`, `MatildaPadVoice`, `MatildaPMVoice`).
- **MPE:** `juce::MPESynthesiser` for MPE; not required for basic pads/PM.

---

## 4. Architectural Patterns and Design

### 4.1 Current Architecture (Matilda)

```
PluginProcessor
├── juce::Synthesiser (32 × MatildaSamplerVoice)
├── MatildaSamplerSound (sample mapping)
├── TapeModule → DelayModule → ReverbModule → masterGain
├── APVTS (ADSR, reverb, delay, master, XY)
└── loadSamples() [init only]
```

Single engine, single voice type, shared FX chain.

### 4.2 Target Architecture (Pads + PM)

**Option A: Multiple Synthesisers**

```
PluginProcessor
├── synthPiano   (MatildaPMVoice or MatildaSamplerVoice)
├── synthPad     (MatildaPadVoice)
├── instrumentSelector → routes MIDI to active engine
├── TapeModule → DelayModule → ReverbModule → masterGain
├── APVTS (instrumentType, piano params, pad params, FX, XY)
└── loadSamples() [only if sampler kept]
```

**Option B: Single Synthesiser, Polymorphic Voices**

```
PluginProcessor
├── juce::Synthesiser
│   ├── MatildaSamplerSound / MatildaPMSound / MatildaPadSound
│   └── MatildaSamplerVoice | MatildaPMVoice | MatildaPadVoice
├── instrumentSelector → which Sound type is active
├── FX chain (unchanged)
└── APVTS (extended)
```

**Recommendation:** Option A is clearer for routing and per-engine parameters. Option B reuses JUCE voice allocation.

### 4.3 Parameter and State Management

- Add `instrumentType` (int or enum) to APVTS.
- Add pad params: `padFilterCutoff`, `padFilterRes`, `padLfoRate`, `padLfoDepth`, `padWaveform`.
- Add PM params (if used): `pmHammerHardness`, `pmStringDecay`, etc.
- Ensure `getStateInformation` / `setStateInformation` include all new params for host project save/load.

### 4.4 Threading and Real-Time Safety

- No allocations or file I/O in `processBlock()`.
- Parameter reads via `getRawParameterValue()->load()` (already used).
- PM and wavetable synthesis are deterministic; avoid locks in the audio path.

---

## 5. Implementation Approaches and Migration

### 5.1 Phased Implementation

| Phase | Scope | Effort |
|-------|-------|--------|
| **Phase 1: Synth pads** | Add pad engine, instrument selector, pad controls, UI updates | Medium |
| **Phase 2: Physical modeling** | Replace or complement sampler with PM engine | High |
| **Phase 3: Polish** | Presets, hybrid modes, UX refinements | Medium |

### 5.2 Synth Pads Implementation Steps

1. Create `MatildaPadVoice` and `MatildaPadSound` (oscillator + filter + ADSR).
2. Add pad parameters to `Parameters.h/cpp` and APVTS.
3. Add `MatildaPadVoice` instances to a second `Synthesiser` or to the existing one.
4. Implement MIDI routing: when `instrumentType == Pad`, route to pad engine.
5. Add `PadControlsPanel` in `PluginEditor` with filter, LFO, waveform.
6. Add instrument selector; show/hide panels by selection.
7. Update Figma and FIGMA-SPEC.md.

### 5.3 Physical Modeling Implementation Steps

1. Choose approach: waveguide (JUCE delay lines) vs OpenPianoCore vs custom FD.
2. Implement `MatildaPMVoice` (or adapt OpenPiano voice).
3. Add PM parameters to APVTS.
4. Replace or parallel `MatildaSamplerVoice` with PM voices.
5. Remove or repurpose sample-loading UI.
6. Tune and optimize CPU (consider per-voice cost).

### 5.4 CPU and Performance

- **Sampling:** ~0.5–2% CPU per voice (playback only).
- **Physical modeling:** 2–5× higher per voice; OpenPiano notes optimization is ongoing.
- **Wavetable/subtractive:** Similar to sampling; efficient.
- **Mitigation:** Limit PM polyphony, use simpler models for high notes, or offer quality/CPU trade-off.

---

## 6. Risk Assessment and Recommendations

### 6.1 Risks

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| UI clutter | High | Medium | Tabs, collapsible panels, or larger frame |
| PM CPU overload | Medium | High | Limit polyphony; offer simpler mode |
| PM sound quality | Medium | High | Use OpenPiano or proven algorithms; iterate |
| State/preset breakage | Low | High | Extend APVTS and state logic carefully |
| Figma drift | Medium | Low | Update FIGMA-SPEC.md with new layouts |

### 6.2 Recommendations

1. **Implement synth pads first** — Lower risk, faster feedback, reuses existing FX chain.
2. **Update Figma before coding** — Define instrument selector, pad panel, and layout for both instruments.
3. **Keep sampler as fallback** — Physical modeling can be an option; retain samples for users who prefer them.
4. **Evaluate OpenPiano** — If AGPL is acceptable, consider integrating OpenPianoCore for PM.
5. **Document UI changes** — Add a "Multi-instrument UI" section to FIGMA-SPEC.md.

---

## 7. Sources and References

### Primary Sources

| Source | URL | Use |
|--------|-----|-----|
| JUCE DSP Delay Line Tutorial | https://docs.juce.com/master/tutorial_dsp_delay_line.html | Waveguide/string synthesis |
| JUCE Wavetable Synth Tutorial | https://docs.juce.com/master/tutorial_wavetable_synth.html | Pad synthesis |
| OpenPiano (GitHub) | https://github.com/michele-perrone/OpenPiano | Physical modeling reference |
| CCRMA Waveguide Piano | https://ccrma.stanford.edu/~jos/pasp/Efficient_Waveguide_Synthesis_Nonlinear.html | Algorithm reference |
| JUCE Synthesiser Reference | https://docs.juce.com/master/classjuce_1_1Synthesiser.html | Architecture |
| Physical Modelling Synthesis (Wikipedia) | https://en.wikipedia.org/wiki/Physical_modelling_synthesis | Algorithm overview |

### Project References

- `docs/FIGMA-SPEC.md` — Current UI spec
- `docs/PRD.md` — Product scope
- `docs/architecture.md` — Current architecture
- `Source/PluginEditor.h`, `Source/PluginProcessor.h` — Current structure

---

**Technical Research Completion Date:** 2026-02-28  
**Document Status:** Complete  
**Source Verification:** All technical claims cited with current sources
