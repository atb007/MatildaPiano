#!/usr/bin/env bash
# Copy WebApp/Assets to ~/Documents/MatildaPiano/Assets so the AU can load
# fonts and images when running inside GarageBand (or any host).
#
# Usage: ./scripts/copy-assets-to-documents.sh [target-dir]
# Default target: ~/Documents/MatildaPiano/Assets

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE="$(cd "$SCRIPT_DIR/.." && pwd)/Assets"
TARGET="${1:-$HOME/Documents/MatildaPiano/Assets}"

if [[ ! -d "$SOURCE" ]]; then
  echo "Error: Source folder not found: $SOURCE"
  exit 1
fi

echo "Matilda Piano — copy assets for AU (e.g. GarageBand)"
echo "From: $SOURCE"
echo "To:   $TARGET"
mkdir -p "$TARGET"
rsync -a --delete "$SOURCE/" "$TARGET/" 2>/dev/null || cp -R "$SOURCE/"* "$TARGET/"
echo "Done. Assets are in $TARGET"
