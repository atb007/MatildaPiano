#include "MatildaSamplerVoice.h"

MatildaSamplerVoice::MatildaSamplerVoice()
{
    adsrParams.attack = 0.1f;
    adsrParams.decay = 0.3f;
    adsrParams.sustain = 0.7f;
    adsrParams.release = 0.5f;
    adsr.setParameters(adsrParams);
}

bool MatildaSamplerVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MatildaSamplerSound*>(sound) != nullptr;
}

void MatildaSamplerVoice::startNote(int midiNoteNumber, float velocity,
                                     juce::SynthesiserSound* sound,
                                     int currentPitchWheelPosition)
{
    if (auto* samplerSound = dynamic_cast<MatildaSamplerSound*>(sound))
    {
        currentVelocity = velocity;
        isNoteOn = true;
        
        // Start the base sampler voice
        juce::SamplerVoice::startNote(midiNoteNumber, velocity, sound, currentPitchWheelPosition);
        
        // Start ADSR envelope
        adsr.noteOn();
    }
}

void MatildaSamplerVoice::stopNote(float velocity, bool allowTailOff)
{
    if (allowTailOff && isNoteOn)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        clearCurrentNote();
        isNoteOn = false;
    }
    
    juce::SamplerVoice::stopNote(velocity, allowTailOff);
}

void MatildaSamplerVoice::pitchWheelMoved(int newPitchWheelValue)
{
    juce::SamplerVoice::pitchWheelMoved(newPitchWheelValue);
}

void MatildaSamplerVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    juce::SamplerVoice::controllerMoved(controllerNumber, newControllerValue);
}

void MatildaSamplerVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                         int startSample, int numSamples)
{
    if (auto* playingSound = dynamic_cast<MatildaSamplerSound*>(getCurrentlyPlayingSound().get()))
    {
        // Render the sampler output
        juce::SamplerVoice::renderNextBlock(outputBuffer, startSample, numSamples);
        
        // Apply ADSR envelope
        if (adsr.isActive())
        {
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                auto* channelData = outputBuffer.getWritePointer(channel, startSample);
                
                for (int sample = 0; sample < numSamples; ++sample)
                {
                    float envelopeValue = adsr.getNextSample();
                    channelData[sample] *= envelopeValue;
                }
            }
        }
    }
}

void MatildaSamplerVoice::setAttack(float attackSeconds)
{
    adsrParams.attack = attackSeconds;
    updateADSRParameters();
}

void MatildaSamplerVoice::setDecay(float decaySeconds)
{
    adsrParams.decay = decaySeconds;
    updateADSRParameters();
}

void MatildaSamplerVoice::setSustain(float sustainLevel)
{
    adsrParams.sustain = juce::jlimit(0.0f, 1.0f, sustainLevel);
    updateADSRParameters();
}

void MatildaSamplerVoice::setRelease(float releaseSeconds)
{
    adsrParams.release = releaseSeconds;
    updateADSRParameters();
}

void MatildaSamplerVoice::setSampleRate(double sampleRate)
{
    adsr.setSampleRate(sampleRate);
    updateADSRParameters();
}

void MatildaSamplerVoice::updateADSRParameters()
{
    adsr.setParameters(adsrParams);
}
