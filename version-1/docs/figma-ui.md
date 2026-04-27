## Figma UI reference (Matilda)

- **File**: `AdMaker-CMS`
- **Frame node**: `4203:94317`
- **URL**: [AdMaker-CMS — frame 4203:94317](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317)

**Milestone (2026-02-27):** M2a + M2b + M2c done. GUI: left panel, chickenhead knobs, XY pad (rounded clip). Fonts load from bundle/user/project. Effect chain and XY pad active; delay shows "Off" at minimum. See `docs/MILESTONES.md`.

### How this repo uses Figma

- **Design size**: 1074×483. Font sizes and layout scale dynamically from this frame (see **Dynamic scaling** below).
- **Background**: Gradient + left panel image + inner shadow (all in code; see `docs/FIGMA-SPEC.md`).
- **Knobs**: Chickenhead style, drawn in code; **zero = 7 o'clock**, 270° clockwise to max. ADSR: white ring, purple marker; FX: black base, orange marker.
- **Keyboard bed**: Light blue strip (teal) drawn under the keys; keys span full width.

### Dynamic scaling (Figma interpolation)

Font sizes and layout are derived from the Figma frame size (1074×483). A scale factor `getFigmaScale() = min(currentWidth/1074, currentHeight/483)` is used so that when the host resizes the plugin window, text (60px → 20px → 16px → 12px → 11px in design) and control positions scale proportionally.

### Testing the AU in GarageBand

When you load the **AU** inside **GarageBand** (or any host), the plugin runs inside the host process. The host’s app bundle is not Matilda’s, so:

- **`Contents/Resources/Assets/...`** (e.g. `.../Assets/Fonts/Jacquard_24/Jacquard24-Regular.ttf`) **does not exist** for the AU. That path exists only inside the **built Standalone app**:  
  `.../Build/.../Standalone/Matilda Piano.app/Contents/Resources/Assets/...`  
  (CMake copies the project `Assets/` into the Standalone .app at build time.)

For the **AU in GarageBand**, assets are loaded from (in order):

1. **BinaryData** (embedded in the plugin at build time; works in any host).
2. **`~/Documents/MatildaPiano/Assets/`** — copy your project **`WebApp/Assets/`** here so the AU can load Images and Fonts from disk:
   - `~/Documents/MatildaPiano/Assets/Images/background-left.png`
   - `~/Documents/MatildaPiano/Assets/Fonts/Jacquard_24/Jacquard24-Regular.ttf`
   - etc.
3. **Project `Assets/`** — only when the current working directory is the WebApp folder (e.g. running Standalone from the project).

### Assets and fonts (project layout)

All assets live under **`WebApp/Assets/`** in the repo:

- **Left panel**: `Assets/Images/background-left.png` — BinaryData, or `~/Documents/MatildaPiano/Assets/Images/`, or `Assets/Images/` when cwd is project root.
- **XY pad**: `Assets/Images/xy-pad.png`.
- **Fonts**: `Assets/Fonts/Jacquard_24/Jacquard24-Regular.ttf`, `Assets/Fonts/Kode_Mono/static/KodeMono-Bold.ttf`, `Assets/Fonts/Inter/` (labels). Inter falls back to system sans if missing.

**Full spec**: Element-level properties, colours, and layout are in **`docs/FIGMA-SPEC.md`**.

