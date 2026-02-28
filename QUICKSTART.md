# Quick Start Guide

**If you’re a designer or new to building:** use **[BUILD-SIMPLE.md](BUILD-SIMPLE.md)** for plain-language steps and a **full clean + build** script (`./clean-and-build.sh`) when you get errors.

**Project status:** M1 complete; M2a + M2b + M2c complete (GUI, fonts, effect module + delay Off + XY). See `docs/MILESTONES.md`.

---

## Prerequisites Check

1. **JUCE Installation**
   ```bash
   # Download JUCE from https://juce.com
   # Extract and note the path, e.g., ~/JUCE
   ```

2. **CMake** (if not installed)
   ```bash
   brew install cmake
   ```

3. **Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

## First Build

1. **Set JUCE path** (install prefix with `JUCEConfig.cmake`, or JUCE **source** folder)
   ```bash
   export JUCE_DIR='/path/to/JUCE'   # or e.g. '/path/to/JUCE 2' (source directory)
   ```

2. **Build using the script**
   ```bash
   ./build.sh
   ```

   Or manually:
   ```bash
   cmake -B build -DJUCE_DIR="$JUCE_DIR" -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

3. **Verify installation**
   - AU: `ls ~/Library/Audio/Plug-Ins/Components/Matilda\ Piano.component`
   - Standalone: `build/MatildaPiano_artefacts/Release/Matilda Piano.app`
   - Unit tests: `./build/MatildaPianoTests_artefacts/Release/MatildaPianoTests` (see `docs/testing.md`).

4. **Run Standalone (fast UI testing without a DAW)**
   ```bash
   open build/MatildaPiano_artefacts/Release/Matilda\ Piano.app
   ```
   Or from Xcode/VS Code: run the "Matilda Piano - Standalone" target.

### Build errors: `'algorithm' file not found` or `Failed to build juceaide`

The C++ compiler can’t find system headers. The project is set up to use the **Command Line Tools SDK** so you don’t need full Xcode.

1. **Use the Command Line Tools path** (no Xcode required)
   ```bash
   sudo xcode-select -s /Library/Developer/CommandLineTools
   ```
   CMake will then use the SDK at `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk` so headers like `<algorithm>` are found.

2. **Reinstall Command Line Tools** only if the path above doesn’t exist or the build still fails: `xcode-select --install`

3. **Full clean, then build** (required so the compiler and JUCE settings are correct). Easiest: run from the project folder:
   ```bash
   ./clean-and-build.sh
   ```
   Or do it step by step: delete the old build (`rm -rf build`), set `export JUCE_DIR='/path/to/JUCE 2'`, then run `./build.sh`. See [BUILD-SIMPLE.md](BUILD-SIMPLE.md) for copy-paste steps.

The project turns off JUCE extras (so juceaide isn’t built) when using the JUCE source folder. You can build with **Command Line Tools only**; full Xcode is not required.

## Adding Your Figma Assets

The plugin can load `background.png` either:
- **Embedded** via `Assets/` (rebuild required), or
- **From disk** at `~/Documents/MatildaPiano/Assets/background.png` (no rebuild needed).

1. Export from Figma as PNG (recommended):
   - `background.png` (800x600px recommended)
   - `matilda-portrait.png` (with transparency)
   - `knob-white.png` (optional)
   - `knob-green.png` (optional)
   - `logo.png` (optional)

2. Choose one:
   - **Embed into the plugin**: copy files to `Assets/`, then rebuild (CMake will auto-generate `BinaryData.h`).
   - **Fast iteration**: copy just `background.png` to `~/Documents/MatildaPiano/Assets/background.png`.

If you’re using the Figma MCP inside Cursor, export or screenshot your UI frame and save it as `background.png` using one of the two paths above.

## Adding Piano Samples

Sample loading looks in:
- `~/Music/MatildaPiano/Samples`
- `~/Documents/MatildaPiano/Samples`

**Duration per note:** **3–8 seconds** recommended; plugin uses up to **30 seconds** per sample. The on-screen keyboard is **7 octaves** (C1–C8) per PRD.

Place WAV/AIFF files there. Supported naming:
- Note names: `Piano_C4.wav`, `Piano_F#3_take1.aif`, `Piano_Bb2.wav`
- MIDI note numbers: `Piano_60.wav`

## Testing in GarageBand

1. Open GarageBand
2. Create Software Instrument track
3. Click instrument slot → Audio Units → Matilda Piano
4. Play MIDI notes to test

## Troubleshooting

**Plugin doesn't appear in GarageBand:**
- Check: `auval -a | grep Matilda`
- Verify: `~/Library/Audio/Plug-Ins/Components/MatildaPiano.component` exists
- Restart GarageBand

**Build errors:**
- Verify JUCE_DIR points to correct JUCE installation
- Check CMake version: `cmake --version` (need 3.15+)
- Check Xcode tools: `xcode-select -p`

**Missing assets:**
- Assets are optional - plugin will build without them
- UI will use gradient background if assets missing
- Add assets to `Assets/` and rebuild

## Next Steps

- [ ] Add your Figma assets to `Assets/` or `~/Documents/MatildaPiano/Assets/`
- [ ] Add piano samples to `~/Music/MatildaPiano/Samples` or `~/Documents/MatildaPiano/Samples`
- [ ] Test in Standalone app or GarageBand; use **`docs/TESTING-LOG.md`** for the checklist and to log observations
- [ ] Add presets system (optional)
