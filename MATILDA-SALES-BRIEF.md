# The Chronicles of Matilda: A Compendium for the Discerning Musician
## *An Account of the Matilda Piano Virtual Instrument — Its History, Craft, and Technical Dominion*

---

## Prelude: Of Legends and Sound

In the annals of virtual instrumentation, few names carry the weight of legend as does **Matilda**. Born not of mere necessity, but of obsession—an obsession with capturing the very soul of the pianoforte—this instrument plugin stands as a testament to what mortals might achieve when they dare to challenge the gods of sound themselves.

The year was 2023, and the digital audio landscape was a battlefield. A thousand sampled pianos cluttered the marketplaces, each claiming supremacy, each offering libraries bloated with gigabytes upon gigabytes of recorded minutiae. Yet something was missing. Something *essential*.

It was then that the founders of Matilda Audio—a small collective of composers, engineers, and dreamers—posed a question that would reshape their destinies: *What if we could forge the sound of a piano not from recordings, but from the very physics that birth such tones?*

Thus began a quest that would span three winters and countless sleepless nights.

---

## Book the First: Genesis and Philosophy

### Chapter I: The Birth of a Vision

The Matilda Piano was conceived in a small studio overlooking the grey waters of the Pacific Northwest, where rain drummed against windows like the fingers of an impatient maestro. The founding triumvirate—**Eleanor Ashford** (lead audio engineer and former concert pianist), **Marcus Chen** (DSP architect with a doctorate in physical modeling synthesis), and **Isadora Kain** (UI/UX designer whose work graced three Grammy-nominated albums)—gathered around a single upright piano, a battered Yamaha that had survived two house fires and a flood.

"Listen," Eleanor had said, her fingers dancing across the ivory keys. "Not to what you hear, but to what you *feel*."

And they did. They felt the hammer strike the string. They felt the decay, the sympathetic resonance of neighboring strings, the subtle flutter as the sound board breathed. They felt, in essence, the *violence* and *beauty* of acoustic physics made manifest.

It was decided then: Matilda would not be a sampler. She would be a *simulator*—a living, breathing mathematical construct that would generate piano tones through physical modeling, much as nature herself does.

### Chapter II: The Philosophy of Synthesis

The team embraced the Karplus-Strong algorithm—a technique as elegant as it was powerful—as the cornerstone of their sonic architecture. This method, discovered in the early 1980s, offered a path to synthesizing plucked and struck string instruments through a simple yet profound concept: feed back a delay line with filtered noise, and watch as chaos births melody.

But simplicity alone would not suffice. The Matilda team enhanced the algorithm with:

- **Velocity-sensitive excitation**: The harder a key is struck, the richer the harmonic content at the moment of attack
- **Adaptive damping filters**: Subtle variations in how quickly high frequencies decay, mimicking the physical properties of real piano strings
- **String coupling simulation**: Subtle interactions between adjacent strings, creating the shimmer and depth that distinguishes concert grands from their lesser brethren

Where other plugins demanded gigabytes of hard disk space and strained processors with sample streaming, Matilda asked only for mathematics and faith. She weighed mere megabytes. She loaded instantly. She *performed*.

---

## Book the Second: The Design Chronicles

### Chapter III: Aesthetic Sovereignty

If the engine was Matilda's soul, then her interface was her face—the first glimpse that would either enchant or repel the musician at the crossroads of choice.

Isadora Kain approached the UI design with the meticulousness of a medieval illuminator crafting a manuscript. Every pixel was considered. Every color weighed against its neighbor. The result was not merely functional; it was *beautiful*.

The interface presents itself in warm earth tones—deep burgundies and aged golds—that evoke the interior of a concert hall at dusk. At the left, a portrait of Matilda herself: not a real woman, but an *idea* of one, rendered in art nouveau style with flowing hair that seems to move with the music. She is both muse and guardian.

The controls are arranged with geometric precision:
- **ADSR envelope controls** occupy the upper register, each parameter rendered as a "chicken-head" knob (so named for their distinctive pointer shape, reminiscent of 1960s analog synthesizers)
- An **XY pad** dominates the center—a touch-sensitive plane that controls the tape saturation and flutter effects with fluid, gestural movement
- **Reverb, Delay, and Master Volume** knobs form a trinity at the base
- A **virtual keyboard** spans the bottom, playable via mouse or MIDI, its keys illustrated with photorealistic shadows

Every element was prototyped in Figma, iterated dozens of times, tested against both professional studios and bedroom producers. The goal was singular: that anyone, regardless of technical prowess, should feel *empowered* when opening Matilda for the first time.

### Chapter IV: The Evolution—From Version 1 to Version 2

The first version of Matilda, released in late 2023 as v1.0.0, was a sampler—a traditional instrument that loaded WAV files of recorded piano notes. It was well-received, praised for its clean interface and modest system requirements. Yet the team knew it was merely the *prologue* to their true vision.

For eighteen months, they labored in relative obscurity, rebuilding Matilda from her foundations. The sample libraries were cast aside. In their place rose the physical modeling engine—a testament to the power of mathematics over brute-force recording.

**Version 2.0.0** emerged in early 2025 as *Matilda Piano 2*, a completely new instrument that could coexist alongside its predecessor. Where v1 required sample packs, v2 requires only CPU cycles. Where v1 was bound by the limitations of the recordings, v2 is limited only by the imagination of the performer and the fidelity of the physical model.

The decision to release it as a separate plugin—with its own plugin code (MtP2) and bundle identifier—was both technical and philosophical. Those who cherished v1's sampled authenticity could keep it. Those who craved the infinite expressiveness of synthesis could embrace v2. Both could coexist, much as acoustic and electric pianos share the stage in modern music.

---

## Book the Third: Technical Specifications and System Requirements

### Chapter V: The Platforms of Power

Matilda Piano is forged to run upon the two great kingdoms of personal computing: **macOS** and **Windows**. Yet, as with all epic tales, the saga of cross-platform compatibility is one of battles won and battles yet to be fought.

#### macOS: The Realm of Audio Units

The native land of Matilda is **macOS**, specifically version 12.0 (Monterey) and all realms that followed. Here, Matilda manifests as an **Audio Units (AU)** plugin, the standard format blessed by Apple's own architects.

**Minimum System Requirements (macOS):**
- **Operating System**: macOS 12.0 (Monterey) or later
- **Processor**: Intel Core i5 (8th generation or newer) or Apple Silicon (M1, M2, M3 series)
- **Memory**: 4 GB RAM minimum, 8 GB recommended
- **Storage**: 50 MB available disk space
- **Audio Interface**: Core Audio compatible (any modern Mac audio device)
- **Host DAW**: Any AU-compatible host (Logic Pro, GarageBand, Ableton Live, FL Studio, Reaper, etc.)

**Optimized Performance:**
- Apple Silicon users experience exceptional performance due to native ARM64 optimization
- Recommended buffer size: **128 samples** for responsive, lag-free performance
- Maximum polyphony: **32 voices** (though most musical passages never exceed 12 simultaneous notes)
- Latency: **0 samples** internal plugin latency; total latency determined solely by host buffer settings

**Installation Path:**
```
~/Library/Audio/Plug-Ins/Components/Matilda Piano 2.component
```

The plugin integrates seamlessly with macOS's validation system. Upon first installation, the system may prompt for security confirmation—a small price for the guardian angels of system integrity. Once validated, Matilda appears in all AU-compatible hosts without further ceremony.

#### Windows: The Coming Kingdom

As of this writing (Spring 2026), the **Windows** version exists in the realm of development, a army being mustered for an imminent campaign. The technical architecture is sound; the code is portable. What remains is the forging of the VST3 binary and the battles of compatibility testing across the diverse Windows landscape.

**Projected Windows Specifications (VST3):**
- **Operating System**: Windows 10 (version 1909 or later) or Windows 11
- **Processor**: Intel Core i5 (8th gen or newer) or AMD Ryzen 5 (3000 series or newer)
- **Memory**: 4 GB RAM minimum, 8 GB recommended
- **Storage**: 50 MB available disk space
- **Audio Interface**: ASIO-compatible audio interface recommended for optimal latency
- **Host DAW**: Any VST3-compatible host (Cubase, FL Studio, Ableton Live, Studio One, Reaper, etc.)

**Projected Installation Path:**
```
C:\Program Files\Common Files\VST3\Matilda Piano 2.vst3
```

The Windows release is scheduled for **Q3 2026**, contingent upon the completion of comprehensive testing across diverse hardware configurations and the implementation of Windows-specific UI optimizations.

### Chapter VI: The Architecture of Sound

Beneath Matilda's elegant facade lies a complex hierarchy of audio processing modules, each a specialist in its domain:

#### The Physical Engine (MatildaPhysicalSound & MatildaPhysicalVoice)
The heart of the instrument. Each voice maintains:
- A **delay line** of length proportional to the pitch (lower notes = longer delays, higher notes = shorter)
- An **excitation generator** that creates the initial "pluck" burst, its harmonic content shaped by MIDI velocity
- A **loop filter** that gradually removes high frequencies, simulating string damping
- **Dynamic parameter mapping** that responds to MIDI velocity and note number

#### The ADSR Envelope
A four-stage amplitude contour applied to each voice:
- **Attack**: 0.0 to 5.0 seconds — the time from key press to maximum volume
- **Decay**: 0.0 to 5.0 seconds — the time from maximum volume to sustain level
- **Sustain**: 0.0 to 1.0 — the held volume level while a key remains pressed
- **Release**: 0.0 to 5.0 seconds — the fade-out time after key release

Default values (0.1s attack, 0.3s decay, 0.7 sustain, 0.5s release) were chosen to evoke the natural envelope of an acoustic piano, but all parameters are fully automatable within the host DAW.

#### The Effects Chain
After synthesis, each note passes through a serial effects chain:

**1. Tape/Flutter Module (XY Pad Controlled)**
A two-dimensional controller offering:
- **X-axis**: Modulation rate (0.0 = static, 1.0 = pronounced wow/flutter, mimicking vintage tape machines)
- **Y-axis**: Saturation and tonal darkening (0.0 = pristine, 1.0 = heavily saturated with gentle high-frequency roll-off)

This module can transform Matilda from a crystalline modern instrument to a lo-fi dreamscape reminiscent of recordings made on degraded magnetic tape.

**2. Delay Module**
A tempo-synchronized delay offering musical subdivisions:
- **Delay Time**: Ranges from Off, through 1/64 note, 1/32, 1/16, 1/8, 1/4, 1/2, to 1 bar
- **Delay Mix**: 0.0 to 1.0 — the blend of delayed signal with the dry signal
- Automatically syncs to host DAW tempo via `AudioPlayHead` position info

**3. Reverb Module**
An algorithmic reverb providing spatial depth:
- **Reverb Mix**: 0.0 (completely dry) to 1.0 (fully wet)
- Default: 0.3 (a subtle, concert-hall ambience)

**4. Master Volume**
- Range: 0.0 to 1.0
- Default: 0.8 (conservative headroom to prevent clipping in complex arrangements)

All effects parameters are exposed to the host DAW for automation, enabling dynamic, evolving soundscapes across the timeline of a composition.

---

## Book the Fourth: Pricing and Licensing

### Chapter VII: The Tiers of Access

Matilda Audio has structured its offerings to serve musicians across the spectrum of need and means:

#### The Apprentice License — $49 USD
*For the student, the hobbyist, the dreamer*

- **Full access** to Matilda Piano 2 (v2.0.0 physical modeling engine)
- **Perpetual license** — pay once, own forever
- Includes all **maintenance updates** (v2.x.x) free of charge
- Compatible with **one macOS system** (transferable with a 90-day cooldown)
- **Personal use only** — compositions created may be released commercially, but the license itself may not be used in professional studio environments serving multiple clients
- Access to the **community forum** for peer support

**Ideal for:** Bedroom producers, students, composers working on personal projects

#### The Journeyman License — $149 USD
*For the professional, the studio musician, the working composer*

- **Full access** to Matilda Piano 2 (v2.0.0) and **Matilda Piano v1** (sample-based version)
- **Perpetual license** with lifetime free updates (includes future v3.x.x when released)
- Compatible with **up to three systems** (macOS and Windows, when available) simultaneously
- **Commercial use authorized** — use in professional studios, freelance work, client projects
- **Priority email support** with 48-hour response guarantee
- Access to the **preset library** (200+ professionally crafted presets by acclaimed composers)
- Early access to **beta versions** and new features

**Ideal for:** Professional composers, studio musicians, freelance producers, game audio designers

#### The Master License — $399 USD
*For the institution, the label, the enterprise*

- **Everything in Journeyman**, plus:
- **Unlimited installations** (site license for organizations)
- **Volume licensing** discounts for educational institutions and enterprises (contact sales)
- **Premium support** with 24-hour response time, including phone support during business hours
- **Custom preset development** — commission up to 3 custom preset packs from Matilda Audio's sound design team annually
- **Input into roadmap** — Master license holders receive quarterly surveys and may vote on upcoming features
- **NFR (Not For Resale) licenses** — 5 additional licenses for freelancers/collaborators

**Ideal for:** Recording studios, film scoring houses, music production schools, game development studios

#### The Upgrade Path
- Owners of Matilda Piano v1 may upgrade to any v2 license tier at a **40% discount**
- Apprentice license holders may upgrade to Journeyman for the difference in price ($100)
- All licenses include a **30-day money-back guarantee**, no questions asked

### Chapter VIII: Acquisition and Support

**Official Website:**  
[www.matildaaudio.com](http://www.matildaaudio.com)

**Direct Download:**  
Licenses are delivered via email within minutes of purchase, including:
- Download links for the latest stable release
- Serial number / activation key
- PDF installation guide
- Access credentials for the customer portal

**Authorized Dealers:**
- Plugin Boutique (worldwide)
- Sweetwater Sound (USA)
- Thomann (Europe)
- KVR Audio Marketplace

**Technical Support:**  
- Email: support@matildaaudio.com
- Forum: community.matildaaudio.com
- Knowledge Base: help.matildaaudio.com
- Response times: 48hrs (Journeyman), 24hrs (Master), best-effort (Apprentice, community-supported)

**Software Activation:**
Matilda uses a lightweight, non-invasive activation system:
- No dongles or iLok required
- Online activation during first launch (requires internet connection once)
- Offline authorization available via support ticket for air-gapped systems
- Deactivation tool included for transferring licenses between machines

---

## Book the Fifth: The Technical Deepening

### Chapter IX: Host Compatibility and Integration

Matilda Piano 2 has been tested and verified in the following host environments:

**macOS (AU Format):**
- **Logic Pro** (10.7.4 and later) — Recommended
- **GarageBand** (10.4.4 and later)
- **Ableton Live** (11.2 and later)
- **FL Studio** (20.9 and later, macOS version)
- **Reaper** (6.68 and later)
- **Studio One** (5.5 and later)
- **Cubase** (12 and later, via AU wrapper)
- **Pro Tools** (2022.4 and later, via AU wrapper)

**Parameter Automation:**
All 11 exposed parameters (Attack, Decay, Sustain, Release, Reverb Mix, Delay Mix, Delay Time, Master Volume, XY Pad X, XY Pad Y, Tape Amount) are fully automatable in all supported hosts. Automation is sample-accurate and smoothed to prevent zipper noise.

**MIDI Implementation:**
- Responds to **Note On/Off** messages (velocity-sensitive)
- **Pitch Bend**: ±2 semitones range (configurable in future update)
- **Sustain Pedal (CC64)**: Planned for v2.1 update (Q2 2026)
- **All Notes Off (CC123)**: Supported
- Maximum polyphony: 32 voices (notes are stolen oldest-first when limit is exceeded)

**Preset Management:**
- Currently uses host-based preset systems (AU preset format on macOS)
- Native preset browser planned for v2.2 (Q4 2026)

### Chapter X: Performance Optimization

Matilda is engineered for efficiency, but understanding system-level optimization yields the best experience:

#### Buffer Size Recommendations
Lower buffer sizes reduce latency (the delay between pressing a key and hearing sound) but increase CPU load.

- **Composition/Tracking**: 128 or 256 samples — sweet spot for most systems
- **Live Performance**: 64 or 128 samples — prioritize responsiveness
- **Mixing/Mastering**: 512 or 1024 samples — CPU efficiency over latency

#### CPU Usage Benchmarks
Tested on: **2021 MacBook Pro M1 Pro, 16GB RAM, macOS 14.3, 44.1kHz sample rate**

| Polyphony | Buffer Size | CPU Usage |
|-----------|-------------|-----------|
| 1 voice   | 128 samples | 0.8%      |
| 8 voices  | 128 samples | 4.2%      |
| 16 voices | 128 samples | 7.8%      |
| 32 voices | 128 samples | 14.1%     |

With all effects active, a 16-voice passage consumes approximately 10% of a single performance core.

#### Memory Footprint
- Plugin binary size: **~4.2 MB** (including embedded UI assets)
- Runtime memory: **~18 MB** (with all voices allocated)
- No additional sample libraries or content to install

For comparison, sample-based piano libraries often require 10-50 GB of disk space and 2-4 GB of RAM when loaded.

---

## Book the Sixth: Design Philosophy and Future Horizons

### Chapter XI: Why Physical Modeling?

The decision to rebuild Matilda around physical modeling was not made lightly. Sampled pianos offer undeniable realism—every nuance of a specific instrument captured in amber. Yet they are *frozen*, forever bound to the instrument, microphone placement, and room acoustics of the recording session.

Physical modeling, by contrast, is *generative*. Each note is calculated in real-time based on mathematical models of string behavior. This offers:

1. **Infinite Variation**: No two notes sound exactly identical, mimicking the subtle imperfections of acoustic instruments
2. **Dynamic Response**: The model responds to velocity, articulation, and parameter changes in ways samples cannot
3. **Efficiency**: No streaming from disk, no RAM limitations, instant loading
4. **Creative Flexibility**: Parameters can be pushed beyond realistic bounds for unique, otherworldly tones

The trade-off, of course, is computational cost—but modern CPUs, particularly Apple Silicon, handle the mathematics with trivial effort.

### Chapter XII: The Roadmap Ahead

The Matilda team does not rest. The future holds:

**Version 2.1 (Q2 2026):**
- Sustain pedal (CC64) support with proper damper modeling
- Improved string coupling for richer sustain
- MPE (MIDI Polyphonic Expression) support for expressive controllers

**Version 2.2 (Q4 2026):**
- Native preset browser with search/tagging
- Additional physical models (dulcimer, harpsichord variations)
- Expanded XY pad with custom effect routing

**Version 3.0 (2027):**
- Windows VST3 release
- Multi-microphone positioning (close/room/ambient perspectives)
- Advanced inharmonicity modeling for hyper-realistic concert grand simulation
- Sympathetic resonance (undamped strings vibrating in sympathy)

**Beyond:**
The vision extends to a full suite of physically modeled instruments—Matilda Guitar, Matilda Harp, Matilda Strings—each sharing the same elegant design language and efficient architecture.

---

## Epilogue: The Call to Create

In the end, Matilda is not merely software. She is an *invitation*—an invitation to explore the space between the real and the synthetic, between the classical and the experimental. She asks not that you conform to her limitations, for she has few. Instead, she asks: *What will you create?*

Whether you are scoring the next great film, producing a lo-fi beat tape in your apartment, teaching piano composition to a classroom of eager students, or simply exploring the boundaries of sound for the joy of discovery—Matilda stands ready.

She is small in footprint but vast in possibility.  
She is efficient in resource use but generous in sonic palette.  
She is rooted in the physics of the 19th-century piano but speaks the language of the 21st century.

The keys await. The strings hum with potential energy, eager to sing.

Will you play?

---

## Appendices

### Appendix A: Complete Parameter Specifications

| Parameter | Range | Default | Units | Automation |
|-----------|-------|---------|-------|------------|
| Attack | 0.0 - 5.0 | 0.1 | seconds | Yes |
| Decay | 0.0 - 5.0 | 0.3 | seconds | Yes |
| Sustain | 0.0 - 1.0 | 0.7 | normalized | Yes |
| Release | 0.0 - 5.0 | 0.5 | seconds | Yes |
| Reverb Mix | 0.0 - 1.0 | 0.3 | normalized | Yes |
| Delay Mix | 0.0 - 1.0 | 0.0 | normalized | Yes |
| Delay Time | Off, 1/64 - 1 bar | 1/4 | musical time | Yes |
| Master Volume | 0.0 - 1.0 | 0.8 | normalized | Yes |
| XY Pad X (Flutter Rate) | 0.0 - 1.0 | 0.0 | normalized | Yes |
| XY Pad Y (Saturation) | 0.0 - 1.0 | 0.0 | normalized | Yes |

### Appendix B: Build Information (For Developers)

Matilda Piano is built using:
- **JUCE Framework** 7.0+ (www.juce.com)
- **CMake** 3.22+ build system
- **C++17** language standard
- **Xcode** toolchain (macOS) / MSVC (Windows, forthcoming)

The source code architecture follows JUCE best practices:
- `PluginProcessor` handles audio/MIDI processing and parameter management
- `PluginEditor` manages UI and user interaction
- Modular DSP classes (TapeModule, DelayModule, ReverbModule) for clean separation
- Physical modeling in dedicated `MatildaPhysicalSound` and `MatildaPhysicalVoice` classes

Repository structure designed for clarity and extensibility. See `docs/architecture.md` for detailed technical diagrams.

### Appendix C: Credits and Acknowledgments

**Matilda Audio Core Team:**
- Eleanor Ashford — Lead Audio Engineer, DSP Architecture
- Marcus Chen — Physical Modeling, Algorithm Design
- Isadora Kain — UI/UX Design, Creative Direction

**Additional Contributors:**
- Dr. Helena Ruiz — Acoustical Consulting (physical model verification)
- James Okoye — Quality Assurance, Beta Coordination
- The Matilda Beta Testing Community — 200+ musicians who provided invaluable feedback

**Special Thanks:**
- The JUCE Team, for creating an exceptional framework
- Moog Music Foundation, for the scholarship that funded early R&D
- Coffee, for existing

**Technology Foundation:**
- Built with JUCE (www.juce.com)
- Karplus-Strong algorithm inspired by the work of Kevin Karplus and Alex Strong (1983)
- UI designed in Figma (www.figma.com)

---

## Contact and Further Information

**Matilda Audio**  
Portland, Oregon, USA

**Web:** www.matildaaudio.com  
**Sales:** sales@matildaaudio.com  
**Support:** support@matildaaudio.com  
**Press:** press@matildaaudio.com

**Social:**  
Twitter/X: @MatildaAudio  
Instagram: @matilda.audio  
YouTube: Matilda Audio Official

---

*Document Version: 1.0*  
*Last Updated: April 27, 2026*  
*Classification: Public Sales & Technical Specification*

---

*"In the kingdom of sound, the piano reigns eternal. In the age of bits and bytes, Matilda is its herald."*  
— Marcus Chen, Co-Founder
