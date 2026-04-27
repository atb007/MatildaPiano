# Matilda Piano v2 Freeze Summary

**Date:** 2026-04-28  
**Status:** ✅ v2.0.0 FROZEN — Ready for v3 development

---

## What Was Frozen

**Version 2.0.0** — Physical String Model (Karplus-Strong)

### Engine Characteristics
- Karplus-Strong delay-line string synthesis
- Inharmonicity modeling (string stiffness)
- Two-pole low-pass damping
- Velocity-dependent hammer brightness
- Percussive "thump" component (soundboard impact)
- Auto-recovery from signal explosions
- Soft clipping with tanh()
- Energy monitoring and reset

### Technical Parameters (Frozen)
```
Feedback gain:      0.983 - 0.990 (note-dependent)
Inharmonicity:      2.5 - 0.5 (high to low notes)
Hammer contact:     0.5 - 2 ms
Output gain:        2.2 - 3.7× (velocity-scaled)
Safety threshold:   |signal| > 4.0, energy > 1.8
```

### Limitations Identified
- **Timbre**: Sounds like plucked strings, not struck piano keys
- **Character**: More guitar/harp-like than grand piano
- **Tuning complexity**: Extensive parameter tweaking needed
- **Physical modeling limits**: Would need full soundboard/frame model for authentic piano sound

### What Works Well
✅ Stable synthesis (no crashes or hangs)  
✅ All 88 piano keys respond  
✅ Velocity sensitivity  
✅ ADSR envelope control  
✅ Effects chain integration  
✅ Musical Typing support (velocity capped)  
✅ Side-by-side installation with v1

---

## Git State

```
Branch: v2-physical-model
Tag: v2.0.0-frozen
Commit: 3b70d40
```

All source files, documentation, and build scripts updated for v2 frozen state.

---

## New Branch: v3-neural-network

**Created:** 2026-04-28  
**Purpose:** Neural network synthesis using PianoForte engine  
**Status:** Setup phase

### v3 Folder Structure
```
v3-integration/
├── README.md              # Overview
├── INTEGRATION-PLAN.md    # Step-by-step roadmap
├── LICENSE-PianoForte     # MIT license
└── STATUS.md              # Current status tracking
```

---

## Documentation Updates

All documentation frozen to reflect v2 state:

✅ `README.md` — Updated version, features, frozen status  
✅ `CHANGELOG.md` — v2.0.0 entry with limitations noted  
✅ `docs/MILESTONES.md` — M3 complete (frozen), M4 added (v3)  
✅ `docs/architecture.md` — Physical model documented  
✅ `docs/PRD.md` — v2 requirements captured  
✅ `docs/DEV-HANDOFF.md` — v2 handoff notes  
✅ Build scripts — All point to v2 artifacts  
✅ Test logs — v2 testing results

---

## Next Steps (v3 Development)

See `v3-integration/INTEGRATION-PLAN.md` for complete roadmap.

**Immediate priorities:**
1. Install ONNX Runtime SDK
2. Copy PianoForte source files
3. Create adapter layer (`MatildaNeuralVoice`)
4. Link ONNX Runtime in CMake
5. Test with pre-trained piano models

**Estimated timeline:** 10 hours (~1.5 days)

---

## Why Neural Network Approach?

**Problem:** Physical modeling is mathematically elegant but struggles to capture the full complexity of a struck piano string, including:
- Soundboard resonance and coupling
- Frame vibrations
- Hammer-string interaction nonlinearities
- Multi-dimensional damping

**Solution:** PianoForte uses micro neural networks (~8KB, 1500 params) trained on real piano recordings. This captures authentic timbre while remaining real-time capable.

**License:** MIT (Carlos Tarjano) — permissive, allows commercial use with attribution.

---

## Files That Changed

**Core Engine (New):**
- `Source/MatildaPhysicalVoice.h/cpp`
- `Source/MatildaPhysicalSound.h/cpp`

**Processor:**
- `Source/PluginProcessor.h/cpp` (wired physical engine)

**Build:**
- `CMakeLists.txt` (v2.0.0, MtP2 plugin code, new product name)

**Documentation (All Updated):**
- README, CHANGELOG, PRD, architecture, MILESTONES, DEV-HANDOFF
- BUILD-SIMPLE, QUICKSTART, TESTING-LOG

**v1 Archive:**
- `version-1/` folder (complete v1 snapshot for reference)

---

## Side-by-Side Installation

**v1.0.0** (if installed):
- Product: "Matilda Piano"
- Plugin Code: `MatP`
- Bundle ID: `com.matildaaudio.matildapiano`

**v2.0.0** (frozen):
- Product: "Matilda Piano 2"
- Plugin Code: `MtP2`
- Bundle ID: `com.matildaaudio.matildapiano2`

**v3.0.0** (planned):
- Product: "Matilda Piano 3" (or same as v2, TBD)
- Plugin Code: `MtP3` or `MtP2` (reuse v2 slot)
- Will decide during v3 development

---

## Resources

**Reference Implementation:**
`/Users/udai.deori/Desktop/CursorAI/VST/Matilda/PianoForte-reference/`

**Key Files to Study:**
- `Source/Voices.h/cpp` — ONNX voice implementation
- `Source/SopranoVoice.h` — Embedded model weights
- `Resources/*.onnx` — Pre-trained piano models

**Documentation:**
- PianoForte README: https://github.com/tesserato/PianoForte
- Research paper: IEEE/ACM TASLP 2022

---

## Success Criteria for v3

✅ Authentic grand piano timbre (Steinway/Yamaha quality)  
✅ All v2 features preserved (UI, effects, ADSR)  
✅ Real-time performance (no dropouts at 128 samples)  
✅ Proper MIT license attribution  
✅ Professional build quality  
✅ Complete documentation  

---

**Ready to begin v3 integration!** 🎹
