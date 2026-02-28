#pragma once

#include <JuceHeader.h>
#include "MatildaSamplerSound.h"

class MatildaSamplerVoice : public juce::SamplerVoice
{
public:
    MatildaSamplerVoice();
    ~MatildaSamplerVoice() override = default;
    
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    // Set ADSR parameters
    void setAttack(float attackSeconds);
    void setDecay(float decaySeconds);
    void setSustain(float sustainLevel);
    void setRelease(float releaseSeconds);

    /** Must be called (e.g. from processor prepareToPlay) so envelope timing is correct. */
    void setSampleRate(double sampleRate);

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    float currentVelocity = 0.0f;
    bool isNoteOn = false;
    
    void updateADSRParameters();
};
