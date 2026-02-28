# Matilda Piano — Testing log

Record observations from testing the **Standalone** app (and optionally the AU in GarageBand). Use one section per test session; keep the most critical issue (sound) at the top until resolved.

**Related:** Unit tests (build/run) → **`docs/testing.md`**.

**Milestone:** **M1** complete (frozen). **M2a** + **M2b** + **M2c** complete (GUI, fonts, effect module + delay Off + XY enabled). See `docs/MILESTONES.md`.

---

## Quick checklist: get audio when clicking GUI keys

Use this when testing with the **on-screen keyboard (mouse clicks)** and you hear no sound:

1. **Samples present**  
   - **keySamples (preferred):** If the project has a `keySamples/` folder (e.g. `c0.wav`, `c#5.wav` — see [Where the plugin looks](#where-the-plugin-looks-for-samples)), the build copies it into the Standalone app bundle. All **7 octaves** (C1–C8, MIDI 24–108) per PRD should then play.  
   - **User folders:** Otherwise the plugin uses `~/Documents/MatildaPiano/Samples` or `~/Music/MatildaPiano/Samples` (e.g. run `./scripts/fetch-piano-samples.sh` for one octave only).

2. **Rebuild and full restart**  
   - Run `./build.sh`, then **quit the Standalone app completely** and launch it again. Samples load **only at startup**; the app does not reload samples until the next launch. After changing `keySamples/`, you must rebuild so the new files are copied into the app bundle, then **fully quit and relaunch** the Standalone (preferably from the build output: `build/MatildaPiano_artefacts/Release/Standalone/Matilda Piano.app`).

3. **Click keys that have samples**  
   - If using **keySamples** (full set): keys **C1–C7** have samples and play; **C0** is shown but has no samples by default. Keyboard ends at C7.  
   - If using **user folder** (fetch script): only **C4–B4** (middle octave) has files — click keys in that octave.

4. **Output device**  
   - In the Standalone app: **Audio** or **Options → Audio** → set **output** to the device you’re listening on (e.g. built-in output or your interface).

5. **Master volume**  
   - Ensure the **Master vol.** knob is not at zero.

**Code fix (GUI key injection):** The plugin now uses JUCE’s standard pattern `keyboardState.processNextMidiBuffer(midiMessages, ...)` so that note-on/note-off from the on-screen keyboard are injected into the MIDI buffer. Previously it only polled key state and injected manually; using the state’s queue is more reliable. Rebuild and test again after pulling this change. See **1d** for the current strip-down (DSP bypass + manual MIDI injection).

---

### 1d. Strip-down: no XY / no Tape–Delay–Reverb (historical — full chain re-enabled 2026-02-27)

- **Observation:** Previously used to isolate “no sound” by bypassing Tape/Delay/Reverb.
- **Current state (2026-02-27):** Full effect chain is **enabled** (`MATILDA_BYPASS_DSP_DEBUG 0`). Tape, delay, reverb, and XY pad all affect sound. Delay knob lowest position = Off (mix 0, label "Off").
- **To bypass again for debug:** In `PluginProcessor.cpp`, set `#define MATILDA_BYPASS_DSP_DEBUG 1` and rebuild.

**Status:** Full chain enabled; delay Off at minimum; XY pad active.

---

## Test 1 — 2025-02-24 (Standalone)

**Build:** Release standalone from `./build.sh` (JUCE from `~/Downloads/JUCE`).

### 1. Sound — CRITICAL (no sound)

- **Observation:** No sound when playing the on-screen keyboard (or external MIDI).
- **Cause:** The plugin only loads samples from **existing** folders. It does **not** create them. If `~/Music/MatildaPiano/Samples` or `~/Documents/MatildaPiano/Samples` don’t exist, or contain no WAV/AIFF files, the synth has no sounds → silent output.
- **What to do:**
  1. Create the Samples folder (see [Troubleshooting: Sound](#troubleshooting-sound) below).
  2. Add piano samples (one file per note, or a subset). Use the **sample naming rules** so the plugin can map files to MIDI notes.
  3. Restart the Standalone app (samples are loaded only at startup).

**Fix applied:** The Samples folder was created and one octave (C4–B4) of free University of Iowa piano samples was downloaded to `~/Documents/MatildaPiano/Samples` via `./scripts/fetch-piano-samples.sh`. **Restart the Standalone app** to load them and get sound. For more keys, download more files from [Iowa MIS Piano](https://theremin.music.uiowa.edu/MISpiano.html) into the same folder (use the same naming pattern, e.g. `Piano.pp.C3.aiff`).

**Status:** [ ] Resolved (restart Standalone and test).

---

### 1b. Meter moves but no sound (MIDI present, level visible)

- **Observation:** Random key presses show activity on the decibel/level meter, but no sound is heard.
- **Likely causes and fixes:**
  1. **Samples cleared by audio reconfig:** The plugin used to call `synth.clearSounds()` in `releaseResources()`. The host/standalone can call that when the audio device is reconfigured (e.g. opening audio settings, or on first device init). That removed all loaded samples so the synth had nothing to play. **Fix applied:** `releaseResources()` no longer clears sounds. Rebuild, then restart the Standalone app so samples load again and are not cleared.
  2. **Wrong output device:** In the Standalone app, open the audio menu (e.g. **Audio** or **Options → Audio**) and ensure the **output device** is the one you’re actually listening on (e.g. built-in output or your speakers/interface). If the app is routing to a different device, you’ll see level but hear nothing.
  3. **Master volume:** Ensure the “Master vol.” knob is up (default 0.8). If it was saved at 0, raise it.
- **Check:** After rebuilding, do a **full quit** of the Standalone app and open it again so `loadSamples()` runs with the fixed behaviour.

**Status:** [ ] Resolved.

---

### 1c. Sharp ear-splitting tone on key press, then nothing (Test 2)

- **Observation:** One key press produces a sharp, ear-splitting tone, then silence; no sustained note. Other VSTs work fine.
- **Root cause:** The ADSR envelope in `MatildaSamplerVoice` was **never given a sample rate**. JUCE’s ADSR requires `setSampleRate()` before use; without it, envelope timing is wrong and can produce an instant full-level spike (sharp tone) and then incorrect decay/sustain (silence).
- **Fix applied:**  
  - `MatildaSamplerVoice::setSampleRate(double)` was added and calls `adsr.setSampleRate(sampleRate)`.  
  - In `prepareToPlay()`, the processor now calls `setSampleRate(sampleRate)` on each `MatildaSamplerVoice`.  
  Rebuild and run the Standalone again; notes should have a proper attack/sustain/release.
- **Status:** [ ] Resolved (verify after rebuild).

---

### 2. GUI / visuals

- **Observation:** GUI not fully correct; font not matching design; Chickenhead knobs not matching Figma.
- **Figma reference:** [AdMaker-CMS — node 4203-94317](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317&t=0agk9terHPGYntjV-4).
- **What helps implementation:**
  - **Fonts:** If the design uses a specific font (name + weight), install it on the machine and share the **exact font name** (and file path or foundry) so the editor can use it via `juce::Font` / typeface.
  - **Chickenhead knobs:** Export from Figma as assets (e.g. PNG strips or frames for different angles). Share the asset files and the **intended mapping** (e.g. “0° = 0%, 270° = 100%”) so `ChickenHeadKnob` can be updated to match.
  - **Breaking down components:** You don’t have to break the whole frame into components upfront. Providing the **Figma node ID** for each element (e.g. keyboard area, XY pad, each knob group) is enough to align layout and styling; we can refine component-by-component as needed.

**Status:** [ ] To be addressed after sound works.

---

### 2.1 Keyboard — C0–C7 (PRD §2.4)

- **Requirement (PRD §2.4):** On-screen keyboard range **C0–C7** (MIDI 12–96). Labels C0, C1, … C7. Sample mapping unchanged (keySamples c0→C1 … c7→C8); C0 keys have no samples by default.
- **Implementation:** `PluginEditor` calls `keyboardComponent.setAvailableRange(12, 96)`, `setLowestVisibleKey(12)`, and **`setOctaveForMiddleC(4)`**.

**Status:** [x] Implemented.

---

### 2.2 XY controller

- **Observation:** XY controller behaviour feels “wonky”.
- **Implementation note:** `XYPadComponent` drives the tape effect (X = wow/flutter, Y = saturation/tone). Need to clarify: is the issue **visual** (position vs. value), **response curve**, or **parameter mapping**? Once sound works, we can tune sensitivity and mapping.

**Status:** [ ] To be refined after sound works.

---

## Troubleshooting: Sound

### Only one octave plays after adding keySamples (full set)

If you added or updated `keySamples/` with files for all keys but still hear only one octave (e.g. middle C only), the app is almost certainly still using an old sample set in memory or from a different path. **Samples load only at startup.** Do this: (1) Fully quit the Standalone app. (2) Run `./build.sh` so the latest `keySamples/` is copied into the app bundle. (3) Launch **from the build folder**: `build/MatildaPiano_artefacts/Release/Standalone/Matilda Piano.app`. See **Test 2 — 2a** above for full details.

### Where the plugin looks for samples

It checks (in order):

1. **keySamples** — inside the Standalone app bundle (`Contents/Resources/keySamples`) or in the **same folder as the .app** (e.g. `build/.../Standalone/keySamples`). Naming: **note + octave 0–7**, e.g. `c0.wav`, `c#5.wav` (c0 = C1 = MIDI 24; c7 = C8 = MIDI 108). PRD: 7 octaves. The project’s `keySamples/` is copied into the app bundle at build time when present.
2. **`~/Music/MatildaPiano/Samples`**
3. **`~/Documents/MatildaPiano/Samples`**

It only uses a path if that **folder already exists** and contains at least one supported file. It does **not** create the folder.

### Supported formats

- **WAV** (`.wav`, `.wave`)
- **AIFF** (`.aif`, `.aiff`)

### Filename → MIDI note mapping

- **Note name in filename:** e.g. `Piano_C4.wav`, `Piano_F#3.aif`, `Piano_Bb2.wav` → mapped to that MIDI note.
- **MIDI number in filename:** e.g. `Piano_60.wav` → mapped to MIDI note 60 (C4).
- **No parseable note:** file is mapped across all keys (useful only for testing).

So you need **one sample per key** (or per key you want to hear), with the note name or MIDI number in the filename.

### Samples valid but inaudible (26 sec, no sound in media player)

- **Cause:** The University of Iowa **"pp" (pianissimo)** piano files are **real AIFF** with correct duration (~26–30 s), but the **recording level is extremely low** (~1.6% of full scale). So they play as near-silence in any media player and in the plugin.
- **Fix (choose one):**
  1. **Normalize existing files** (sox or ffmpeg):
     ```bash
     ./scripts/normalize-piano-samples.sh
     ```
     Requires `sox` (`brew install sox`) or `ffmpeg` (`brew install ffmpeg`). Backups are created; restart Standalone after.
  2. **Re-download louder set:** The fetch script now uses **"mf" (mezzo forte)** by default. Remove or rename the existing `Samples` folder, then run:
     ```bash
     ./scripts/fetch-piano-samples.sh
     ```
     New files will be `Piano.mf.C4.aiff` etc. and should be audible.

### Free piano sample set (one per note)

- **University of Iowa MIS Piano** (Steinway B, free to use):  
  https://theremin.music.uiowa.edu/MISpiano.html  
  Use **`Piano.mf.*.aiff`** (mezzo forte) for audible level. **`Piano.pp.*.aiff`** (pianissimo) is very quiet; normalize with `./scripts/normalize-piano-samples.sh` if you already have pp files.

### Sample duration for upload

When preparing or uploading samples for each note:

- **Recommended duration per note: 3–8 seconds.**  
  Long enough for natural decay and a short tail; keeps load time and memory reasonable.
- **Maximum duration used by the plugin: 30 seconds.**  
  Any extra length beyond 30 s is not played (plugin trims internally).
- **Format:** WAV or AIFF; include note name (e.g. `C4`, `F#3`) or MIDI number in the filename. One sample per note for the **7-octave** range (C1–C8 = MIDI 24–108, or fewer if you only need a subset). See PRD §2.4.

### Quick setup (create folder + optional script)

1. **Create the folder (required):**
   ```bash
   mkdir -p ~/Documents/MatildaPiano/Samples
   ```
   or
   ```bash
   mkdir -p ~/Music/MatildaPiano/Samples
   ```

2. **Add samples:** Copy WAV/AIFF files into that folder, or run the project script to download **one octave** (C4–B4) of Iowa piano samples (**mf** = audible; avoid **pp** unless you normalize):
   ```bash
   ./scripts/fetch-piano-samples.sh
   ```
   If you already have **pp** files and hear nothing, run **`./scripts/normalize-piano-samples.sh`** (needs sox or ffmpeg) or re-download with the script (it now fetches **mf**).

3. **Restart** the Standalone app.

---

## Test 2 — 2026-02-26 (Standalone, after keySamples update for all keys)

**Build:** Release standalone from `./build.sh`; project `keySamples/` contains 85 WAV files (octaves 0–7 in keySamples naming; maps to C1–C8, 7 octaves per PRD; c0 = MIDI 24).

### 2a. Only one octave plays; latest (full-range) samples not playing

- **Observation:** After updating samples for all keys (keySamples has 85 files covering 7 octaves), only one octave is still heard; the latest samples are not playing.
- **Cause (verified):** Samples are loaded **only at startup** in `loadSamples()`. The plugin does **not** reload from disk until the next app launch. In addition, sample source priority is:
  1. **keySamples** inside the Standalone app bundle (`Contents/Resources/keySamples`) or next to the .app
  2. `~/Music/MatildaPiano/Samples`
  3. `~/Documents/MatildaPiano/Samples`
  If you had been using the user folder (e.g. one octave from the fetch script) and then added a full `keySamples/` and rebuilt, the **running** Standalone still has the old set in memory until you quit and relaunch. When you relaunch, the app must be the one from the **latest build** so it uses the bundle’s `keySamples` (which CMake copies from the project at build time).
- **What to do:**
  1. **Fully quit** the Matilda Piano Standalone app (no instance running).
  2. Rebuild so the latest `keySamples/` is copied into the bundle: `./build.sh`.
  3. Launch the Standalone from the **build output**:  
     `build/MatildaPiano_artefacts/Release/Standalone/Matilda Piano.app`  
     (Do not use an old .app from another location or a previously copied alias.)
  4. Test keys across the keyboard (C0–C7); C1–C7 should play if keySamples are present.

**Status:** [ ] Resolved (verify with full quit + relaunch from build folder). After updating samples for all keys, use the same flow: full quit, rebuild, launch from `build/MatildaPiano_artefacts/Release/Standalone/Matilda Piano.app` so the latest keySamples are in the bundle.

### Testing in GarageBand (AU) — keySamples are not used

When you load Matilda Piano as an **AU inside GarageBand**, the plugin’s “current application” is **GarageBand**, not the Standalone app. So it **never** looks at the Standalone app bundle’s `keySamples`; it only uses the **user folders**:

- `~/Music/MatildaPiano/Samples`
- `~/Documents/MatildaPiano/Samples`

**To get all octaves in GarageBand:**

1. **Copy your full sample set into a user folder** (e.g. Documents). From the project root (where `keySamples/` lives):
   ```bash
   mkdir -p ~/Documents/MatildaPiano/Samples
   cp -n keySamples/*.wav ~/Documents/MatildaPiano/Samples/
   ```
   Or use `~/Music/MatildaPiano/Samples` instead of `~/Documents/...` if you prefer. (`cp -n` skips overwriting existing files.)

2. **Quit GarageBand completely** (so the plugin is unloaded).

3. **Reopen GarageBand** and open your project (or add Matilda Piano again). Samples load only when the plugin starts, so a full quit + reopen is required.

After that, the plugin will load the 85 WAVs from `~/Documents/MatildaPiano/Samples`. Filenames like `c0.wav`, `c#5.wav` are parsed as note names; the keyboard (C0–C7; C1–C7 have samples) should play in GarageBand.

**Summary:** Step 3 (launch Standalone from build folder) only applies to the **Standalone** app. In **GarageBand** you must use the user folder and quit/reopen GarageBand after copying samples.

### F3 / G3 (GUI) glitch or click

- **Observation:** F3 and G3 (from the on-screen keyboard) can play with a distracting glitch or click.
- **Mitigation applied:** The plugin now applies a **3 ms sample attack** when loading each sound (in `loadSamples()`), so the sample does not start at full level instantly. This reduces clicks from hard transients. Rebuild and test; if the glitch remains, the cause may be in the **sample files** for those notes (e.g. `f2.wav`, `g2.wav` in keySamples naming). Try replacing those two files with versions that have a slightly softened attack or no DC offset.

### Other weird-sounding keys (to be reported)

- **Observation (pending):** User has observed weird sound on **three particular keys** only; will report which keys after confirming GUI labelling (C1–C8) for easier troubleshooting.
- **Action:** Identify and log the three keys (by GUI label, e.g. “C2”, “F#3”) and any description of the artefact (click, buzz, wrong pitch, etc.) here or in a follow-up test session.

---

## Test 3 — 2026-02-27 (Standalone, after full keySamples + effect module enabled)

**Build:** Release standalone; keySamples with samples for all keys (full range). Effect chain (tape, delay, reverb) and XY pad enabled (M2c).

### 3a. Delay wonky / “whole sequence” instead of single-note echo

- **Observation:** Delay module doesn't work. Root cause: `popSample(channel, 0.0f)` = 0 samples delay (instant). JUCE expects **-1** to use `setDelay()`.
- **Fixes applied:**
  1. **Max delay cap:** Delay time capped at **1.0 s** so 1/2 bar and 1 bar don’t repeat a full phrase.
  2. **Correct delay line order:** Per JUCE, use **push then pop** (push current input, then pop delayed sample). Order was corrected in `DelayModule::process()`.
  3. **Stable delay time:** `updateDelayTime()` only calls `setDelay()` when the computed delay in samples changes (threshold 0.5 samples) to avoid zipper/glitches from recalc every block. `lastDelaySamples` is reset in `prepare()`.
  4. **popSample(channel, -1.0f)** so delay uses configured time; **delayModule.reset()** in prepare; **higher mix** (0.4–0.8 when on).
- **Status:** [ ] Verify after rebuild (play note, move delay knob from Off to 1/8; hear clear repeat).

### 3b. Ear-splitting burst on polyphony — sudden rise then “goes blank”

- **Observation:** With polyphony, burst still happens: sudden rise in volume then goes blank as more keys are pressed.
- **Root cause:** Synth **sums** all voices; with 1/8 gain we had 32 notes → 4.0 → **hard clamp to ±1** → square-wave distortion (burst then flat “blank” sound). ADSR is fine; the issue is **clipping** from gain stacking.
- **Fix applied (2026-02-27):**  
  - **Polyphony gain = 1/numVoices (1/32)** so 32 voices peak at 1.0 and we **never** hit the first clamp (no burst, no square wave).  
  - **Master make-up:** knob 0–1 is applied as linear gain × 16 so a single note (1/32) is still audible; final **safety clamp** after the full DSP chain so we never send > 1.0 to the host.  
  So: no sudden jump into clipping; level rises smoothly to 1.0 as more keys are held.
- **Status:** [ ] Verify after rebuild (polyphony: no burst, no “goes blank”).

### 3c. XY pad wonky / not audible

- **Observation:** XY pad not working; user wants "watery washed-out vibes", "black box where cursor makes sound evolve — washed out, fluttery, pitch in and out".
- **Fixes applied:**
  1. **Dot sync:** Dot now syncs from params in `paint()` and `resized()` so it reflects saved/automated state.
  2. **Stronger tape effect (TapeModule):** Wow/flutter modulation increased (wow ×0.06, flutter ×0.04) so X axis is clearly audible; saturation drive and wet mix increased so Y axis (saturation + tone) is obvious; tone filter range widened (18 kHz → 2 kHz) so “darker” is audible.
- **Status:** [ ] Verify after rebuild (move XY to top-right while playing; expect clear wobble and saturation).

---

## Test 4 — (date)

*(Template for next session.)*

- **Build:**
- **Sound:** [ ] OK / [ ] No sound / [ ] Other: ___
- **GUI:** ___
- **Other:** ___

---

*Last updated: 2026-02-27 (Delay: popSample -1 fix, reset, higher mix; XY: reverb wash for watery vibe).*
