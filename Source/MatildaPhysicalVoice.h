#pragma once

#include <JuceHeader.h>
#include "MatildaPhysicalSound.h"

/**
 * Karplus–Strong style string loop (waveguide-inspired) for piano-like tones.
 * Output is shaped by the same ADSR as v1 so existing knob behaviour is preserved.
 */
class MatildaPhysicalVoice final : public juce::SynthesiserVoice
{
public:
    MatildaPhysicalVoice();
    ~MatildaPhysicalVoice() override = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void setAttack(float attackSeconds);
    void setDecay(float decaySeconds);
    void setSustain(float sustainLevel);
    void setRelease(float releaseSeconds);

    /** Resize delay memory for current sample rate (call from prepareToPlay). */
    void setSampleRate(double sr);

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    double sampleRate = 44100.0;
    float currentVelocity = 0.0f;
    bool isNoteOn = false;

    int currentMidiNote = 60;
    float pitchWheelRatio = 1.0f;

    std::vector<float> delayLine;
    int writeIndex = 0;

    float delayInSamples = 100.0f;
    float loopLpState = 0.0f;
    float loopLpState2 = 0.0f;  // Second pole for more realistic damping
    float dcBlockerX1 = 0.0f;
    float dcBlockerY1 = 0.0f;
    float feedbackGain = 0.998f;
    float loopLowPassCoeff = 0.35f;
    float loopLowPassCoeff2 = 0.6f;  // Second damping stage
    
    float energyTracker = 0.0f;
    int badSampleCount = 0;
    
    float inharmonicity = 0.0f;  // Stretching factor for overtones
    
    float thumpDecay = 0.0f;  // Percussive body thump (separate from string)

    int excitationSamplesLeft = 0;
    int excitationTotal = 1;
    float hammerLevel = 0.0f;
    float hammerNoiseHpState = 0.0f;  // For velocity-dependent brightness

    juce::Random rng;

    void updateADSRParameters();
    void recomputeDelayLength();
    void resetDelayLine();
    float readDelay(float delay) const;
    void writeSample(float v);
    void updateLoopTimbreForMidiNote();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaPhysicalVoice)
};
