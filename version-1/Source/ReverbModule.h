#pragma once

#include <JuceHeader.h>

class ReverbModule
{
public:
    ReverbModule();
    ~ReverbModule() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);
    void reset();
    
    void setMix(float mix); // 0.0 to 1.0
    
private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;
    
    float mix = 0.0f;
    juce::AudioBuffer<float> wetBuffer;
    
    void updateReverbParameters();
};
