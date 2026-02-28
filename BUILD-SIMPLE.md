# Build Matilda Piano — simple steps (for designers)

This guide uses plain language and copy-paste steps. If you see errors like **"algorithm file not found"** or **"Failed to build juceaide"**, follow **Section B** (full clean, then build).

---

## What you need once

- **Terminal** (search “Terminal” in Spotlight).
- **JUCE folder** on your Mac (e.g. your `JUCE 2` folder). You’ll tell the script where it is.
- **Command Line Tools** (no full Xcode needed). If you’re not sure, run the “One-time setup” below.

---

## One-time setup

Open **Terminal** and run these **one after the other** (you can paste each block and press Enter).

**1. Point your Mac at the right developer tools** (so the compiler can find system files):

- **If you have Xcode installed** (recommended):
  ```bash
  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
  ```
- **If you only have Command Line Tools** (no Xcode):
  ```bash
  sudo xcode-select -s /Library/Developer/CommandLineTools
  ```

If it asks for your Mac password, type it (nothing will show) and press Enter.

**2. Make sure CMake is installed:**

```bash
brew install cmake
```

If you don’t have Homebrew, install it from [brew.sh](https://brew.sh), then run the line above.

---

## A. Normal build (when things already work)

Use this when you’ve already built successfully and only changed design or code.

1. Open Terminal.
2. Go to the project folder (replace with your real path if different):

   ```bash
   cd ~/Desktop/CursorAI/VST/Matilda/WebApp
   ```

3. Tell the build where JUCE is (use **your** JUCE folder path):

   ```bash
   export JUCE_DIR='/Users/udai.deori/Desktop/CursorAI/JUCE 2'
   ```

4. Build:

   ```bash
   ./build.sh
   ```

When it finishes, you can open the app:  
**Matilda Piano.app** → usually in `build/MatildaPiano_artefacts/Release/`.

---

## B. Full clean, then build (when you get errors)

Use this when you see **"algorithm file not found"** or **"Failed to build juceaide"** or when something just “doesn’t build right.”  
A **full clean** means: delete the old build folder, then build again from scratch so the right settings are used.

### Option 1: One script (easiest)

1. **If you have Xcode:** run this once in Terminal (so the compiler uses Xcode’s headers):
   ```bash
   sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
   ```

2. Open Terminal and go to the project folder:
   ```bash
   cd ~/Desktop/CursorAI/VST/Matilda/WebApp
   ```

3. Run the clean-and-build script:
   ```bash
   ./clean-and-build.sh
   ```

The script will: delete the old `build` folder, use the Xcode SDK (if Xcode is installed), skip building juceaide, set the JUCE path, then configure and build the plugin.

### Option 2: Copy-paste each step

Do these in order, one block at a time.

**Step 1 — Go to the project folder**

```bash
cd ~/Desktop/CursorAI/VST/Matilda/WebApp
```

**Step 2 — Delete the old build (full clean)**  
(This removes the `build` folder so the next build starts fresh.)

```bash
rm -rf build
```

**Step 3 — Set your JUCE folder path**  
(Use your actual path; the one below is an example.)

```bash
export JUCE_DIR='/Users/udai.deori/Desktop/CursorAI/JUCE 2'
```

**Step 4 — Build**

```bash
./build.sh
```

---

## After a successful build

- **Standalone app (no DAW):**  
  Open: `build/MatildaPiano_artefacts/Release/Matilda Piano.app`
- **In GarageBand:**  
  The AU plugin is copied to your system; in GarageBand, add a Software Instrument and choose **Matilda Piano**.

---

## If it still fails

1. Make sure you ran **Section B** (full clean) and used **the same Terminal window** for all steps (don’t close it between steps).
2. Check that your JUCE path is correct: in Finder, go to the folder that contains the JUCE project (e.g. `JUCE 2`). Copy that path and use it in `export JUCE_DIR='...'` (with quotes if the path has spaces).
3. If you see a new error, copy the **full error message** from Terminal and share it so we can fix the next step.
