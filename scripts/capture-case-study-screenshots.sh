#!/bin/bash
# Capture built Matilda Piano standalone app windows for the case study.
# Requires macOS Screen Recording permission for Terminal (or Cursor).
#
# Usage: ./scripts/capture-case-study-screenshots.sh

set -e

OUT="$(cd "$(dirname "$0")/.." && pwd)/docs/case-study-images"
BASE="$(cd "$(dirname "$0")/.." && pwd)/build/MatildaPiano_artefacts/Release/Standalone"

mkdir -p "$OUT"

capture_app() {
  local app_path="$1"
  local proc_name="$2"
  local out_file="$3"

  if [ ! -d "$app_path" ]; then
    echo "Skip: $app_path not found (build the project first)"
    return 1
  fi

  open "$app_path"
  sleep 5
  osascript -e "tell application \"System Events\" to set frontmost of process \"$proc_name\" to true" 2>/dev/null || true
  sleep 1

  local info
  info=$(swift - <<SWIFT
import CoreGraphics
let target = "$proc_name"
let opts: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
guard let windows = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else { print("FAIL"); exit(0) }
for w in windows {
  guard let owner = w[kCGWindowOwnerName as String] as? String, owner == target else { continue }
  let layer = w[kCGWindowLayer as String] as? Int ?? -1
  let bounds = w[kCGWindowBounds as String] as? [String: CGFloat] ?? [:]
  let num = w[kCGWindowNumber as String] as? Int ?? 0
  let width = bounds["Width"] ?? 0
  if layer == 0 && width > 200 {
    let x = Int(bounds["X"] ?? 0)
    let y = Int(bounds["Y"] ?? 0)
    let h = Int(bounds["Height"] ?? 0)
    let wi = Int(width)
    print("\(num),\(x),\(y),\(wi),\(h)")
    exit(0)
  }
}
print("FAIL")
SWIFT
)

  if [ "$info" = "FAIL" ]; then
    echo "Failed: no window for $proc_name"
    osascript -e "tell application \"$proc_name\" to quit" 2>/dev/null || true
    return 1
  fi

  IFS=',' read -r wid x y w h <<< "$info"
  if screencapture -x -l "$wid" "$out_file" 2>/dev/null; then
    echo "Saved $out_file (window capture)"
  elif screencapture -x -R "$x,$y,$w,$h" "$out_file" 2>/dev/null; then
    echo "Saved $out_file (region capture)"
  else
    echo "Failed: enable Screen Recording for Terminal in System Settings → Privacy & Security"
    osascript -e "tell application \"$proc_name\" to quit" 2>/dev/null || true
    return 1
  fi

  osascript -e "tell application \"$proc_name\" to quit" 2>/dev/null || true
  sleep 1
}

echo "Capturing Matilda Piano journey screenshots → $OUT"
echo ""

capture_app "$BASE/Matilda Piano.app" "Matilda Piano" "$OUT/03-v1-sample-ui-built.png"
capture_app "$BASE/Matilda Piano 2.app" "Matilda Piano 2" "$OUT/04-v2-physical-ui-built.png"
capture_app "$BASE/Matilda Piano 3.app" "Matilda Piano 3" "$OUT/09-v3-neural-ui-built.png"

echo ""
echo "Done. Open docs/case-study-images/README.md for the full journey map."
