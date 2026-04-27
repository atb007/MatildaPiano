#include "TapeModule.h"

TapeModule::TapeModule()
{
    // Initialize oscillators for wow and flutter
    wowOscillator.initialise([](float x) { return std::sin(x); }, 128);
    flutterOscillator.initialise([](float x) { return std::sin(x); }, 128);
}

void TapeModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    // Prepare oscillators
    wowOscillator.prepare(spec);
    flutterOscillator.prepare(spec);
    
    // Prepare tone filter
    toneFilter.prepare(spec);
    updateFilters();
}

void TapeModule::process(juce::dsp::AudioBlock<float>& block)
{
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    // Update oscillator frequencies based on wow/flutter rate
    float wowFreq = 0.5f + wowFlutterRate * 2.0f; // 0.5 to 2.5 Hz
    float flutterFreq = 5.0f + wowFlutterRate * 10.0f; // 5 to 15 Hz
    
    wowOscillator.setFrequency(wowFreq);
    flutterOscillator.setFrequency(flutterFreq);
    
    // Process each sample
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* channelData = block.getChannelPointer(channel);
        
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            // Wow/flutter: stronger modulation so XY pad (X axis) is clearly audible
            float wow = wowOscillator.processSample(0.0f) * 0.06f * wowFlutterRate;
            float flutter = flutterOscillator.processSample(0.0f) * 0.04f * wowFlutterRate;
            float modulation = wow + flutter;
            float originalSample = channelData[sample];
            float modulatedSample = originalSample * (1.0f + modulation);
            modulatedSample = applySaturation(modulatedSample);
            channelData[sample] = modulatedSample;
        }
    }
    
    // Apply tone filter
    toneFilter.process(context);
}

void TapeModule::reset()
{
    wowOscillator.reset();
    flutterOscillator.reset();
    toneFilter.reset();
}

void TapeModule::setWowFlutterRate(float rate)
{
    wowFlutterRate = juce::jlimit(0.0f, 1.0f, rate);
}

void TapeModule::setSaturation(float sat)
{
    saturation = juce::jlimit(0.0f, 1.0f, sat);
}

void TapeModule::setToneCutoff(float cutoff)
{
    toneCutoff = juce::jlimit(0.0f, 1.0f, cutoff);
    updateFilters();
}

void TapeModule::updateFilters()
{
    // Y axis: 0 = bright, 1 = darker. Wider range so XY pad movement is clearly audible
    float cutoffHz = 18000.0f - (toneCutoff * 16000.0f); // 18kHz down to 2kHz
    if (sampleRate > 0.0)
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, cutoffHz, 0.707f);
}

float TapeModule::applySaturation(float sample)
{
    if (saturation < 0.001f)
        return sample;
    // Stronger drive so XY pad (Y axis) is clearly audible
    float drive = 1.0f + saturation * 4.0f;
    float driven = sample * drive;
    float saturated = std::tanh(driven);
    // More wet mix so saturation is obvious
    float wet = saturation * 0.85f + 0.15f;
    return sample * (1.0f - wet) + saturated * wet;
}
