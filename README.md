# Matilda Piano VST Plugin

<img width="1074" height="483" alt="Piano Frame" src="https://github.com/user-attachments/assets/f1f91ab5-2b36-41af-a9da-d5a9d52389c8" />

A beautiful piano AU plugin built with JUCE, featuring neural network synthesis and professional DSP effects.

**Version:** **v3.0.0** — Neural network synthesis (ONNX Runtime). See **`CHANGELOG.md`** and **`docs/MILESTONES.md`**.

**Next:** **v4.0.0** — Schuck–Young hybrid engine. PRD: **`docs/version4-prd.md`**. Context: **`docs/V3-RETROSPECTIVE.md`**.

**Previous versions:**
- v2.0.0 (frozen): Karplus-Strong physical modeling - see branch `v2-physical-model`
- v1.0.0 (archived): Sample-based - see git tag `v1.0.0`

**Side-by-side installs:** This build installs as **Matilda Piano 3** (new `PLUGIN_CODE` **MtP3**, bundle ID `com.matildaaudio.matildapiano3`) so it does not replace previous versions in your DAW.

**Designers / first-time build:** See **[BUILD-SIMPLE.md](BUILD-SIMPLE.md)** for simple steps and the **full clean + build** script (`./clean-and-build.sh`) when you get errors.

**New to the project?** See **[docs/PROJECT-GUIDE-NOOB-FRIENDLY.md](docs/PROJECT-GUIDE-NOOB-FRIENDLY.md)** for a noob-friendly guide: why we use this tech stack, how components work together, why CMake and the folder layout matter, and critical errors we fixed.

## Features

- **Physical piano engine (v2):** Karplus–Strong–style string synthesis per voice — no WAV sample packs required; velocity-sensitive excitation
- **ADSR Envelope**: Full control over Attack, Decay, Sustain, and Release
- **Effects Chain**:
  - Tape/Flutter effect controlled by XY pad
  - Tempo-synced delay with musical subdivisions
  - Reverb for spatial depth
  - Master volume control
- **Custom UI**: Beautiful interface matching Figma design
- **macOS AU Plugin**: **Matilda Piano 2** — compatible with GarageBand and other AU hosts alongside v1

## Project Structure

```
MatildaPiano/
├── CMakeLists.txt          # CMake build configuration
├── build.sh                # Configure + build (requires JUCE_DIR)
├── clean-and-build.sh      # Full clean + build (designer-friendly; set JUCE path in script)
├── BUILD-SIMPLE.md         # Plain-language build steps for designers
├── Source/                 # Source code
│   ├── PluginProcessor.*  # Main audio processor (APVTS, synth, keyboard state, DSP chain)
│   ├── PluginEditor.*     # UI editor
│   ├── Parameters.*       # Parameter definitions
│   ├── MatildaPhysical*   # Physical string engine (voices + sound)
│   ├── TapeModule.*       # Tape/flutter DSP
│   ├── DelayModule.*      # Delay effect
│   ├── ReverbModule.*     # Reverb effect
│   ├── XYPadComponent.*   # XY pad UI component
│   └── ChickenHeadKnob.*  # Custom knob styling
├── Assets/                 # Figma exported assets (PNG/SVG)
├── Tests/                  # Unit tests (MatildaPianoTests.cpp)
├── docs/                   # Architecture, PRD, testing, Figma
└── .vscode/               # VS Code configuration
```

## Prerequisites

- **macOS** (12.0 or later)
- **CMake** 3.22 or later (`brew install cmake`)
- **JUCE** 7.0+ — either an **install prefix** (with `JUCEConfig.cmake`) or the **JUCE source folder** (project uses `add_subdirectory` when no config found).
- **Xcode** or **Xcode Command Line Tools** (for compiler and SDK). With Xcode installed, use `sudo xcode-select -s /Applications/Xcode.app/Contents/Developer` if you see header/SDK errors.
- **VS Code** (optional, for editing)

## Building

### 1. Install JUCE

Download JUCE from [juce.com](https://juce.com) and extract it. Note the path to the JUCE folder.

### 2. Configure and build

Set `JUCE_DIR` to your JUCE **install path** or **source folder** (e.g. your `JUCE 2` folder), then build:

```bash
export JUCE_DIR='/path/to/JUCE'   # or '/path/to/JUCE 2' for source
./build.sh
```

For a **full clean** (e.g. after errors), use **`./clean-and-build.sh`** (edit the `JUCE_PATH` inside the script if needed). See **[BUILD-SIMPLE.md](BUILD-SIMPLE.md)** for step-by-step and troubleshooting.

`./build.sh` configures (if needed) and builds the plugin and the test target. The AU is copied to `~/Library/Audio/Plug-Ins/Components/Matilda Piano 2.component`. To build again without reconfiguring: `cmake --build build --config Release`.

### 3. Using VS Code

1. Open the project in VS Code
2. Press `Cmd+Shift+B` to build (or use the CMake: Build task)
3. The plugin will be automatically installed after a successful build

## Adding Figma Assets

1. Export your Figma design assets as PNG files:
   - `background.png` - Full UI background (800x600px recommended)
   - `matilda-portrait.png` - Left side portrait artwork
   - `knob-white.png` - White knob sprite (optional, for custom knob graphics)
   - `knob-green.png` - Green knob sprite (optional)
   - `logo.png` - Matilda logo (optional)

2. Place all exported files in the `Assets/` directory

3. Rebuild the project - CMake will automatically generate `BinaryData.h` with asset accessors

4. The assets will be embedded in the plugin binary and accessible via `BinaryData::background_png`, etc.

## Samples (v1 only)

**v2 does not use sample files.** The instrument is generated in real time. For the historical sample-based workflow (v1 / `version-1/`), see **`version-1/README.md`**, `docs/TESTING-LOG.md`, and `docs/architecture.md` (historical section).

## Smooth play and low latency

For the most responsive, glitch-free feel (e.g. dragging the mouse across the keys):

- **Use a small buffer size** in your host or DAW. Set the audio device buffer to **128 samples** (or 64 if your system handles it without dropouts). In GarageBand: choose **GarageBand → Settings → Audio/MIDI** and pick a smaller buffer (e.g. 128). Lower buffer = less input-to-sound delay and smoother continuous note triggering.
- The plugin reports **0 samples latency**; the main variable is the host buffer. Keeping the buffer at 128 (or 64) gives a smooth, lag-free experience similar to built-in instruments.

## Testing in GarageBand

1. Build the plugin (see Building section above)

2. Open GarageBand

3. Create a new Software Instrument track

4. Click on the instrument slot and look for **"Matilda Piano 2"** in the Audio Units list (v1 appears as **"Matilda Piano"**)

5. If the plugin doesn't appear:
   - Check that it's in `~/Library/Audio/Plug-Ins/Components/`
   - Run `auval -a | grep Matilda` to verify AU registration
   - Check Console.app for any error messages

## Parameter Reference

### ADSR Envelope
- **Attack**: 0.0 - 5.0 seconds (default: 0.1s)
- **Decay**: 0.0 - 5.0 seconds (default: 0.3s)
- **Sustain**: 0.0 - 1.0 (default: 0.7)
- **Release**: 0.0 - 5.0 seconds (default: 0.5s)

### Effects
- **Reverb**: 0.0 - 1.0 mix (default: 0.3)
- **Delay Mix**: 0.0 - 1.0 (default: 0.0)
- **Delay Time**: Lowest position = **Off**; then musical subdivisions (1/64 to 1 bar, default: 1/4)
- **Master Volume**: 0.0 - 1.0 (default: 0.8)

### XY Pad (Tape/Flutter)
- **X-axis**: Wow/flutter rate (0.0 = none, 1.0 = max)
- **Y-axis**: Saturation + tone (0.0 = clean, 1.0 = saturated + darker)

## Testing

- **Unit tests:** `Tests/MatildaPianoTests.cpp` (target `MatildaPianoTests`). They cover parameter layout (IDs, count, default ranges). See **`docs/testing.md`** for build and run.
  ```bash
  cmake --build build --target MatildaPianoTests --config Release
  ./build/MatildaPianoTests_artefacts/Release/MatildaPianoTests
  ```
- **Manual testing (Standalone / AU):** Record sessions and use the troubleshooting checklist in **`docs/TESTING-LOG.md`** (sound, GUI keys, output device).

## Development Notes

- The plugin uses JUCE's `AudioProcessorValueTreeState` for parameter management.
- The processor owns a `MidiKeyboardState` shared with the editor; on-screen keyboard input is injected into MIDI in `processBlock()` so the synth plays from the GUI keyboard.
- All parameters are automatable in the host DAW.
- The delay module syncs to host tempo via `AudioPlayHead::getPosition()` / `PositionInfo::getBpm()`.
- **v2:** No sample loading; the physical engine allocates delay lines in `prepareToPlay()`.
- Maximum polyphony: 32 voices.

## Future Enhancements

- [ ] **v4 hybrid engine** — Schuck–Young partials, ONNX amplitudes, Inharmonicity knob (`docs/version4-prd.md`)
- [ ] Preset system
- [ ] Sustain pedal support
- [ ] Windows VST3 build

**Retired from v3 roadmap (superseded by v4):** richer physical model via Karplus-Strong inharmonicity — see `docs/V3-RETROSPECTIVE.md` for why Schuck–Young hybrid replaces both pure neural and v2 physical paths.

## Pushing to GitHub

The repo is initialized with v1.0.0 tagged. To push:

1. **Create a new repository** on GitHub (e.g. `MatildaPiano`).
2. **Add remote and push:**
   ```bash
   git remote add origin https://github.com/atb007/MatildaPiano.git
   git branch -M main
   git push -u origin main
   git push origin v1.0.0
   ```
3. Optionally create a **Release** on GitHub from the v1.0.0 tag.

## License

[Add your license here]

## Credits

- Built with [JUCE](https://juce.com)
- UI design from Figma
