#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "MatildaNeuralVoice.h"

// Set to 1 to bypass Tape/Delay/Reverb (synth -> master only). Use to isolate "no sound" when testing.
#ifndef MATILDA_BYPASS_DSP_DEBUG
#define MATILDA_BYPASS_DSP_DEBUG 0
#endif

namespace
{
void sanitizeBuffer(juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (!std::isfinite(data[i]))
                data[i] = 0.0f;
            else
                data[i] = std::tanh(data[i]);
        }
    }
}
} // namespace

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
            synth.addVoice(new MatildaNeuralVoice(neuralModel.get(), &inferenceScheduler));
        
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
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<MatildaNeuralVoice*>(synth.getVoice(i)))
            voice->setSampleRate(sampleRate);
    }

    // Prepare DSP modules
    tapeModule.prepare(spec);
    tapeModule.reset();
    delayModule.prepare(spec);
    delayModule.reset();
    reverbModule.prepare(spec);
    reverbModule.reset();
    masterGain.prepare(spec);
    masterGain.setGainLinear(1.0f);

    smoothedSynthBusGain = Parameters::MASTER_VOL_DEFAULT * masterMakeUp
                         / static_cast<float>(numVoices);
    hostWasPlaying = false;
    inferenceScheduler.start();
}

void MatildaPianoAudioProcessor::releaseResources()
{
    inferenceScheduler.stop();
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

    if (!inferenceScheduler.isRunning())
        inferenceScheduler.start();

    bool hostIsPlaying = false;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
            hostIsPlaying = position->getIsPlaying();
    }

    if (hostIsPlaying && !hostWasPlaying)
    {
        synth.allNotesOff(0, true);
        tapeModule.reset();
        delayModule.reset();
        reverbModule.reset();
    }
    else if (!hostIsPlaying && hostWasPlaying)
    {
        synth.allNotesOff(0, true);
    }

    hostWasPlaying = hostIsPlaying;

    // Host MIDI at concert pitch; on-screen keyboard gets +12 (UI label offset from UI-UPDATES.md).
    juce::MidiBuffer synthMidi;
    synthMidi.addEvents(midiMessages, 0, -1, 0);

    const int midiChannel = 1;
    for (int note = 0; note < 128; ++note)
    {
        const bool nowOn = keyboardState.isNoteOn(midiChannel, note);
        if (nowOn != keyWasDown[note])
        {
            keyWasDown[note] = nowOn;
            const int transposedNote = note + 12;
            if (transposedNote > 127)
                continue;
            if (nowOn)
                synthMidi.addEvent(juce::MidiMessage::noteOn(midiChannel, transposedNote, (juce::uint8)100), 0);
            else
                synthMidi.addEvent(juce::MidiMessage::noteOff(midiChannel, transposedNote), 0);
        }
    }

    int incomingNoteOnEvents = 0;
    for (const auto metadata : synthMidi)
    {
        if (metadata.getMessage().isNoteOn())
            ++incomingNoteOnEvents;
    }
    inferenceScheduler.beginAudioBlock(incomingNoteOnEvents);

    synth.renderNextBlock(buffer, synthMidi, 0, buffer.getNumSamples());

    int activeVoices = 0;
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (synth.getVoice(i)->isVoiceActive())
            ++activeVoices;
    }

    const float masterVol = valueTreeState.getRawParameterValue(Parameters::MASTER_VOL)->load();
    const float activeVoiceCount = static_cast<float>(juce::jmax(1, activeVoices));
    const float targetSynthBusGain = masterVol * masterMakeUp
                                   / (static_cast<float>(numVoices) * activeVoiceCount);

    // Fast drop when polyphony rises (prevents clip screech); slow rise when voices release
    // (prevents FX tail amplification at clip end / transport edges).
    if (targetSynthBusGain < smoothedSynthBusGain)
        smoothedSynthBusGain = targetSynthBusGain;
    else
        smoothedSynthBusGain += (targetSynthBusGain - smoothedSynthBusGain) * 0.02f;

    buffer.applyGain(smoothedSynthBusGain);

    sanitizeBuffer(buffer);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

#if MATILDA_BYPASS_DSP_DEBUG
    masterGain.process(context);
#else
    tapeModule.process(block);
    delayModule.process(block);
    reverbModule.process(block);
    masterGain.process(context);
#endif

    sanitizeBuffer(buffer);
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
                if (*bpm > 0.0 && std::abs(*bpm - lastHostTempo) > 0.1)
                {
                    lastHostTempo = *bpm;
                    delayModule.setHostTempo(*bpm);
                }
            }
        }
    }
    
    // Update reverb module. Base reverb from knob; XY pad adds "wash" (watery, washed-out vibe)
    float baseReverb = valueTreeState.getRawParameterValue(Parameters::REVERB)->load();
    float xyWash = xyY * 0.5f + xyX * 0.3f;
    float reverbMix = juce::jlimit(0.0f, 1.0f, baseReverb + xyWash);
    reverbModule.setMix(reverbMix);

    masterGain.setGainLinear(1.0f);
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MatildaPianoAudioProcessor();
}
