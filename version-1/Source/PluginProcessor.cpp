#include "PluginProcessor.h"
#include "PluginEditor.h"

// Set to 1 to bypass Tape/Delay/Reverb (synth -> master only). Use to isolate "no sound" when testing.
#ifndef MATILDA_BYPASS_DSP_DEBUG
#define MATILDA_BYPASS_DSP_DEBUG 0
#endif

MatildaPianoAudioProcessor::MatildaPianoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
    , valueTreeState(*this, nullptr, "PARAMETERS", Parameters::createParameterLayout())
{
    // Add voices to synthesiser
    for (int i = 0; i < numVoices; ++i)
    {
        synth.addVoice(new MatildaSamplerVoice());
    }
    
    // Load samples (will be implemented to load from Samples/ directory)
    loadSamples();
}

MatildaPianoAudioProcessor::~MatildaPianoAudioProcessor()
{
}

const juce::String MatildaPianoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MatildaPianoAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MatildaPianoAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MatildaPianoAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MatildaPianoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MatildaPianoAudioProcessor::getNumPrograms()
{
    return 1;
}

int MatildaPianoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MatildaPianoAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String MatildaPianoAudioProcessor::getProgramName(int index)
{
    return {};
}

void MatildaPianoAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void MatildaPianoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    // Prepare synthesiser
    synth.setCurrentPlaybackSampleRate(sampleRate);
    // Set ADSR sample rate on our voices so envelope timing is correct (was causing sharp burst then silence)
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<MatildaSamplerVoice*>(synth.getVoice(i)))
            v->setSampleRate(sampleRate);
    }

    // Prepare DSP modules
    tapeModule.prepare(spec);
    delayModule.prepare(spec);
    delayModule.reset();
    reverbModule.prepare(spec);
    masterGain.prepare(spec);
    
    // Set initial gain
    masterGain.setGainLinear(Parameters::MASTER_VOL_DEFAULT);
}

void MatildaPianoAudioProcessor::releaseResources()
{
    // Do NOT clear synth sounds here — the host/standalone may call this when
    // reconfiguring audio (e.g. opening device settings), which would remove
    // all loaded samples and cause "meter moves but no sound". Samples are
    // only cleared in loadSamples() when reloading.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MatildaPianoAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
        
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void MatildaPianoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update parameters
    updateParameters();

    // Inject on-screen / laptop keyboard state into MIDI (poll state so we don't rely on processNextMidiBuffer timing)
    const int midiChannel = 1;
    for (int note = 0; note < 128; ++note)
    {
        const bool nowOn = keyboardState.isNoteOn(midiChannel, note);
        if (nowOn != keyWasDown[note])
        {
            keyWasDown[note] = nowOn;
            if (nowOn)
                midiMessages.addEvent(juce::MidiMessage::noteOn(midiChannel, note, (juce::uint8)100), 0);
            else
                midiMessages.addEvent(juce::MidiMessage::noteOff(midiChannel, note), 0);
        }
    }

    // Process MIDI and render synthesiser
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Polyphony gain: Synthesiser sums all voices; many notes → clip → burst then flat "blank" sound.
    // Use 1/numVoices so 32 voices peak at 1.0 (no clamp needed). Single note = 1/32; master gain
    // is scaled in updateParameters() so the 0–1 knob gives audible level (see MASTER_MAKEUP).
    const float polyphonyGain = 1.0f / static_cast<float>(numVoices);
    buffer.applyGain(polyphonyGain);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = juce::jlimit(-1.0f, 1.0f, data[i]);
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

#if MATILDA_BYPASS_DSP_DEBUG
    // Bypass Tape, Delay, Reverb — synth -> master only (for "no sound" debugging; set MATILDA_BYPASS_DSP_DEBUG to 0 to restore full chain)
    masterGain.process(context);
#else
    // Full DSP chain: Tape (XY) -> Delay -> Reverb -> Master Gain
    tapeModule.process(block);
    delayModule.process(block);
    reverbModule.process(block);
    masterGain.process(context);
#endif
    // Final safety clamp so master make-up never sends > 1.0 to the host (avoids burst/blank when many keys held)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = juce::jlimit(-1.0f, 1.0f, data[i]);
    }
}

bool MatildaPianoAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MatildaPianoAudioProcessor::createEditor()
{
    return new MatildaPianoAudioProcessorEditor(*this);
}

void MatildaPianoAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MatildaPianoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(valueTreeState.state.getType()))
        {
            valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

void MatildaPianoAudioProcessor::loadSamples()
{
    // Clear existing sounds
    synth.clearSounds();
    sampleLoadStatus_.clear();

    // Search order:
    // 1) keySamples — bundled (Contents/Resources/keySamples) or next to the .app
    //    Naming: note + octave 0–7, e.g. c0.wav, c#5.wav (c0 = C1 = MIDI 24; c7 = C8 = MIDI 108). PRD: 7 octaves.
    // 2) ~/Music/MatildaPiano/Samples
    // 3) ~/Documents/MatildaPiano/Samples
    //    Naming: note name (e.g. Piano_C4.wav) or MIDI number (e.g. Piano_60.wav)
    juce::File samplesDir;
    bool useKeySamplesNaming = false;

    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    juce::File keySamplesInBundle = appFile.getChildFile("Contents/Resources/keySamples");
    juce::File keySamplesNextToApp = appFile.getParentDirectory().getChildFile("keySamples");
    if (keySamplesInBundle.isDirectory())
        samplesDir = keySamplesInBundle;
    else if (keySamplesNextToApp.isDirectory())
        samplesDir = keySamplesNextToApp;
    if (samplesDir.exists())
        useKeySamplesNaming = true;

    if (!samplesDir.isDirectory())
    {
        samplesDir = juce::File::getSpecialLocation(juce::File::userMusicDirectory)
                         .getChildFile("MatildaPiano")
                         .getChildFile("Samples");
        if (!samplesDir.isDirectory())
        {
            samplesDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                             .getChildFile("MatildaPiano")
                             .getChildFile("Samples");
        }
    }

    if (!samplesDir.isDirectory())
    {
        sampleLoadStatus_ = "No samples found — add keySamples folder or WAV/AIFF to ~/Music/MatildaPiano/Samples or ~/Documents/MatildaPiano/Samples";
        return;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Parser for keySamples naming: lowercase note + optional # + octave 0–7 (e.g. c0, c#5).
    // Octave 0 = C1 = MIDI 24; octave 7 = C8 = MIDI 108. PRD: 7 octaves (C1–C8).
    auto keySamplesStemToMidi = [](const juce::String& stem) -> int
    {
        if (stem.isEmpty()) return -1;
        juce::String s = stem.toLowerCase().trim();
        int i = 0;
        auto letter = s[0];
        int base = -1;
        switch (letter)
        {
            case 'c': base = 0; break;
            case 'd': base = 2; break;
            case 'e': base = 4; break;
            case 'f': base = 5; break;
            case 'g': base = 7; break;
            case 'a': base = 9; break;
            case 'b': base = 11; break;
            default: return -1;
        }
        i = 1;
        if (i < s.length() && s[i] == '#') { base += 1; i++; }
        if (i >= s.length() || !juce::CharacterFunctions::isDigit(s[i])) return -1;
        int octave = 0;
        while (i < s.length() && juce::CharacterFunctions::isDigit(s[i]))
        {
            octave = octave * 10 + (s[i] - '0');
            i++;
        }
        if (octave < 0 || octave > 7) return -1;
        int midi = 24 + octave * 12 + base;
        return (midi >= 0 && midi <= 127) ? midi : -1;
    };

    auto noteNameToMidi = [](juce::String noteName) -> int
    {
        noteName = noteName.toUpperCase().retainCharacters("ABCDEFG#B0123456789-");
        if (noteName.isEmpty())
            return -1;

        const juce::String letters = "CDEFGAB";
        auto letter = noteName[0];
        int base = -1;
        switch (letter)
        {
            case 'C': base = 0; break;
            case 'D': base = 2; break;
            case 'E': base = 4; break;
            case 'F': base = 5; break;
            case 'G': base = 7; break;
            case 'A': base = 9; break;
            case 'B': base = 11; break;
            default: return -1;
        }

        int idx = 1;
        int accidental = 0;
        if (idx < noteName.length() && (noteName[idx] == '#' || noteName[idx] == 'B'))
        {
            accidental = (noteName[idx] == '#') ? 1 : -1;
            ++idx;
        }

        auto octaveStr = noteName.substring(idx).trim();
        if (octaveStr.isEmpty() || !octaveStr.containsOnly("0123456789-"))
            return -1;

        const int octave = octaveStr.getIntValue();
        const int midi = (octave + 1) * 12 + base + accidental; // MIDI 60 = C4
        return (midi >= 0 && midi <= 127) ? midi : -1;
    };

    auto parseMidiNoteFromName = [&](const juce::String& fileStem) -> int
    {
        // 1) Look for note names like C4, F#3, Bb2 (we treat 'b' as 'B' in uppercase pass above)
        for (int i = 0; i < fileStem.length() - 1; ++i)
        {
            auto c = juce::CharacterFunctions::toUpperCase(fileStem[i]);
            if (c < 'A' || c > 'G')
                continue;

            // Build candidate: letter + optional #/b + octave (at least 1 digit, maybe -1)
            juce::String cand;
            cand << c;

            int j = i + 1;
            if (j < fileStem.length())
            {
                auto acc = fileStem[j];
                if (acc == '#' || acc == 'b' || acc == 'B')
                {
                    cand << acc;
                    ++j;
                }
            }

            if (j >= fileStem.length())
                continue;

            // Octave: optional '-' then digits
            int k = j;
            if (fileStem[k] == '-')
                ++k;

            int digitStart = k;
            while (k < fileStem.length() && juce::CharacterFunctions::isDigit(fileStem[k]))
                ++k;

            if (k == digitStart)
                continue;

            cand << fileStem.substring(j, k);
            if (auto midi = noteNameToMidi(cand); midi != -1)
                return midi;
        }

        // 2) Look for a MIDI note number token 0..127
        for (int i = 0; i < fileStem.length(); ++i)
        {
            if (!juce::CharacterFunctions::isDigit(fileStem[i]))
                continue;

            int j = i;
            while (j < fileStem.length() && juce::CharacterFunctions::isDigit(fileStem[j]))
                ++j;

            auto token = fileStem.substring(i, j);
            const int midi = token.getIntValue();
            if (midi >= 0 && midi <= 127)
                return midi;

            i = j;
        }

        return -1;
    };

    juce::Array<juce::File> files;
    samplesDir.findChildFiles(files, juce::File::findFiles, true, "*.wav;*.wave;*.aif;*.aiff");
    if (files.isEmpty())
    {
        sampleLoadStatus_ = "No samples found — add WAV/AIFF to " + samplesDir.getFullPathName();
        return;
    }

    for (const auto& f : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(f));
        if (!reader)
            continue;

        auto fileStem = f.getFileNameWithoutExtension();
        int midi = -1;
        if (useKeySamplesNaming)
            midi = keySamplesStemToMidi(fileStem);
        if (midi == -1)
            midi = parseMidiNoteFromName(fileStem);

        juce::BigInteger notes;
        if (midi != -1)
            notes.setBit(midi);
        else
            notes.setRange(0, 128, true); // fallback

        // Small attack (3 ms) to avoid clicks/glitches on note start (e.g. F3/G3 transients)
        const double sampleAttackSecs = 0.003;
        auto sound = std::make_unique<MatildaSamplerSound>(
            f.getFileNameWithoutExtension(),
            *reader,
            notes,
            midi != -1 ? midi : 60,
            sampleAttackSecs,
            0.1,
            30.0
        );

        synth.addSound(sound.release());
    }
    // Clear status when at least one sound was loaded
    sampleLoadStatus_.clear();
}

void MatildaPianoAudioProcessor::updateParameters()
{
    // Update ADSR for all voices
    float attack = valueTreeState.getRawParameterValue(Parameters::ATTACK)->load();
    float decay = valueTreeState.getRawParameterValue(Parameters::DECAY)->load();
    float sustain = valueTreeState.getRawParameterValue(Parameters::SUSTAIN)->load();
    float release = valueTreeState.getRawParameterValue(Parameters::RELEASE)->load();
    
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<MatildaSamplerVoice*>(synth.getVoice(i)))
        {
            voice->setAttack(attack);
            voice->setDecay(decay);
            voice->setSustain(sustain);
            voice->setRelease(release);
        }
    }
    
    // Update tape module (XY pad)
    float xyX = valueTreeState.getRawParameterValue(Parameters::XY_X)->load();
    float xyY = valueTreeState.getRawParameterValue(Parameters::XY_Y)->load();
    
    tapeModule.setWowFlutterRate(xyX);
    tapeModule.setSaturation(xyY);
    tapeModule.setToneCutoff(1.0f - xyY * 0.5f); // Darker as Y increases
    
    // Update delay module — lowest knob position = Off (mix 0), then 1/64..1
    float delayKnob = valueTreeState.getRawParameterValue(Parameters::DELAY_TIME)->load();
    const float delayOffThreshold = 0.05f;
    if (delayKnob <= delayOffThreshold)
    {
        delayModule.setMix(0.0f);
        delayModule.setDelayTime(0.0f);
    }
    else
    {
        float t = (delayKnob - delayOffThreshold) / (1.0f - delayOffThreshold);
        delayModule.setDelayTime(t);
        // Higher mix so delay is clearly audible (0.4–0.8 range when on)
        delayModule.setMix(juce::jlimit(0.4f, 0.8f, 0.4f + t * 0.4f));
    }
    
    // Try to get host tempo if available
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (auto bpm = positionInfo->getBpm())
            {
                if (*bpm > 0.0)
                    delayModule.setHostTempo(*bpm);
            }
        }
    }
    
    // Update reverb module. Base reverb from knob; XY pad adds "wash" (watery, washed-out vibe)
    float baseReverb = valueTreeState.getRawParameterValue(Parameters::REVERB)->load();
    float xyWash = xyY * 0.5f + xyX * 0.3f;  // Y = main wash, X = secondary
    float reverbMix = juce::jlimit(0.0f, 1.0f, baseReverb + xyWash);
    reverbModule.setMix(reverbMix);
    
    // Update master gain. Knob stays 0–1; we apply make-up so that after 1/numVoices polyphony gain
    // a single note is audible (e.g. 0.8 → ~12.8 linear so 1 note ≈ 0.4).
    const float masterMakeUp = 16.0f;
    float masterVol = valueTreeState.getRawParameterValue(Parameters::MASTER_VOL)->load();
    masterGain.setGainLinear(masterVol * masterMakeUp);
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatildaPianoAudioProcessor();
}
