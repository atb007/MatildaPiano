#pragma once

#include <array>
#include <JuceHeader.h>
#include "Parameters.h"
#include "MatildaPhysicalVoice.h"
#include "MatildaPhysicalSound.h"
#include "TapeModule.h"
#include "DelayModule.h"
#include "ReverbModule.h"

class MatildaPianoAudioProcessor : public juce::AudioProcessor
{
public:
    MatildaPianoAudioProcessor();
    ~MatildaPianoAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; }
    
    /** Registers the physical-model sound (v2 — no WAV samples). */
    void setupPhysicalEngine();
    juce::Synthesiser& getSynth() { return synth; }

    /** Shared keyboard state for the on-screen MidiKeyboardComponent; processor injects it into MIDI in processBlock. */
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }
    const juce::MidiKeyboardState& getKeyboardState() const { return keyboardState; }

    /** Optional status line for UI (v2: usually empty). Safe to read from message thread. */
    juce::String getSampleLoadStatus() const { return sampleLoadStatus_; }

private:
    juce::AudioProcessorValueTreeState valueTreeState;
    juce::MidiKeyboardState keyboardState;
    juce::Synthesiser synth;
    static constexpr int numVoices = 32;
    
    TapeModule tapeModule;
    DelayModule delayModule;
    ReverbModule reverbModule;
    
    juce::dsp::Gain<float> masterGain;
    
    double currentSampleRate = 44100.0;

    juce::String sampleLoadStatus_;
    std::array<bool, 128> keyWasDown = {};

    void updateParameters();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaPianoAudioProcessor)
};
