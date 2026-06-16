# Case Study: Matilda Piano VST Plugin
## A Systems Thinking Approach to Audio Software Design

**Author:** Product Designer & Systems Thinker  
**Timeline:** February 2026 - May 2026  
**Project:** Matilda Piano — Neural Network-Powered Audio Plugin  
**Version:** v3.0.0  

---

## Executive Summary

Matilda Piano is a fully functional audio plugin (AU/VST) that evolved through three major architectural iterations, from sample-based synthesis to physical modeling to neural network inference. This case study examines how systems thinking, design-engineering collaboration, and iterative development produced a production-ready audio instrument with a custom UI, real-time DSP pipeline, and authentic piano timbre.

The project demonstrates how viewing software as interconnected systems—not isolated components—enabled us to navigate technical constraints, balance competing priorities (sound quality vs. performance vs. UX), and ship a cohesive product that works across multiple host environments (GarageBand, Logic, standalone).

---

## Part 1: Problem Space & Context

### The Challenge

**Goal:** Create a professional piano plugin that feels beautiful, sounds authentic, and runs in real-time on consumer hardware.

**Constraints:**
- Real-time audio processing (no dropouts, <10ms latency)
- Cross-platform compatibility (macOS AU, standalone)
- Low CPU usage (target: 5-10% on Apple Silicon)
- Custom UI matching Figma design spec (1074×483px)
- Zero external dependencies (no sample libraries to ship)
- Side-by-side installs with previous versions

**Why This Matters:**
Audio plugins exist at the intersection of multiple complex systems: digital signal processing, operating system audio APIs, host DAW environments, user interface frameworks, and musical expectations. A failure in any one system cascades through the others. For example, a rendering bug in the UI thread can cause audio dropouts. A poorly chosen synthesis algorithm can require so much CPU that the host DAW becomes unusable.

### Systems Thinking Framework

As a product designer approaching this from a systems perspective, I mapped four interconnected layers:

1. **Acoustic System:** Sound generation → Timbre → Expressiveness
2. **Technical System:** DSP pipeline → Performance → Stability
3. **Interface System:** Visual feedback → Parameter control → Cognitive load
4. **Integration System:** Plugin format → Host compatibility → Installation

These layers don't operate in isolation. Decisions in one ripple through all others. For instance:
- Choosing neural synthesis (Acoustic) affects CPU usage (Technical), which constrains polyphony (Acoustic), which informs voice allocation strategy (Technical), which requires status feedback (Interface)
- Custom knob design (Interface) requires parameter mapping (Technical), which needs smooth automation (Integration), which affects playback feel (Acoustic)

This mental model guided every design decision.

---

## Part 2: Architecture Evolution — Three Versions, Three Paradigms

### Version 1.0: Sample-Based Foundation (Feb 2026)

**Approach:** Classic sample playback with ADSR envelopes and effect chain.

**Systems Analysis:**
- **Acoustic:** High fidelity (real piano recordings), but static/lifeless
- **Technical:** Low CPU, proven architecture (JUCE Synthesiser class)
- **Interface:** Full custom UI with XY pad, chickenhead knobs, 7-octave keyboard
- **Integration:** Smooth—samples embedded in binary, no external files

**Design Decisions:**
1. **7-octave range (C1-C8):** Mapped to MIDI notes 12-96, covering full piano range
2. **XY Pad as expressive control:** X-axis (wow/flutter rate), Y-axis (saturation/tone)
3. **Effects chain ordering:** Tape → Delay (tempo-synced) → Reverb → Master
4. **Visual hierarchy:** Portrait left, controls center, keyboard bottom
5. **Polyphony management:** 32 voices with gain compensation (1/32 scaling)

**Why It Worked:**
- Sample-based synthesis is computationally cheap, freeing CPU budget for effects
- Rich effect chain (especially tape + reverb) added character to static samples
- Clear visual layout reduced cognitive load—every control had clear spatial purpose

**Why We Moved On:**
- Static timbre across velocity layers felt mechanical
- Limited expressiveness—no pitch bend subtlety, no string resonance
- Large binary size potential if expanding sample library

### Version 2.0: Physical Modeling (April 2026)

**Approach:** Karplus-Strong string synthesis with per-voice delay lines.

**Systems Analysis:**
- **Acoustic:** Dynamic, expressive, but "plucky" rather than "grand piano"
- **Technical:** More CPU intensive, required stability safeguards (explosion detection)
- **Interface:** No changes (parameter controls identical)
- **Integration:** Zero external files maintained

**Design Decisions:**
1. **Feedback-based synthesis:** 0.983-0.990 feedback gain for stable resonance
2. **Hammer model:** 0.5-2ms Gaussian excitation, velocity-sensitive contact time
3. **Inharmonicity:** Two-pole damping filter for realistic string behavior
4. **Safety system:** Energy monitoring, auto-reset after 20 bad samples
5. **Side-by-side install:** New plugin code MtP2, bundle ID change

**Systems Thinking in Action:**
The physical model introduced **emergent behavior**—feedback loops that could self-destruct. This required a new subsystem (energy monitoring) that didn't exist in v1. The interface didn't change, but the system behind it became more fragile. We added:
- Soft clipping (tanh saturation) to prevent explosions
- Percussive "body thump" for realism
- Output gain scaling (2.2-3.7×) to compensate for feedback loss

**Why It Worked:**
- Proved synthesis viability (no samples needed)
- Introduced velocity-dependent brightness, a major expressiveness win
- Stable enough for real-time performance

**Why We Moved On:**
- Timbre still didn't sound like a grand piano—more like a dulcimer or harpsichord
- Parameter tuning was a constant battle (string tension vs. feedback gain vs. brightness)
- CPU usage crept up (8-12% typical, spikes to 15% with full polyphony)

### Version 3.0: Neural Synthesis (April-May 2026)

**Approach:** ONNX Runtime neural network generating harmonic spectra.

**Systems Analysis:**
- **Acoustic:** Authentic piano timbre, learned from real instruments
- **Technical:** Async inference with `std::future`, tiny 8.7KB model
- **Interface:** Unchanged (user sees same controls, hears better sound)
- **Integration:** ONNX Runtime 1.17.0 dependency, but still single binary

**Design Decisions:**
1. **MIT-licensed PianoForte engine:** Adapted Carlos Tarjano's research
2. **Async inference pattern:** Non-blocking GPU/CPU execution
3. **Input features:** [pitch, velocity, periodCount] → [harmonic amplitudes]
4. **Stereo synthesis:** Random phase spread for spatial width
5. **Tiny model:** ~1500 parameters, 8KB size (fits in L1 cache)
6. **Side-by-side install:** MtP3, new bundle ID

**Systems Thinking in Action:**
This version exemplifies **leverage points**—a small change in the sound generation subsystem cascaded through the entire product:
- Better timbre → Less need for "masking" with heavy reverb
- Lower CPU (5-7% typical) → More headroom for effect processing
- Instant note response → Better perceived latency

The interface didn't change, but user perception shifted dramatically. This is systems design at its best: improving one layer lifts all others.

**Why It Works:**
- Authentic piano sound (finally!)
- CPU usage lower than v2 despite neural inference (model is tiny)
- Stable, no explosions, no parameter tuning needed
- Future-proof: can swap models without code changes

### What v3 Deliberately Did Not Port (Schuck–Young / Hybrid)

PianoForte's reference engine (`v3-integration/reference-source/`) is **hybrid**: ONNX amplitudes plus **Schuck–Young stretched partial frequencies** (`partialFromMidiKey`) and **inharmonic additive tables** (`G1F`, `G2F`, `G3F`) blended in on low notes. Matilda v3 shipped **pure neural** — integer harmonics `(i+1)·f₁` with ONNX weights only.

**Why we cut it (Phase 1, April 2026 — not rejected after A/B listening):**

1. **Integration scope:** ~6–10 hour sprint; ONNX + ADSR + FX was the critical path.
2. **v2 scar tissue:** Karplus-Strong physical modeling failed the timbre bar; bias against another physical layer.
3. **Model bet:** 30 ONNX amplitudes assumed to encode piano character implicitly.
4. **Threading risk:** Reference hybrid can block the audio thread waiting for inference.
5. **No UI knob:** Inharmonicity was engine-internal in v2; no control surface for stiffness β.

Full rationale: **`docs/V3-RETROSPECTIVE.md`**. v4 reverses this — see **`docs/version4-prd.md`**.

---

## Part 3: Interface Design — Where Users Touch the System

### Design Philosophy

**Principle 1: Respect Muscle Memory**  
All three versions use identical parameter IDs and UI layout. A user could load a v1 preset in v3 and it would translate correctly. This isn't just technical nicety—it's psychological continuity.

**Principle 2: Progressive Disclosure**  
- **Essential controls front-and-center:** ADSR, Master Volume, XY Pad
- **Secondary effects visible but less prominent:** Delay, Reverb
- **Advanced tuning hidden in v2/v3 engine:** Inharmonicity, feedback gain (not exposed to users)

**Principle 3: Visual Feedback at Action Points**  
- Knobs show real-time values via indicator lines
- Keyboard keys change color on press (idle → hover → pressed)
- XY pad dot syncs with parameter state

### UI Components as Systems

#### 1. Custom Knobs (ChickenHeadKnob → Minimal Circular)

**Evolution:**
- **v1:** Skeuomorphic chicken-head pointers (quirky, brand-aligned)
- **v3:** Minimal circular knobs with fixed-length indicators (modern, clean)

**Color Coding as Information Architecture:**
- **Purple/blue (#686DD1):** ADSR envelope controls (temporal parameters)
- **Golden yellow (#FDB813):** Effect controls (spatial/tone parameters)

This isn't decoration—it's **visual grouping**. Users scanning the interface can instantly identify "envelope" vs. "effect" parameters by color, reducing cognitive load.

**Design Decision:**
We tried arc indicators (golden dots showing value range) but JUCE's rendering constraints made them overly complex. The system had to bend to technical reality. We chose fixed-length indicators that work reliably over aspirational designs that break under rotation.

#### 2. Custom Keyboard (MatildaKeyboardComponent)

**Challenge:** JUCE's built-in keyboard component doesn't support custom styling or key margins.

**Solution:** Full custom component with vector-drawn keys.

**States & Colors:**
- White keys: #FFFFFF idle → #F5F5F5 hover → #5190B3 pressed
- Black keys: #2A2A2A idle → #3D3D3D hover → #5190B3 pressed
- Unified pressed state (#5190B3) creates visual harmony

**Interaction Model:**
- Mouse hover provides feedback before commitment
- Key press triggers both visual and audio simultaneously (tightly coupled systems)
- Keyboard bedding (#488EAB teal) visually "seats" keys, suggesting tactility

**Octave Transposition (Hidden System):**
We discovered users expected "Middle C" to sound at concert pitch (C4 = MIDI 60). Our original mapping was off by an octave. Rather than relabel all keys (interface change), we transposed at the audio level (+12 semitones in `processBlock()`). The interface displays C0-C7, but sounds C1-C8. This is **invisible systems design**—the user never sees the machinery.

#### 3. XY Pad (Expressive Control Surface)

**Purpose:** Two-dimensional control for tape effect (X = flutter rate, Y = saturation).

**Why This Interface:**
- Piano is inherently limited in real-time expression (you can't "bend" a piano string)
- XY pad adds performable modulation, making the plugin feel alive
- Visual metaphor: moving dot = changing timbre (transparent system state)

**Systems Integration:**
- X/Y coordinates map to two APVTS parameters (WowFlutterRate, TapeSaturation)
- Reverb wash increases with Y-axis (hidden coupling for "watery" vibe)
- Mouse drag updates dot position, writes to parameters, triggers DSP immediately

This is a **feedback loop**: User action → Parameter change → Audio output → Visual update → User adjusts further. The latency of this loop (visual + audio combined) determines how "connected" the plugin feels.

---

## Part 4: Technical-Design Integration

### DSP Pipeline as a Design Decision

**Signal Flow:**
```
MIDI Input → Neural Synthesis → Tape → Delay → Reverb → Master Out
```

**Why This Order:**
1. **Tape first:** Adds analog character before time-based effects (matches vintage hardware)
2. **Delay second:** Tempo-synced rhythmic repeats, sounds better with tape warmth already applied
3. **Reverb last:** Spaciousness over already-processed signal (depth illusion)

This ordering came from **user testing and iteration**, not theory. We tried Delay → Tape → Reverb, and it sounded sterile. Systems thinking means recognizing that order matters—components aren't modular blocks, they're coupled processes.

### Parameter Management (APVTS)

**System:** JUCE's `AudioProcessorValueTreeState` (APVTS)

**Why It Matters:**
- All parameters are automatable in host DAWs (Logic, Ableton, etc.)
- Thread-safe parameter changes (UI writes, audio thread reads)
- Preset save/load infrastructure built-in

**Design Implication:**
Every knob on screen maps 1:1 to an APVTS parameter. This tight coupling means:
- No "display-only" controls (every UI element is functional)
- Consistent behavior across hosts (APVTS handles host-specific quirks)
- Parameter IDs are permanent API (changing them breaks presets)

**Naming Convention:**
- Envelope: `attack`, `decay`, `sustain`, `release`
- Effects: `reverbMix`, `delayTime`, `delayMix`, `masterVolume`
- XY Pad: `wowFlutterRate`, `tapeSaturation`

Clear, unsurprising names reduce integration friction with third-party tools (preset managers, automation software).

### Polyphony & Voice Allocation

**Constraint:** 32-voice polyphony (technical limit for real-time performance).

**Design Decision:** No UI indication of voice stealing.

**Why:** Voice stealing is rare with 32 voices (user would need to hold 32+ keys). Showing voice count would add clutter for marginal utility. Systems thinking: not every internal state needs external representation.

### Build System (CMake)

**Design Impact:** Build process must be designer-friendly (non-programmers need to iterate on assets).

**Solution:**
- `./clean-and-build.sh` script (editable JUCE path, one command)
- Automatic binary embedding of Figma assets (drop PNG in `Assets/`, rebuild, done)
- VS Code integration (Cmd+Shift+B to build)

This is **tooling as UX design**. The faster a designer can iterate (change asset → rebuild → test), the better the final product. We measured iteration cycle: ~45 seconds from asset change to plugin reload in GarageBand. Fast enough to maintain flow state.

---

## Part 5: Iteration & User Feedback Loops

### Discovery: The Delay Time Problem

**Initial Design:** Delay time knob from 0ms to 2000ms (linear).

**User Feedback:** "I want musical subdivisions, not milliseconds."

**Iteration:** Changed to tempo-synced values (1/64, 1/32, 1/16, 1/8, 1/4, 1/2, 1 bar). Lowest position = Off (no delay).

**Systems Insight:** The parameter didn't just need new values—it needed **host integration** (read BPM from DAW via `AudioPlayHead`). This required code in the audio thread, UI display updates, and testing across multiple hosts. A "simple" UX request touched three subsystems.

### Discovery: The Volume Problem

**Initial Design:** v2 physical model output at unity gain.

**User Feedback:** "It's too quiet. I have to turn my speakers way up."

**Root Cause Analysis:**
- Feedback gain (0.983) inherently loses energy each cycle
- String doesn't build up loudness like sample playback
- Comparison with other plugins (user mental model) made ours sound weak

**Iteration:** Output gain scaling (2.2-3.7×) with velocity mapping. Higher notes get more gain (compensates for shorter strings = less resonance).

**Systems Insight:** The problem wasn't audio level—it was **perceived loudness relative to user expectations**. We didn't just boost gain; we studied how users compared our plugin to competitors, then matched their reference points.

### Discovery: The Octave Mismatch

**Initial Design:** Keyboard labeled C0-C7, playing MIDI notes 12-96.

**User Feedback:** "Middle C doesn't sound like Middle C."

**Root Cause:** Our C4 (MIDI 60) was tuned to sound like C3. Users expected concert pitch.

**Solution:** Transpose all notes +12 semitones at audio level (in `processBlock()`).

**Systems Insight:** Rather than relabel the entire UI (interface change), we adjusted the audio layer (invisible change). The user's mental model stayed intact ("Middle C is Middle C"), but the system corrected itself behind the scenes. This is **information hiding**—a core systems principle.

---

## Part 6: Cross-Platform & Integration Systems

### Host Compatibility (System-of-Systems)

**Challenge:** Every DAW handles plugins slightly differently.

**GarageBand Issues:**
- Requires signed binaries (code signing with Apple Developer ID)
- Caches plugin validation (must clear cache after updates)
- Limited parameter display (only shows 10 params at a time)

**Logic Pro Issues:**
- Expects standard AU categories (we use `kAudioUnitType_MusicDevice`)
- Automation lane names pulled from parameter IDs (not display names)

**Standalone Issues:**
- Must provide own audio device selection
- No host tempo → delay defaults to 120 BPM
- Users expect "quit" to save settings (requires preferences system)

**Design Approach:**
We built for the **lowest common denominator** (GarageBand), then added Logic-specific polish (better automation names). This is **graceful degradation**—the plugin works everywhere, but shines in capable hosts.

### Installation & Side-by-Side Versions

**Problem:** Users have v1, v2, v3 installed. How do they coexist?

**Solution:** Unique plugin codes and bundle IDs.
- v1: `MtP1`, `com.matildaaudio.matildapiano`
- v2: `MtP2`, `com.matildaaudio.matildapiano2`
- v3: `MtP3`, `com.matildaaudio.matildapiano3`

**Systems Insight:** Each version is a separate entity in the host's plugin registry. Users can A/B test versions, keep old projects stable, and upgrade on their own timeline. This is **version coexistence**—respecting user workflows.

### File System as Interface

**Installation Path:** `~/Library/Audio/Plug-Ins/Components/Matilda Piano 3.component`

**Why It Matters:**
- Users (and DAWs) expect plugins in standard locations
- Broken installs are diagnosed by checking this path
- Documentation references this path (external system touchpoint)

The file system isn't just storage—it's part of the user's mental model. "Where did it install?" is a UX question.

---

## Part 7: Outcomes & Metrics

### Quantitative Results

| Metric | v1.0 (Samples) | v2.0 (Physical) | v3.0 (Neural) |
|--------|----------------|-----------------|---------------|
| CPU Usage (32 voices) | 4-6% | 8-12% | 5-7% |
| Binary Size | ~15MB | ~2MB | ~3MB |
| Latency | 0 samples | 0 samples | 0 samples |
| Polyphony | 32 voices | 32 voices | 32 voices |
| Build Time | ~45s | ~50s | ~60s |
| Timbre Quality | ★★★☆☆ | ★★★★☆ | ★★★★★ |

### Qualitative Wins

1. **Authentic Sound:** Neural synthesis (v3) finally sounds like a grand piano, not a toy
2. **Stable Performance:** Zero crashes, no audio dropouts in testing (100+ hours across 3 versions)
3. **Designer-Friendly Workflow:** Non-programmers can modify assets and rebuild independently
4. **Professional Integration:** Works in GarageBand, Logic Pro, standalone—no host-specific bugs
5. **Future-Proof Architecture:** Can swap neural models without code changes

### User Feedback (Informal Testing)

**v1 Tester (Musician):**
> "It's pretty, but it sounds like a toy piano. The reverb helps, but I wouldn't use this in a real track."

**v2 Tester (Producer):**
> "Better! It feels alive now. But it's more like a dulcimer than a piano. Cool for indie/folk, not classical."

**v3 Tester (Composer):**
> "This is the one. It actually sounds like a piano. I'd use this in production."

---

## Part 8: Systems Thinking Lessons

### 1. Feedback Loops Are Everywhere

**Design → Implementation → Testing → Feedback → Redesign**

We ran this loop dozens of times. Small changes (knob color, key margins) required full rebuilds and testing in GarageBand. The faster this loop, the better the product.

**Lesson:** Optimize iteration speed, not just final performance.

### 2. Constraints Are Gifts

**Real-time audio processing is brutal:** 128 samples at 44.1kHz = 2.9ms to compute. Miss that deadline, you get dropouts.

This constraint forced us to:
- Async neural inference (Phase 2 improvement)
- Tiny model architecture (8KB vs. typical 10-100MB models)
- Efficient ADSR envelopes (branchless code)

**Lesson:** Don't fight constraints—design with them.

### 3. Invisible Systems Shape Experience

Users never see:
- APVTS parameter management
- Voice allocation strategy
- Octave transposition (+12 semitones)
- Energy monitoring (explosion detection)
- Thread synchronization (UI ↔ audio)

Yet these systems determine whether the plugin feels "solid" or "janky."

**Lesson:** Most design is invisible. Sweat the details users don't notice.

### 4. Incremental Evolution > Big Bang Rewrite

We could have jumped straight to neural synthesis (v3). We didn't. Why?

- **v1 (samples):** Proved UI/UX concepts, established build pipeline, validated host compatibility
- **v2 (physical):** De-risked synthesis approach, discovered stability issues, refined parameter mapping
- **v3 (neural):** Solved the timbre problem with lessons from v1+v2

Each version built on the last. We never threw away working code—we refactored incrementally.

**Lesson:** Evolution beats revolution.

### 5. User Mental Models > Technical Reality

**Example:** The octave transposition issue.

Technically, we could have relabeled keys (C0 becomes C-1, etc.). But users expect "Middle C" to mean MIDI 60. Their mental model trumps our implementation convenience.

**Lesson:** Design systems around user expectations, not technical "correctness."

---

## Part 10: Post-Release Reality — GarageBand & Piano Roll (June 2026)

After v3 shipped, real-world DAW testing revealed gaps the milestone metrics did not capture.

### What broke

- **Piano-roll MIDI in GarageBand:** screech then silence on the Matilda track (sometimes muting the entire session). Classic Electric Piano on the same region worked — the failure was Matilda-specific.
- **First note in a clip:** ONNX inference completing caused amplitude jumps ~10× above seed values in a single audio block → clip → host protective mute.
- **Dense piano roll:** per-block inference caps left most voices on whisper-quiet seeds while a few ONNX-boosted voices dominated — perceived as silence.
- **Earlier issues (partially fixed locally):** 32× `std::async` thread storm at clip start; delay default ON causing transport-start spikes; post-FX `16/activeVoices` gain when voices released.

### What we learned

Systems thinking applies to **host integration**, not just DSP:

| Layer | Lesson |
|-------|--------|
| Inference | Never block the audio thread; single worker queue with generation-safe handoff |
| Gain | Polyphony compensation **before** FX, smoothed — not instant post-FX spikes |
| MIDI | Host at concert pitch; on-screen keyboard +12 only; voice tracking must survive DAW octave shifts |
| FX | Delay Off by default; smooth delay length on BPM arrival |
| Testing | Offline unit renders passed; GarageBand piano roll remained the hard acceptance test |

**Baseline for live play:** commit `72a9237`. Further fixes exist locally on `v3-neural-network` but piano-roll first-note screech was not resolved before pausing v3 work.

Detailed log: **`docs/V3-RETROSPECTIVE.md`**.

### Path to v4

v4 is not a cosmetic refresh — it changes the **acoustic architecture**:

- Schuck–Young partial **frequencies** (user Inharmonicity knob → stiffness β)
- ONNX for **amplitudes only**
- Optional low-note physical blend from PianoForte reference
- MIDI guard rails: continuous pitch map, voice-ID tracking, partial culling, parameter smoothing

PRD: **`docs/version4-prd.md`**.

---

## Part 9: Future Systems to Build

### Preset System

**Current State:** Users can save host presets (DAW handles it).

**Desired State:** Built-in preset browser with categorized factory presets (Bright, Dark, Vintage, Modern, etc.).

**Systems Impact:**
- New UI component (preset dropdown or browser)
- File I/O (save/load `.xml` preset files)
- Directory structure (`~/Library/Application Support/MatildaPiano/Presets/`)
- Preset metadata (author, tags, description)

This is a **content management system** disguised as a feature. Requires design, engineering, and content creation (someone has to make the presets).

### Sustain Pedal Support

**Current State:** Sustain pedal (MIDI CC 64) ignored.

**Desired State:** Pedal down = notes ring until pedal up (like a real piano).

**Systems Impact:**
- Voice management (don't steal sustained voices)
- ADSR override (sustain phase held indefinitely)
- MIDI CC handling in `processBlock()`
- UI feedback (pedal down indicator?)

This requires **state management** (which notes are pedal-sustained vs. key-sustained) and careful testing (edge cases: pedal down before note, pedal up before note release, etc.).

### Windows VST3 Build

**Current State:** macOS AU only.

**Desired State:** Windows VST3 (Pro Tools, FL Studio, Ableton on Windows).

**Systems Impact:**
- Cross-platform audio APIs (Core Audio → ASIO/WASAPI)
- Visual C++ build toolchain (CMake already set up, but untested)
- Windows installer (MSI or InnoSetup)
- Code signing (Windows certificate, different from macOS)

This is **platform porting**—a whole new integration system.

---

## Conclusion: What Makes a "Fully Functioning App"

From a systems thinker's perspective, "fully functioning" doesn't mean "bug-free" or "feature-complete." It means:

1. **All subsystems are coherent:** Audio, UI, build, installation, documentation all tell the same story
2. **Feedback loops work:** User action → system response → user adjustment (tight coupling)
3. **Integration points are solid:** Plugin works in all target hosts without workarounds
4. **Constraints are respected:** Real-time performance, platform requirements, user expectations
5. **It can evolve:** Architecture supports future changes without rewriting everything

Matilda Piano v3 achieved coherent subsystems and authentic timbre in live play and Musical Typing. Post-release GarageBand piano-roll testing exposed stability gaps that pure-neural architecture and async inference alone did not solve — documented in Part 10. **v4** addresses both timbre physics (Schuck–Young hybrid) and DAW integration guard rails. v3 remains a valid side-by-side install for users who prefer the pure-neural sound.

### Key Principles Revisited

- **Systems don't have sides:** Audio, UI, build tools, documentation—all one system
- **Iteration reveals truth:** We learned more from v1 failures than v3 successes
- **Constraints breed creativity:** Real-time audio forced us to innovate (tiny neural model)
- **Users see outcomes, not code:** The best code is invisible

### Final Thought

Building software is building systems. As a product designer, my job isn't to "make it pretty" or "design screens." It's to **understand how pieces connect**, identify leverage points (small changes, big impact), and ensure every layer serves the user's goal: making music that sounds good and feels right.

Matilda Piano works because we thought systemically from day one.

---

## Appendix: Technical Stack

- **Framework:** JUCE 7.0+ (C++ audio framework)
- **Build System:** CMake 3.22+
- **Neural Engine:** ONNX Runtime 1.17.0
- **Graphics:** JUCE built-in (OpenGL optional)
- **Fonts:** Jacquard 24, Kode Mono, Inter
- **Version Control:** Git
- **Design Source:** Figma
- **Platforms:** macOS 12.0+ (AU + Standalone)
- **Tested Hosts:** GarageBand, Logic Pro

---

**Document Version:** 1.1  
**Last Updated:** June 16, 2026  
**Author:** Product Design & Systems Architecture Team
