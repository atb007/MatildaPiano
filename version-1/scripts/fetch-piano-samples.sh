#!/usr/bin/env bash
# Create MatildaPiano Samples folder and optionally download one octave of
# free piano samples (University of Iowa MIS — Steinway B, per-note AIFF).
# Usage: ./scripts/fetch-piano-samples.sh [target-dir]
# Default target: ~/Documents/MatildaPiano/Samples
#
# For full 7-octave uploads: use 3–8 s per note (plugin uses up to 30 s). See docs/TESTING-LOG.md.

set -e

TARGET="${1:-$HOME/Documents/MatildaPiano/Samples}"
BASE_URL="https://theremin.music.uiowa.edu/sound%20files/MIS/Piano_Other/piano"

# One octave C4–B4 (12 notes). Use "mf" (mezzo forte) for audible level; "pp" (pianissimo) is very quiet.
DYNAMIC="mf"
NOTES=(C4 Db4 D4 Eb4 E4 F4 Gb4 G4 Ab4 A4 Bb4 B4)

echo "Matilda Piano — sample folder setup"
echo "Target: $TARGET"
mkdir -p "$TARGET"

if ! command -v curl &>/dev/null; then
    echo "curl not found. Folder created; add WAV/AIFF files manually."
    echo "See docs/TESTING-LOG.md (Troubleshooting: Sound) for naming rules."
    exit 0
fi

echo "Downloading one octave (C4–B4) from University of Iowa MIS Piano (${DYNAMIC})..."
for note in "${NOTES[@]}"; do
    # Iowa filenames: Piano.mf.C4.aiff (or Piano.pp.* for very quiet)
    file="Piano.${DYNAMIC}.${note}.aiff"
    url="${BASE_URL}/${file}"
    dest="$TARGET/$file"
    if [[ -f "$dest" ]]; then
        echo "  skip $file (already exists)"
    else
        if curl -sfL -o "$dest" "$url"; then
            echo "  ok $file"
        else
            echo "  fail $file ($url)"
            rm -f "$dest"
        fi
    fi
done

echo "Done. Restart the Matilda Piano Standalone app to load samples."
