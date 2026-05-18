#pragma once

#include <JuceHeader.h>

class TapeModule
{
public:
    TapeModule();
    ~TapeModule() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);
    void reset();
    
    void setWowFlutterRate(float rate);  // 0.0 to 1.0
    void setSaturation(float saturation); // 0.0 to 1.0
    void setToneCutoff(float cutoff);     // Normalized cutoff (0.0 to 1.0)
    
private:
    juce::dsp::Oscillator<float> wowOscillator;
    juce::dsp::Oscillator<float> flutterOscillator;
    juce::dsp::IIR::Filter<float> toneFilter;
    
    // Delay line for pitch modulation
    std::vector<float> delayLineL;
    std::vector<float> delayLineR;
    int delayWritePos = 0;
    
    float wowFlutterRate = 0.0f;
    float saturation = 0.0f;
    float toneCutoff = 1.0f;
    
    double sampleRate = 44100.0;
    
    void updateFilters();
    float applySaturation(float sample);
    float readDelayLine(const std::vector<float>& delayLine, float delayInSamples);
};
