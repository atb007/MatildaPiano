#pragma once

#include <JuceHeader.h>

class DelayModule
{
public:
    DelayModule();
    ~DelayModule() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::AudioBlock<float>& block);
    void reset();
    
    void setDelayTime(float normalizedTime); // 0.0 to 1.0
    void setMix(float mix); // 0.0 to 1.0
    void setHostTempo(double tempoBPM); // Host tempo in BPM
    
    // Get current delay time display string (e.g., "1/4", "1/8T")
    juce::String getDelayTimeDisplay() const;
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    
    float delayTimeNormalized = 0.5f;
    float mix = 0.0f;
    double hostTempo = 120.0;
    double sampleRate = 44100.0;
    float lastDelaySamples = -1.0f;
    
    // Musical subdivisions (const char* so the type is literal and constexpr is valid)
    struct Subdivision
    {
        float beats;
        const char* display;
    };
    
    static constexpr Subdivision subdivisions[9] = {
        { 1.0f / 64.0f, "1/64" },
        { 1.0f / 32.0f, "1/32" },
        { 1.0f / 16.0f, "1/16" },
        { 1.0f / 8.0f, "1/8" },
        { 1.0f / 6.0f, "1/8T" },  // Triplet
        { 1.0f / 4.0f, "1/4" },
        { 1.0f / 3.0f, "1/4T" },  // Triplet
        { 1.0f / 2.0f, "1/2" },
        { 1.0f, "1" }  // 1 bar
    };
    
    void updateDelayTime();
    int getSubdivisionIndex(float normalized) const;
};
