#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MatildaNeuralVoice.h"

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
    // Initialize neural model first (loads ONNX model)
    try
    {
        neuralModel = std::make_unique<NeuralModel>("engineMain");
        neuralModelStatus_ = "Neural model loaded successfully";
        
        // Create voices with neural model reference
        for (int i = 0; i < numVoices; ++i)
            synth.addVoice(new MatildaNeuralVoice(neuralModel.get()));
        
        setupNeuralEngine();
    }
    catch (const std::exception& e)
    {
        neuralModelStatus_ = "ERROR: " + juce::String(e.what());
        juce::Logger::writeToLog("Failed to initialize neural engine: " + juce::String(e.what()));
    }
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
    // Note: MatildaNeuralVoice doesn't need explicit setSampleRate() call
    // (it gets sampleRate from getSampleRate() in renderNextBlock)

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
    // Do not clear synth sounds here — the host may call this when reconfiguring
    // audio; the neural engine sound is re-registered only from setupNeuralEngine().
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

    // Transpose all MIDI notes UP by one octave (+12 semitones)
    // C0 (MIDI 12) -> C1 (MIDI 24), C1 (MIDI 24) -> C2 (MIDI 36), etc.
    juce::MidiBuffer transposedMessages;
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOnOrOff())
        {
            int transposedNote = message.getNoteNumber() + 12;
            // Clamp to valid MIDI range (0-127)
            if (transposedNote >= 0 && transposedNote <= 127)
            {
                if (message.isNoteOn())
                    transposedMessages.addEvent(juce::MidiMessage::noteOn(message.getChannel(), transposedNote, message.getVelocity()), metadata.samplePosition);
                else
                    transposedMessages.addEvent(juce::MidiMessage::noteOff(message.getChannel(), transposedNote, message.getVelocity()), metadata.samplePosition);
            }
        }
        else
        {
            // Keep non-note messages as-is
            transposedMessages.addEvent(message, metadata.samplePosition);
        }
    }

    // Process MIDI and render synthesiser with transposed messages
    synth.renderNextBlock(buffer, transposedMessages, 0, buffer.getNumSamples());

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

void MatildaPianoAudioProcessor::setupNeuralEngine()
{
    synth.clearSounds();
    synth.addSound(new MatildaNeuralSound());
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
        if (auto* voice = dynamic_cast<MatildaNeuralVoice*>(synth.getVoice(i)))
        {
            voice->setADSRParameters(attack, decay, sustain, release);
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
