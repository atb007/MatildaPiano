#!/usr/bin/env bash
# Normalize piano sample levels. Iowa "pp" (pianissimo) AIFFs are valid but
# extremely quiet (~1.6% full scale), so they are inaudible in players and in the plugin.
# This script boosts them to a usable level (e.g. peak ~ -3 dB).
# Usage: ./scripts/normalize-piano-samples.sh [samples-dir]
# Requires: sox (brew install sox) or ffmpeg (brew install ffmpeg)

set -e

SAMPLES_DIR="${1:-$HOME/Documents/MatildaPiano/Samples}"
BACKUP_DIR="${SAMPLES_DIR}.backup.$$"
mkdir -p "$SAMPLES_DIR"

if [[ ! -d "$SAMPLES_DIR" ]]; then
    echo "Error: directory does not exist: $SAMPLES_DIR"
    exit 1
fi

shopt -s nullglob
files=("$SAMPLES_DIR"/*.aiff "$SAMPLES_DIR"/*.aif "$SAMPLES_DIR"/*.wav "$SAMPLES_DIR"/*.wave)
if [[ ${#files[@]} -eq 0 ]]; then
    echo "No AIFF/WAV files found in $SAMPLES_DIR"
    exit 0
fi

if command -v sox &>/dev/null; then
    echo "Using sox to normalize (peak -3 dB)..."
    mkdir -p "$BACKUP_DIR"
    for f in "${files[@]}"; do
        bn=$(basename "$f")
        cp "$f" "$BACKUP_DIR/$bn"
        tmp="${f%.*}.tmp.${f##*.}"
        sox "$f" -b 16 "$tmp" norm -3 && mv "$tmp" "$f"
        echo "  $bn"
    done
    echo "Backups in $BACKUP_DIR (remove when satisfied)."
elif command -v ffmpeg &>/dev/null; then
    echo "Using ffmpeg to normalize (loudnorm / peak)..."
    mkdir -p "$BACKUP_DIR"
    for f in "${files[@]}"; do
        bn=$(basename "$f")
        ext="${bn##*.}"
        cp "$f" "$BACKUP_DIR/$bn"
        tmp="${f%.*}.tmp.$ext"
        ffmpeg -y -i "$f" -af "loudnorm=I=-16:TP=-3:LRA=11" -ar 44100 -ac 2 "$tmp" 2>/dev/null
        mv "$tmp" "$f"
        echo "  $bn"
    done
    echo "Backups in $BACKUP_DIR (remove when satisfied)."
else
    echo "Neither sox nor ffmpeg found. Install one:"
    echo "  brew install sox"
    echo "  or: brew install ffmpeg"
    echo ""
    echo "Or re-download louder samples: edit scripts/fetch-piano-samples.sh to use"
    echo "  Piano.mf.*.aiff (mezzo forte) instead of Piano.pp.*.aiff (pianissimo),"
    echo "  then run fetch-piano-samples.sh again in a new folder."
    exit 1
fi

echo "Done. Restart the Matilda Piano Standalone app to use normalized samples."
