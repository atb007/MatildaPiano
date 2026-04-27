# Matilda Piano — Version 1 Snapshot

This folder is a **snapshot of the project as released in v1.0.0** (what was committed to GitHub as the first version).

## What’s inside

- **Root:** `CMakeLists.txt`, `build.sh`, `clean-and-build.sh`, `README.md`, `CHANGELOG.md`, `BUILD-SIMPLE.md`, `QUICKSTART.md`
- **Source/** — All C++ source and headers for the plugin
- **Tests/** — Unit tests
- **docs/** — Architecture, PRD, Figma spec, testing log, milestones
- **scripts/** — Sample fetch, normalize, copy-assets
- **Assets/** — Figma-exported images/fonts (if present)
- **keySamples/** — Piano WAV samples (if present)

## How to use this snapshot

- **Reference:** Use this folder to see exactly what “version 1” looked like.
- **Build from here:** From this `version-1` folder you can run `./build.sh` (after setting `JUCE_DIR`) the same way as from the main project root. You may want to copy `.vscode` from the main project if you use VS Code.

## Main project vs version-1

The **main project root** may have newer changes (e.g. BMad workflow files, extra docs). This `version-1` folder is a frozen copy of the v1.0.0 release for comparison and safe reference.
