# Matilda Piano — Noob-Friendly Project Guide

This guide explains **why** we chose our tech stack, **how** the pieces work together, **why** the project is organized the way it is, and **what** critical issues we hit and how we fixed them. It’s written so you can follow what to do and why we do it that way.

---

## 1. What is Matilda Piano?

Matilda Piano is a **piano plugin** that runs inside music software (e.g. GarageBand) or as a **Standalone** app. You get:

- A **sampled piano** (real recordings per note)
- **ADSR** knobs (attack, decay, sustain, release)
- **Effects**: tape wobble (XY pad), delay, reverb, master volume
- A **custom UI** (knobs, XY pad, on-screen keyboard) that matches a Figma design

We built **one codebase** that produces both the **AU plugin** (for DAWs) and the **Standalone** app (for quick testing without opening a DAW).

---

## 2. Why This Tech Stack?

### 2.1 Why JUCE?

- **JUCE** is a C++ framework for **audio plugins** and **audio apps**. It already knows how to:
  - Talk to the system (macOS) and host (e.g. GarageBand) as an **AU**
  - Handle **audio** (sample rate, buffer size, real-time safe code)
  - Handle **MIDI** (notes from the keyboard or from the host)
  - Draw **UI** (windows, knobs, sliders, images)
- So we don’t reinvent plugin hosting, audio I/O, or UI from scratch. We focus on **our** logic: sampler, effects, and layout.

**In short:** We use JUCE because it’s the standard way to build pro audio plugins on macOS (and other platforms later). Without it we’d have to implement all the plugin/audio/MIDI/UI glue ourselves.

### 2.2 Why C++?

- **Audio code** must run in **real time**: no random delays, no allocations in the audio callback. C++ gives us control over memory and performance.
- JUCE is C++, so our plugin code is C++ too. We use **C++17** (set in CMake).

**In short:** C++ is what JUCE uses and what the industry uses for low-latency audio. We need that for a responsive piano.

### 2.3 Why CMake?

- **CMake** is a **build system**: it takes our source files + JUCE and produces the actual **Xcode project** or **Makefiles** that compile everything.
- JUCE’s official way to integrate is via **CMake**: we call `juce_add_plugin(...)`, `target_sources(...)`, `target_link_libraries(...)`, and JUCE’s CMake scripts do the rest (AU bundle, Standalone app, copy plugin after build, etc.).
- We don’t hand-edit Xcode projects; CMake generates them. That keeps builds consistent and scriptable (`./build.sh`).

**In short:** We use CMake because JUCE is designed to be used with it, and it lets us build both the plugin and the Standalone app from one set of instructions.

### 2.4 Why macOS and AU (Audio Unit) first?

- The **PRD** (product requirements) targeted **GarageBand** and **macOS** for the first version. **AU** is the native plugin format on macOS, so DAWs like GarageBand load our plugin as an AU.
- **Standalone** is the same code running without a host; we use it for fast UI/audio testing.

**In short:** We started on macOS with AU because that’s the platform and format we chose for v1; later we can add Windows/VST3/etc.

### 2.5 How we use Figma

- **Figma** is where the **UI design** lives (one frame: 1074×483 px). We don’t design the layout in code from scratch; we **implement** the Figma layout in JUCE (positions, colors, knobs, XY pad, keyboard).
- Exported assets (background, knobs, fonts) go into **Assets/** and are either **embedded** by CMake into the binary (BinaryData) or loaded from disk (e.g. `~/Documents/MatildaPiano/Assets`).

**In short:** Figma is the single source of truth for how the plugin looks; we copy that look and behavior into JUCE.

---

## 3. How the Components Work Together

The plugin has a clear split: **audio engine** vs **UI**.

### 3.1 The big picture

```
[ Host (e.g. GarageBand) ]
         │
         ▼
[ PluginProcessor ]  ← Audio + MIDI + parameters + DSP chain
         │
         ├── Parameters (ADSR, reverb, delay, XY, master)
         ├── Synth (MatildaPhysicalVoice + MatildaPhysicalSound) — v2 physical string model
         ├── TapeModule → DelayModule → ReverbModule → masterGain
         └── MidiKeyboardState (shared with editor for on-screen keys)
         │
         ▼
[ PluginEditor ]     ← UI only (knobs, XY pad, keyboard, paint)
         │
         └── SliderAttachment / XYPad → APVTS (parameters)
```

- **PluginProcessor** = the “brain”: it owns parameters, the synth, and the effect chain. It runs on the **audio thread** when the host calls `processBlock()`.
- **PluginEditor** = the “face”: it draws the UI and binds knobs/sliders to parameters. When you turn a knob, the editor updates the **parameter** in the processor; the processor reads those parameters in `processBlock()` and applies them (ADSR, tape, delay, reverb, gain).

### 3.2 Parameters (APVTS)

- **APVTS** = **AudioProcessorValueTreeState**. It’s JUCE’s way to store **all** our knobs (attack, decay, sustain, release, reverb, delay time, XY X/Y, master) in one place.
- Benefits:
  - **Host automation**: The DAW can read/write these parameters (automation, save/load project).
  - **Thread-safe**: The UI thread writes, the audio thread reads (via `getRawParameterValue()->load()`), so no crashes.
- We define parameter IDs and ranges in **Parameters.h/cpp** and create attachments in the **PluginEditor** (e.g. `SliderAttachment(apvts, "attack", attackSlider)`).

**In short:** All knobs and the XY pad are “parameters.” The editor changes parameters; the processor uses them every block to shape sound.

### 3.3 Physical engine (voices + sounds) — v2

- **MatildaPhysicalSound**: one `SynthesiserSound` that applies to **MIDI 0–127** (no per-note WAV mapping).
- **MatildaPhysicalVoice**: one “voice” with a **delay-line string loop** (Karplus–Strong style), noise excitation, and the same **ADSR** as before on the output. We create **32** voices for polyphony.
- The **processor** owns a `juce::Synthesiser`, adds 32 `MatildaPhysicalVoice` instances, and registers one `MatildaPhysicalSound` in `setupPhysicalEngine()` (no disk load).

**In short:** v2 generates piano-like tones in real time; knobs still shape ADSR and effects. The old sampler lives in **`version-1/`** if you need WAV-based behaviour.

### 3.4 DSP chain (Tape → Delay → Reverb → Gain)

- **TapeModule**: wow/flutter (wobble), saturation, and a tone filter. Driven by the **XY pad** (X = wobble, Y = saturation/darkness).
- **DelayModule**: tempo-synced delay (1/64 … 1 bar). **Lowest knob position = Off** (no delay). Uses JUCE’s `dsp::DelayLine`; we use `popSample(channel, -1.0f)` so the delay time we set is actually used.
- **ReverbModule**: JUCE’s `dsp::Reverb` with wet/dry mix.
- **Master gain**: final volume. We also apply **polyphony gain** (1/32) so 32 voices don’t clip, then a **make-up** so one note is still loud enough.

All of this runs inside **processBlock()**: synth renders into a buffer, then we run tape → delay → reverb → gain on that buffer.

**In short:** After the synth, audio goes through tape (XY), then delay (knob), then reverb (knob), then master volume. Each module is a small class we call from the processor.

### 3.5 On-screen keyboard

- The **editor** has a **MatildaKeyboardComponent** (JUCE keyboard). When you click a key, it doesn’t send MIDI to the host; instead we **inject** that note into the **MidiKeyboardState** and then the processor pulls it into the MIDI buffer in `processBlock()` with `keyboardState.processNextMidiBuffer(...)`. So the synth plays from both host MIDI and GUI key clicks.

**In short:** The on-screen keys update a shared “keyboard state”; the processor turns that into MIDI and feeds the synth so you hear the note.

---

## 4. Why Is CMakeLists.txt Required?

**CMakeLists.txt** is **the** file that tells CMake (and thus the compiler) what to build and how. It’s not optional; without it we don’t have a defined project.

### 4.1 What it does (in plain terms)

1. **Sets language and standard**  
   We need C++17. CMake sets that so every source file is compiled with the right standard.

2. **Finds or embeds JUCE**  
   We set `JUCE_DIR` to either:
   - A JUCE **install** (with `JUCEConfig.cmake`), or  
   - The JUCE **source** folder; then CMake does `add_subdirectory(JUCE)` and builds JUCE as part of our project.  
   That’s why we have logic for “if JUCEConfig.cmake exists, use it; else add_subdirectory.” It’s a technical requirement so the same project works with both an installed JUCE and a source copy.

3. **Defines the plugin target**  
   `juce_add_plugin(MatildaPiano ...)` creates the actual plugin (AU + Standalone). It sets:
   - Plugin name, manufacturer, format (AU), Standalone
   - That it’s a synth, needs MIDI input, etc.  
   So the host and the OS see **“Matilda Piano 2”** (v2) correctly; v1 was “Matilda Piano.”

4. **Lists our source files**  
   All `.cpp` and `.h` in **Source/** are added to the target. If we add a new file, we add it to CMakeLists.txt so it gets compiled.

5. **Links JUCE modules**  
   We need juce_audio_basics, juce_audio_processors, juce_dsp, juce_graphics, juce_gui_*, etc. CMakeLists.txt says “link these to MatildaPiano.” Without that, we’d get “undefined symbol” errors.

6. **Embeds assets (BinaryData)**  
   It **globs** PNG/SVG/fonts from **Assets/** and tells JUCE to generate **BinaryData** (a C++ header with the bytes). The plugin then uses `BinaryData::background_png` etc. So CMake is what turns “files in Assets/” into “data inside the binary.”

7. **macOS-specifics**  
   For Apple builds it sets:
   - Architectures (arm64, x86_64)
   - Bundle ID, deployment target (e.g. 12.0)
   - **Copy keySamples** into the Standalone bundle if the folder exists (optional; **v2** does not use samples)
   - **Copy Assets** into the bundle (for fonts/images when BinaryData names differ)

8. **Unit test target**  
   It adds **MatildaPianoTests** (the test executable) and links the same source so tests can use our Parameters, processor, etc.

### 4.2 Why not just “open a project in Xcode”?

- We **could** use a hand-made Xcode project, but:
  - JUCE’s recommended and supported way is **CMake**.
  - CMake **regenerates** the Xcode project from one file; adding a new source file or asset is one line in CMakeLists.txt, not clicking through Xcode.
  - Our **build.sh** and **clean-and-build.sh** assume CMake. So CMakeLists.txt is required for **our** workflow and for JUCE’s plugin macros.

**In short:** CMakeLists.txt is required because it’s the single place that defines “what is the plugin, what sources, what JUCE modules, what assets, what platforms.” It’s a technical necessity for how JUCE and our build process work.

---

## 5. Why Directory / Folder Structure Matters

A clear layout helps us (and tools) know **where to find what** and **what each folder is for**.

### 5.1 Root

- **CMakeLists.txt**, **build.sh**, **clean-and-build.sh**: build entry points. Everyone expects these at the root.
- **README.md**, **CHANGELOG.md**, **BUILD-SIMPLE.md**: first thing someone sees; root is standard for “project overview” and “how to build.”

### 5.2 Source/

- **All** plugin C++ code lives here. CMakeLists.txt points at `Source/*.cpp` and `Source/*.h`. If we put code somewhere else without adding it to CMake, it won’t be compiled.
- Naming: **PluginProcessor** = main audio processor; **PluginEditor** = main UI; **Parameters** = parameter IDs and creation; **MatildaPhysical*** = physical string engine; **TapeModule**, **DelayModule**, **ReverbModule** = effects; **XYPadComponent**, **ChickenHeadKnob**, **MatildaKeyboardComponent** = UI widgets. So the **folder** (Source/) and **names** tell you “this is the plugin implementation.”

### 5.3 Tests/

- **Unit tests** (e.g. MatildaPianoTests.cpp) live here. CMake adds a **separate** target (MatildaPianoTests) that links our Source files. Keeping tests in **Tests/** (not Source/) avoids shipping test code inside the plugin and keeps “what is the product” vs “what tests the product” clear.

### 5.4 Assets/

- **Figma exports**: PNGs (background, knobs), SVGs, fonts. CMake **globs** this folder for BinaryData. So “put new assets in Assets/” is the rule; if we scattered them, we’d have to list every path in CMake. One folder = one place for “things we embed or copy.”

### 5.5 keySamples/

- **v1 only:** Piano WAVs (e.g. c0.wav, c#5.wav). CMake may still copy **`keySamples/`** into the Standalone bundle if present; **v2 ignores them** for the core instrument. See **`version-1/`** for the sample-based workflow.

### 5.6 docs/

- **Architecture**, **PRD**, **Figma spec**, **testing log**, **milestones**. Keeping them in **docs/** means:
  - The codebase root isn’t cluttered with dozens of .md files.
  - New devs (or agents) know “documentation is in docs/.”
  - README can say “see docs/ for details” and we have one place to look.

### 5.7 scripts/

- **fetch-piano-samples.sh**, **normalize-piano-samples.sh**, **copy-assets-to-documents.sh**: helper scripts for samples and assets. Putting them in **scripts/** keeps the root clean and makes it obvious these are “tools,” not the main build.

**In short:** Directory structure matters so that (1) the build system knows what to compile and where to find assets/samples, (2) docs and tests are in one place, and (3) anyone opening the project can quickly find “source,” “tests,” “assets,” and “docs.”

---

## 6. Critical Errors We Hit and How We Fixed Them

These are real issues we ran into; the fixes are in the codebase and in **docs/TESTING-LOG.md**. Here they are in a noob-friendly form.

### 6.1 No sound when clicking keys

- **What happened:** You open the plugin, click the on-screen keyboard, and hear nothing.
- **Causes we found:**
  1. **No sample folder or no WAVs:** The plugin only loads from **existing** folders with WAV/AIFF. If `~/Documents/MatildaPiano/Samples` (or keySamples in the bundle) doesn’t exist or is empty, the synth has no sounds → silence.
  2. **Samples cleared on audio reconfig:** We used to call `synth.clearSounds()` in `releaseResources()`. When the host or Standalone reconfigures the audio device (e.g. you open audio settings), it can call `releaseResources()`, which was wiping all loaded samples. So after that, no sound until restart.
- **Fixes:**
  - Create the Samples folder and add WAVs (or use **keySamples/** and rebuild so CMake copies them into the Standalone bundle).
  - **Stop clearing sounds in releaseResources()** so reconfig doesn’t wipe samples. Restart the app after adding samples or changing keySamples.

### 6.2 Ear-splitting “pop” on one key, then silence

- **What happened:** One key press gave a sharp, painful burst, then no sustained note.
- **Cause (v1):** The **ADSR** in the voice was **never given a sample rate**. JUCE’s ADSR needs `setSampleRate()` so its timing is correct. Without it, the envelope could fire to full instantly (the “pop”) and then behave wrong (silence). **v2:** `MatildaPhysicalVoice::setSampleRate()` sets ADSR in `prepareToPlay()`.
- **Fix:** In the voice class we added `setSampleRate(double)` and call `adsr.setSampleRate(sampleRate)`. In the processor’s `prepareToPlay()` we call `setSampleRate(sampleRate)` on every voice. After that, notes have proper attack/sustain/release.

### 6.3 Meter moves but no sound

- **What happened:** Level meter shows activity when you play, but you hear nothing.
- **Causes:** (1) Samples were cleared (see above). (2) **Wrong output device**: Standalone was sending audio to a different device than the one you’re listening on. (3) **Master volume** at zero.
- **Fixes:** Don’t clear sounds in releaseResources; in Standalone set **Audio → output** to the device you use; raise the Master vol. knob.

### 6.4 Only one octave plays after adding full keySamples

- **What happened:** You added WAVs for all keys in keySamples and rebuilt, but only one octave still played.
- **Cause:** Samples are loaded **only at startup**. The **running** app still had the old set in memory. Also you must launch the **new** .app from the **build** folder so it uses the bundle that has the updated keySamples (CMake copies at build time).
- **Fix:** Fully **quit** Standalone, run **./build.sh**, then launch **from** `build/MatildaPiano_artefacts/Release/Standalone/Matilda Piano 2.app` (or `.../Release/Matilda Piano 2.app` depending on JUCE layout). Don’t use an old .app from somewhere else.

### 6.5 Polyphony: loud burst then “goes blank”

- **What happened:** Hold many keys; volume suddenly jumps then goes flat/blank.
- **Cause:** The synth **sums** all voices. With 32 notes and no per-voice gain we had 32× level → **clipping** (hard clamp to ±1) → square-wave distortion (burst then flat).
- **Fix:** Apply **polyphony gain** = 1/32 so 32 voices peak at 1.0 and we don’t hit the clamp. Then **master make-up** (e.g. ×16) so a single note is still loud. Final **safety clamp** after the full chain so we never send > 1.0 to the host.

### 6.6 Delay “doesn’t work” or sounds wrong

- **What happened:** Delay didn’t seem to apply or gave weird timing.
- **Causes we found:**
  1. **popSample(channel, 0.0f)** meant “0 samples delay” (instant). JUCE expects **-1** to mean “use the delay time we set.”
  2. No **reset** of the delay line when sample rate or buffer changed → leftover state.
  3. 1/2 bar or 1 bar delay could repeat a whole phrase (confusing).
- **Fixes:** Use **popSample(channel, -1.0f)**; call **delayModule.reset()** in **prepareToPlay()**; **cap** max delay time at 1.0 s; only call **setDelay()** when the computed delay in samples actually changes (to avoid zipper noise).

### 6.7 XY pad “not audible” or not “washed out”

- **What happened:** Moving the XY pad didn’t change the sound much.
- **Fixes:** (1) **Dot sync**: make the XY dot reflect the current parameter values (and saved/automated state) in paint/resized. (2) **Stronger tape**: increase wow/flutter and saturation so X and Y are clearly audible; add **reverb wash** from XY (e.g. reverb mix += f(xyY, xyX)) for a “watery” vibe.

### 6.8 Build: “algorithm not found” or “juceaide” fails

- **What happened:** Compiling with JUCE **source** (add_subdirectory) and **Command Line Tools** (no full Xcode) could miss C++ headers or try to build juceaide and fail.
- **Fix:** In CMake we (1) set **JUCE_BUILD_EXTRAS OFF** when using JUCE source so we don’t build juceaide, and (2) set **CMAKE_OSX_SYSROOT** to the Command Line Tools SDK when needed. For a clean slate, use **./clean-and-build.sh** (and point it at Xcode if you have it: `sudo xcode-select -s /Applications/Xcode.app/Contents/Developer`).

### 6.9 GarageBand: plugin doesn’t see keySamples

- **What happened:** In Standalone, keySamples from the bundle work; in **GarageBand** (AU), no sound or only one octave.
- **Cause:** When the plugin runs inside GarageBand, the “app bundle” is **GarageBand**, not our Standalone. So it **never** looks at the Standalone app’s keySamples; it only uses **user folders** (`~/Music/MatildaPiano/Samples`, `~/Documents/MatildaPiano/Samples`).
- **Fix:** Copy your full sample set into a user folder (e.g. `cp keySamples/*.wav ~/Documents/MatildaPiano/Samples/`), then **quit GarageBand completely** and reopen so the plugin reloads samples.

---

## 7. Quick Reference: “What do I do when…?”

- **First time build:** Install CMake (e.g. `brew install cmake`), set `JUCE_DIR` to your JUCE folder, run `./build.sh`. See **BUILD-SIMPLE.md** for step-by-step.
- **Weird build errors:** Run **./clean-and-build.sh** (full clean then build). Ensure Xcode/Command Line Tools and JUCE path are correct.
- **No sound:** Check: (1) Samples folder exists and has WAV/AIFF, (2) Standalone fully restarted after adding samples, (3) output device and Master volume, (4) we don’t clear sounds in releaseResources.
- **Only one octave (v1 samples):** Quit app, rebuild, launch from `build/.../Standalone/Matilda Piano 2.app` or the v1 app if testing legacy. In GarageBand, use a user folder and quit/reopen GarageBand. **v2** is not limited to one octave.
- **Delay/XY not doing much:** Ensure you’re on the latest code (delay popSample -1, reset in prepare, XY reverb wash and stronger tape). Rebuild and test.

---

## 8. Where to Read More

- **Build steps (simple):** **BUILD-SIMPLE.md**
- **Architecture (threading, parameters, sample paths):** **docs/architecture.md**
- **Testing and troubleshooting:** **docs/TESTING-LOG.md**
- **Product scope and features:** **docs/PRD.md**
- **Figma → JUCE layout and assets:** **docs/FIGMA-SPEC.md**
- **Milestones (M1, M2a, M2b, M2c):** **docs/MILESTONES.md**
- **Version 1 snapshot:** The **version-1/** folder is a copy of the project as released in v1.0.0 (GitHub); see **version-1/README-VERSION-1.md**.

---

*This guide was written to be noob-friendly: it explains the “why” and “how” of our tech choices, component layout, CMake, folder structure, and the main bugs we fixed, so you can follow and extend the project with confidence.*
