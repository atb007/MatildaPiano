# v3 Integration Status

**Last Updated:** 2026-04-28  
**Branch:** `v3-neural-network`  
**Status:** Setup phase

---

## Completed

✅ Frozen v2.0.0 (physical model) with git tag `v2.0.0-frozen`  
✅ Created branch `v3-neural-network`  
✅ Created `v3-integration/` folder structure  
✅ Documented integration plan (`INTEGRATION-PLAN.md`)  
✅ Copied PianoForte LICENSE (MIT)  
✅ Created TODO list for v3 development

---

## Next Steps

See `INTEGRATION-PLAN.md` for full details.

**Immediate next actions:**
1. Install ONNX Runtime SDK (via Homebrew or manual download)
2. Copy key source files from PianoForte-reference
3. Begin adapter layer development

---

## Files Created

```
v3-integration/
├── README.md              # Overview of v3 work
├── INTEGRATION-PLAN.md    # Detailed integration roadmap  
├── LICENSE-PianoForte     # MIT license from original project
└── STATUS.md              # This file
```

---

## Branch Structure

- `main` → v1.0.0 (sample-based, tag: `v1.0.0`)
- `v2-physical-model` → v2.0.0 (frozen, tag: `v2.0.0-frozen`)
- `v3-neural-network` → v3.0.0 (current, in development)

---

## Reference Materials

- PianoForte source: `/Users/udai.deori/Desktop/CursorAI/VST/Matilda/PianoForte-reference/`
- License: MIT (Carlos Tarjano)
- Repository: https://github.com/tesserato/PianoForte

---

## Key Decisions

**Why freeze v2?**  
Physical modeling produced stable, playable sound but timbre resembled plucked strings rather than struck piano keys. Neural network approach (PianoForte) offers authentic piano sound with proven results.

**Why PianoForte?**  
- Proven results with real piano timbre
- Tiny models (~8KB, 1500 params)
- MIT licensed (permissive)
- Active development and documentation
- Real-time capable

**What stays the same?**  
All UI, effects, parameters, and user-facing features remain unchanged. Only the core synthesis engine is replaced.
