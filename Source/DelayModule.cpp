#include "DelayModule.h"
#include <cmath>

DelayModule::DelayModule()
{
}

void DelayModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    // Maximum delay time: 4 bars at 60 BPM = ~16 seconds
    float maxDelaySeconds = 16.0f;
    int maxDelaySamples = static_cast<int>(maxDelaySeconds * sampleRate);
    
    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(spec);
    lastDelaySamples = -1.0f;
    updateDelayTime();
}

void DelayModule::process(juce::dsp::AudioBlock<float>& block)
{
    // Update delay time only when params changed (avoids zipper/glitches from recalc every block)
    updateDelayTime();
    
    // Process each channel
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            float input = channelData[sample];
            // Standard delay order per JUCE: push current input, then pop to get delayed sample
            delayLine.pushSample(static_cast<int>(channel), input);
            float delayed = delayLine.popSample(static_cast<int>(channel), -1.0f);
            channelData[sample] = input * (1.0f - mix) + delayed * mix;
        }
    }
}

void DelayModule::reset()
{
    delayLine.reset();
}

void DelayModule::setDelayTime(float normalizedTime)
{
    delayTimeNormalized = juce::jlimit(0.0f, 1.0f, normalizedTime);
    updateDelayTime();
}

void DelayModule::setMix(float mixValue)
{
    mix = juce::jlimit(0.0f, 1.0f, mixValue);
}

void DelayModule::setHostTempo(double tempoBPM)
{
    hostTempo = tempoBPM > 0.0 ? tempoBPM : 120.0;
    updateDelayTime();
}

juce::String DelayModule::getDelayTimeDisplay() const
{
    int index = getSubdivisionIndex(delayTimeNormalized);
    return juce::String(subdivisions[index].display);
}

void DelayModule::updateDelayTime()
{
    int index = getSubdivisionIndex(delayTimeNormalized);
    float beats = subdivisions[index].beats;
    float secondsPerBeat = 60.0f / static_cast<float>(hostTempo);
    float delaySeconds = beats * secondsPerBeat;
    const float maxDelaySeconds = 1.0f;
    delaySeconds = juce::jmin(delaySeconds, maxDelaySeconds);
    float delaySamples = delaySeconds * static_cast<float>(sampleRate);
    // Only update when changed to avoid zipper noise / wonky behaviour
    if (std::abs(delaySamples - lastDelaySamples) > 0.5f)
    {
        lastDelaySamples = delaySamples;
        delayLine.setDelay(delaySamples);
    }
}

int DelayModule::getSubdivisionIndex(float normalized) const
{
    // Map normalized value (0.0 to 1.0) to subdivision index
    int index = static_cast<int>(normalized * 8.99f); // 0 to 8
    return juce::jlimit(0, 8, index);
}
