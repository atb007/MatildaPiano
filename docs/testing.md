# Testing — Matilda Piano

## Overview

Matilda Piano has two main testing surfaces:

| Type | Where | Purpose |
|------|--------|--------|
| **Unit tests** | `Tests/MatildaPianoTests.cpp` (target `MatildaPianoTests`) | Parameter layout, default values, IDs. Runs as a console app. |
| **Manual testing / observations** | **`docs/TESTING-LOG.md`** | Record Standalone and AU testing: sound, GUI, keyboard, XY pad, troubleshooting. Use the checklist there when audio or GUI keys don’t behave. |

---

## Unit tests

- **Source:** `Tests/MatildaPianoTests.cpp`
- **CMake target:** `MatildaPianoTests` (uses `juce_generate_juce_header(MatildaPianoTests)` and `#include <JuceHeader.h>`).
- **Scope:** Instantiates `MatildaPianoAudioProcessor` and checks that `getParameters()` returns 9 parameters with expected IDs (ADSR, reverb, delay time, master vol, XY X/Y) and that float defaults lie within their ranges.

### Build and run

After configuring with `JUCE_DIR` set:

```bash
cmake --build build --target MatildaPianoTests --config Release
```

Run the test executable (path depends on generator):

- **Unix Makefiles / Ninja:**  
  `./build/MatildaPianoTests_artefacts/Release/MatildaPianoTests`  
  or sometimes `./build/MatildaPianoTests`
- **Xcode:** Run the **MatildaPiano Tests** scheme, or run the built executable from the build directory.

**Success:** `All tests passed.`  
**Failure:** non-zero exit code and failure messages on stderr.

### What’s tested

| Area | Notes |
|------|--------|
| Parameter layout | 9 parameters, expected IDs, defaults in range (via processor). |

### Adding tests

- Add new test functions in `Tests/MatildaPianoTests.cpp` and call them from `main()`.
- For code that depends on the full plugin, the test target already links the processor and related sources; you can add more assertions or new test functions as needed.

---

## Manual testing (Standalone / AU)

- **Log:** **`docs/TESTING-LOG.md`**
- Use it to record each test session (build, sound, GUI, keyboard range, XY pad).
- **Quick checklist** at the top of that file: samples folder, rebuild/restart, which keys have samples (C4–B4), output device, master volume.
- When debugging “no sound when clicking GUI keys”, follow that checklist and note results in the log.

---

## References

- **Unit test details:** this file (`docs/testing.md`).
- **Manual test log and troubleshooting:** `docs/TESTING-LOG.md`.
- **Architecture (threading, sample loading, parameters):** `docs/architecture.md`.
