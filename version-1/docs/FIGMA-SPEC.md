# Figma → JUCE spec: Matilda Piano UI

**Figma:** [AdMaker-CMS — frame 4203:94317](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4203-94317)  
**Frame size:** 1074 × 483 px  
**Purpose:** Single source of truth for GUI implementation. Every element’s properties are listed so JUCE can match the design.

---

## 1. Root frame — “Matilda Design” (4203:94317)

| Property | Value | JUCE note |
|----------|--------|-----------|
| Size | 1074 × 483 px | Already used in `PluginEditor` |
| Corner radius | 50 px | Clip or draw rounded rect |
| Background | Gradient (linear, top→bottom) | See below |
| Inner shadow | 3 insets | See below |

### 1.1 Background gradient

- **Type:** Linear, vertical (top to bottom).
- **Stops:**
  - Top: `#1c1f45`
  - 46.154%: `#2b477d`
  - Bottom: `#61b2bf`

Use this when no background image is loaded (or as fallback).

### 1.2 Inner shadow (frame overlay)

Applied as a single overlay on the full frame (inset shadows):

| Shadow | Offset X | Offset Y | Blur | Color |
|--------|----------|-----------|------|--------|
| 1 | 0 | 16 | 4 | rgba(0,0,0,0.25) |
| 2 | 8 | 0 | 4 | rgba(0,0,0,0.25) |
| 3 | -8 | 0 | 4 | rgba(0,0,0,0.25) |

JUCE: draw these as inner shadows after background (or bake into a full-frame overlay image if easier).

---

## 2. Left panel — “pic” (4203:94433)

| Property | Value | JUCE note |
|----------|--------|-----------|
| Position | left -3 px, top 0 | |
| Size | 409 × 483 px | |
| Content | Masked image | Rectangle149479868 inside mask Rectangle149479867 |

**Assets:**

- **Rectangle149479868** — Main left-panel image (e.g. character/art). Used as content.
- **Rectangle149479867** — Mask shape (406×483). Content is drawn through this mask.

**What you need to do:** Export the **full left-panel art** (with or without mask applied) as one PNG at 409×483 (or 406×483) so JUCE can show it as the left section background. If the design uses a photo/illustration, that single exported image is enough; no need to ship the mask separately if the export is already masked.

---

## 3. Knob console (4203:94319, 4203:94320)

Position in frame: **inset 22.36% from top, 26.72% right, 31.88% bottom, 41.06% left** → in pixels (from 1074×483): roughly **left 441, top 108, right 287, bottom 154**, so the **Knob Console** content is centered in that box.

Layout: flex column, **gap 20 px** between ADSR row and Effects row.

### 3.1 ADSR panel (4203:94321)

- **Layout:** Row, gap **30 px**, height **77 px**.
- **Knobs:** 4 items (Attack, Decay, Sustain, Release), each **51 × 51 px**, **11 px** gap to label below.

#### Attack (4203:94322), Decay (4203:94327), Sustain (4203:94331), Release (4203:94335)

Each knob unit:

| Element | Figma node | Asset / style | JUCE note |
|---------|------------|----------------|-----------|
| Knob base (outer ring) | 4203:94323 (etc.) | **imgEllipse21872** | Same for all 4 ADSR knobs |
| Label text | 4203:94324, 94329, 94333, 94337 | Font below | Attack, Decay, Sustain, Release |
| Indicator line | 4203:94325, 94330, 94334, 94338 | **imgLine10** (Attack), **imgLine11** (others) | Rotation = value (e.g. -161° Attack, 135° others in design) |
| Inner dot / highlight | 4203:94326, 94344… | **imgEllipse21874** | Small ellipse overlay |

**ADSR label typography:**

- Font: **Inter**, style **Regular**
- Size: **12 px**
- Color: **white**
- Align: center

**Assets to export (ADSR):**

- **Ellipse21872** — White outer ring (51×51 or with bleed).
- **Line10** — Indicator line for one angle (e.g. Attack).
- **Line11** — Indicator line for another angle (Decay/Sustain/Release).
- **Ellipse21874** — Inner knob highlight/dot.

You can either export a **strip or set of knob frames** for different rotation angles (e.g. 0°, 45°, …) and have JUCE pick the frame by value, or export the **base + one indicator line** and have JUCE rotate the line (or redraw) by parameter value.

---

### 3.2 Effects panel (4203:94339)

- **Layout:** Row, gap **30 px**. Same vertical gap **20 px** below ADSR.
- **Knobs:** 3 items (Reverb, Delay, Master vol.), each **51 × 51 px**, **11 px** gap to label.

#### Reverb (4203:94340), Delay (4203:94345), Master volume (4203:94350)

| Element | Figma node | Asset / style | JUCE note |
|---------|------------|----------------|-----------|
| Knob base | 4203:94341, 94346, 94351 | **imgEllipse21873** | Dark/green base for all 3 |
| Label | 4203:94342, 94347, 94352 | See typography | Reverb; Delay + (1/3T); master vol. |
| Indicator line | 4203:94343, 94348, 94353 | **imgLine12** | Same asset for all 3 |
| Inner highlight | 4203:94344, 94349, 94354 | **imgEllipse21875** | Orange/dark inner |

**Effects label typography:**

- **Reverb:** Inter Regular, 12 px, white, center.
- **Delay (4203:94347):**  
  - “Delay” + newline: Inter Regular, 12 px, white.  
  - “(1/3T)”: **Inter Bold**, 12 px, white. Lowest knob position shows **(Off)**; above that subdivision is dynamic (1/64, 1/32, … 1) in JUCE.
- **master vol.:** Inter Regular, 12 px, white, center, **capitalize** (so “Master vol.” in UI).

**Assets to export (Effects):**

- **Ellipse21873** — Dark/green knob base (51×51).
- **Line12** — Orange (or accent) indicator line.
- **Ellipse21875** — Inner knob highlight.

Same as ADSR: either multi-frame strip for angles or base + line and JUCE draws/rotates by value.

---

## 4. Top-right label — “Matilda” + “v1.0” (4286:136666, 4203:94355, 4286:136646)

| Property | Value | JUCE note |
|----------|--------|-----------|
| Position | left 853 px, top 15.09 px | |
| Width | 160 px | |
| Align | Right | |

**Typography:**

- **“Matilda”** (4203:94355): **Jacquard_24**, Regular, **60 px**, white, right-aligned.
- **“v1.0”** (4286:136646): **Jacquard_24**, Regular, **20 px**, white, right-aligned.

**Font:** **Jacquard 24** (Google Fonts: “Jacquard 24”). You need the font file(s) (e.g. TTF/OTF) in a project folder and JUCE must load it (e.g. from BinaryData or a known path).

---

## 5. Grand Piano label (4203:94356)

- **Position:** Horizontal center, top **35 px** (centered in X).
- **Content:** Three text nodes + one vector:
  - “<” — Jacquard_24, Regular, **20 px**, white (4203:94357).
  - “Grand piano” — **Kode Mono**, **Bold**, **16 px**, white, **uppercase**, center (4203:94358).
  - “>” — Jacquard_24, Regular, **20 px**, white (4203:94359).
  - **Vector243** (4203:94360) — Decorative underline/background under the text (e.g. dark bar).

**Fonts:**

- **Jacquard_24** — for “<” and “>”.
- **Kode Mono** (Google: “Kode Mono”) — Bold, 16 px, for “GRAND PIANO”.

**Asset:**

- **Vector243** — Export as PNG or SVG for the underline/decoration behind “< GRAND PIANO >”.

---

## 6. Piano keyboard — “keys” (4203:94361)

| Property | Value | JUCE note |
|----------|--------|-----------|
| Position | inset 75.36% from top, 0 right, -0.41% bottom, -0.19% left | Effectively: from top 364 px down, full width |
| Asset | **imgKeys** | Full keyboard graphic |

**Asset:** **Keys** — Export the full keyboard as one image (same width as frame or 1074 px, height to match design). JUCE can use it as the keybed background and draw key highlights on top, or you can try to match key colors and shape in code; exporting the asset guarantees a visual match.

---

## 7. XY controller (4286:136645)

| Property | Value | JUCE note |
|----------|--------|-----------|
| Position | left 834 px, top 119 px | |
| Size | 198 × 192 px | |
| Asset | **imgXyController** | Full XY pad as one image |

**Asset:** **XY Controller** — Export as one PNG (198×192). The design is a single asset (crosshair + dot + border). JUCE can either:

- Use it as a static background and draw only the **movable dot** by parameter (X,Y), or  
- Use it as full asset and overlay a dot whose position is driven by parameters.

---

## 8. Fonts summary — what you must provide

All text in the frame uses these fonts. JUCE does not ship them; you need to add them to the project.

| Font name (Figma) | Use | Size(s) | Style | Where to get |
|-------------------|-----|---------|--------|----------------|
| **Inter** | ADSR labels, Reverb, Delay, master vol. | 12 px | Regular, Bold (for delay subdivision) | [Google Fonts](https://fonts.google.com/specimen/Inter) — Inter-Regular.ttf, Inter-Bold.ttf |
| **Jacquard 24** | “Matilda”, “v1.0”, “<”, “>” | 20 px, 60 px | Regular | [Google Fonts](https://fonts.google.com/specimen/Jacquard+24) |
| **Kode Mono** | “GRAND PIANO” | 16 px | Bold | [Google Fonts](https://fonts.google.com/specimen/Kode+Mono) |

**What to do:**

1. Download the font files (TTF or OTF) for **Inter** (Regular + Bold), **Jacquard 24** (Regular), **Kode Mono** (Bold).
2. Put them in the project folder **`Assets/Fonts/`** (e.g. `Assets/Fonts/Jacquard_24/Jacquard24-Regular.ttf`). All assets in this repo live under **`WebApp/Assets/`**.
3. Either embed via JUCE BinaryData or load from disk. When testing the **AU in GarageBand**, the path `Contents/Resources/Assets/...` does not apply (that exists only inside the built **Standalone** .app). For the AU, use BinaryData or copy `Assets/` to **`~/Documents/MatildaPiano/Assets/`** (see `docs/figma-ui.md`).

No other fonts appear in this frame.

---

## 9. Image / vector assets summary — what to export from Figma

Export at **1×** (or 2× for retina if you prefer; document the scale). Prefer PNG for raster, SVG only if JUCE or your pipeline can use it.

| Figma name / node | Suggested export name | Use |
|-------------------|------------------------|-----|
| Rectangle149479868 | `background-left.png` or `panel-left.png` | Left panel art (with mask applied if desired) |
| Ellipse21872 | `knob-adsr-base.png` | ADSR white ring |
| Ellipse21873 | `knob-fx-base.png` | Reverb/Delay/Master base |
| Line10 | `knob-line-adsr-1.png` (or as part of a strip) | ADSR indicator (Attack) |
| Line11 | `knob-line-adsr-2.png` | ADSR indicator (Decay/Sustain/Release) |
| Line12 | `knob-line-fx.png` | FX knob indicator |
| Ellipse21874 | `knob-adsr-inner.png` | ADSR inner dot |
| Ellipse21875 | `knob-fx-inner.png` | FX inner dot |
| Vector243 | `grand-piano-underline.png` or `.svg` | “< GRAND PIANO >” decoration |
| Keys | `keyboard.png` | Piano keybed |
| XY Controller (4286:136645) | `xy-pad.png` | XY pad background + static graphics |

Optional: full **frame export** (1074×483) as `background.png` for a single “baked” background (gradient + left art + inner shadow). Then JUCE only draws controls and text on top. Otherwise use gradient + left panel image + inner shadow in code as above.

---

## 10. Layout numbers (for implementation)

All from design context; frame = 1074×483.

| Element | x | y | w | h |
|---------|------|------|------|------|
| Left panel (pic) | -3 | 0 | 409 | 483 |
| Knob Console (content) | (centered in inset box) | ~108 | — | — |
| ADSR knobs | 0, 81, 162, 243 from group origin | 0 | 51 | 51 |
| ADSR labels | same X as knobs | 62 | 51 | 15 |
| FX knobs | 0, 81, 162 from group origin | 97 | 51 | 51 |
| FX labels | same X as knobs | 97+62 | 51 | 15–30 |
| Matilda label | 853 | 15 | 160 | — |
| Grand Piano | center, 35 | — | — | — |
| Keyboard | 0 | 364 | 1074 | 119 |
| XY Controller | 834 | 119 | 198 | 192 |

Group origin for knobs: derive from “Knob Console” center in the inset box (22.36% / 26.72% / 31.88% / 41.06%). Current code uses group at (467, 124); confirm against Figma’s computed position and adjust if needed.

---

## 11. Resolved decisions

1. **Knob rotation range:** Knobs are drawn in code (no assets). **Reverted to JUCE default rotary angles** (standard full range). Chicken-head design (track, fill, pointer, colours) kept; only angle range reverted.
2. **Background:** Implemented in code: **gradient + left panel image** (from Assets folder or BinaryData) **+ inner shadow**. No single baked frame PNG.
3. **Fonts and images:** Provided in `Assets/Fonts/` and `Assets/Images/`. Whatever is not there (e.g. Inter if missing) is drawn with system/code fallback.
4. **XY pad:** Background image with aspect-ratio preservation; rounded-rect clip; sliders are non-painted (`addChildComponent`) so no sharp rectangular frame; dot indicator on top. XY drives tape effect (wow/flutter, saturation, tone); effect chain enabled.
5. **Delay:** Lowest knob position (≤5%) = **Off** (mix 0, label "Delay (Off)"); remainder maps to subdivisions 1/64…1. Effect chain (tape → delay → reverb → gain) is enabled by default.

**Implementation status (2026-02-27):** M2a + M2b + M2c complete. Fonts load from bundle Resources (Standalone), user/cwd Assets, or BinaryData; flat and nested paths supported. Effect module (tape, delay, reverb) and XY pad fully active; delay Off at minimum (see docs/MILESTONES.md, docs/architecture.md).

---

## 12. Will this approach work? (Blunt)

**Yes.** A single spec that lists every element, its props, fonts, and assets is the right way to get the GUI close to Figma.

**What you must do:**

- **Fonts:** Download Inter (Regular + Bold), Jacquard 24 (Regular), Kode Mono (Bold) and put them in a folder (or embed). JUCE cannot match type without the actual font files.
- **Assets:** Export the listed images/vectors from Figma and add them to the project (or user folder). Gradients and inner shadows can be implemented in JUCE from the numbers in this doc; the left panel, knobs, keyboard, and XY pad will look correct only if you use the exported assets (or redraw to match them exactly).

**What this doc does not do:** It does not implement anything. Implementation is in JUCE (and possibly build/CMake for font and asset embedding). Use this doc as the checklist: fonts, gradient, inner shadow, each asset, each label font/size/color, and layout numbers. When something doesn’t match, compare to this spec and to Figma.

**Suggested next step:** Create an `Assets/Fonts/` and `Assets/Images/` (or your preferred layout), add the font files and exported images, then in code: (1) load fonts, (2) set background (gradient + left image + inner shadow or single background.png), (3) replace knob drawing with the exported assets or redraw to match, (4) use the same font/size/color for every label, (5) use the XY and keyboard assets as specified. Revisit §11 after you’ve decided on knob range, delay label format, and background strategy.
