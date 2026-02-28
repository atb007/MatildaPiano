# Matilda Piano VST Plugin

A beautiful piano VST plugin built with JUCE, featuring a custom UI design and professional DSP effects.

**Version:** **v1.0.0** (2026-02-27). See **`CHANGELOG.md`** and **`docs/MILESTONES.md`**.

**Designers / first-time build:** See **[BUILD-SIMPLE.md](BUILD-SIMPLE.md)** for simple steps and the **full clean + build** script (`./clean-and-build.sh`) when you get errors.

## Features

- **Piano Sampler**: High-quality piano samples with velocity-sensitive playback
- **ADSR Envelope**: Full control over Attack, Decay, Sustain, and Release
- **Effects Chain**:
  - Tape/Flutter effect controlled by XY pad
  - Tempo-synced delay with musical subdivisions
  - Reverb for spatial depth
  - Master volume control
- **Custom UI**: Beautiful interface matching Figma design
- **macOS AU Plugin**: Compatible with GarageBand and other AU hosts

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
│   ├── MatildaSampler*    # Sampler engine
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

`./build.sh` configures (if needed) and builds the plugin and the test target. The AU is copied to `~/Library/Audio/Plug-Ins/Components/Matilda Piano.component`. To build again without reconfiguring: `cmake --build build --config Release`.

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

## Adding Piano Samples

**Preferred: use the project’s `keySamples` folder.**  
Place WAV files named by note + octave 0–7 (e.g. `c0.wav`, `c#5.wav`). Octave 0 = first octave (MIDI 24). The plugin looks for `keySamples` inside the Standalone app bundle (copied at build) or next to the .app. See `docs/architecture.md` and `docs/TESTING-LOG.md` for full rules.

**Alternatively**, place piano samples in one of `~/Music/MatildaPiano/Samples` or `~/Documents/MatildaPiano/Samples` (see naming rules in `docs/TESTING-LOG.md`).

1. **Sample duration:** Use **3–8 seconds per note** for natural decay; the plugin uses up to **30 seconds** per sample. One sample per note; the on-screen keyboard is **7 octaves** (C1–C8) per PRD.
2. **Naming (user folders):** Include a note name (e.g. `C4`, `F#3`) or MIDI number (e.g. `60`) in the filename. See `docs/architecture.md` and `docs/TESTING-LOG.md` (Troubleshooting: Sound).
3. If no samples are found, the plugin shows a status message in the UI (e.g. “No samples found — add keySamples or WAV/AIFF to …”).

## Testing in GarageBand

1. Build the plugin (see Building section above)

2. Open GarageBand

3. Create a new Software Instrument track

4. Click on the instrument slot and look for "Matilda Piano" in the Audio Units list

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
- **Manual testing (Standalone / AU):** Record sessions and use the troubleshooting checklist in **`docs/TESTING-LOG.md`** (sound, GUI keys, samples, output device).

## Development Notes

- The plugin uses JUCE's `AudioProcessorValueTreeState` for parameter management.
- The processor owns a `MidiKeyboardState` shared with the editor; on-screen keyboard input is injected into MIDI in `processBlock()` so the synth plays from the GUI keyboard.
- All parameters are automatable in the host DAW.
- The delay module syncs to host tempo via `AudioPlayHead::getPosition()` / `PositionInfo::getBpm()`.
- Samples are loaded into RAM (no disk streaming in v1).
- Maximum polyphony: 32 voices.

## Future Enhancements

- [ ] Preset system
- [ ] Additional piano variations
- [ ] Sustain pedal support
- [ ] Round-robin sample playback
- [ ] Windows VST3 build

## Pushing to GitHub

The repo is initialized with v1.0.0 tagged. To push:

1. **Create a new repository** on GitHub (e.g. `MatildaPiano`).
2. **Add remote and push:**
   ```bash
   git remote add origin https://github.com/YOUR_USERNAME/MatildaPiano.git
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
