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
    
    // Prepare delay lines for pitch modulation (200ms max delay)
    int maxDelaySamples = static_cast<int>(sampleRate * 0.2);
    delayLineL.resize(maxDelaySamples, 0.0f);
    delayLineR.resize(maxDelaySamples, 0.0f);
    delayWritePos = 0;
    
    // Prepare tone filter
    toneFilter.prepare(spec);
    updateFilters();
}

void TapeModule::process(juce::dsp::AudioBlock<float>& block)
{
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    // Update oscillator frequencies based on wow/flutter rate
    float wowFreq = 0.5f + wowFlutterRate * 3.0f; // 0.5 to 3.5 Hz
    float flutterFreq = 5.0f + wowFlutterRate * 15.0f; // 5 to 20 Hz
    
    wowOscillator.setFrequency(wowFreq);
    flutterOscillator.setFrequency(flutterFreq);
    
    // Process each sample
    for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
    {
        // Get wow and flutter modulation (for pitch shifting via delay)
        float wow = wowOscillator.processSample(0.0f) * wowFlutterRate;
        float flutter = flutterOscillator.processSample(0.0f) * wowFlutterRate;
        
        // Convert modulation to delay time (in samples)
        // More modulation = more pitch wobble
        float baseDelay = 10.0f; // Base delay of 10 samples (~0.2ms at 44.1kHz)
        float modulationAmount = (wow * 8.0f + flutter * 4.0f); // Strong modulation
        float delayInSamples = baseDelay + modulationAmount;
        
        // Clamp delay to valid range
        delayInSamples = juce::jlimit(1.0f, (float)(delayLineL.size() - 1), delayInSamples);
        
        // Process each channel
        for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
        {
            auto* channelData = block.getChannelPointer(channel);
            float inputSample = channelData[sample];
            
            // Write to delay line
            if (channel == 0)
            {
                delayLineL[delayWritePos] = inputSample;
                // Read from delay line with variable delay (creates pitch shift)
                float delayedSample = readDelayLine(delayLineL, delayInSamples);
                channelData[sample] = applySaturation(delayedSample);
            }
            else
            {
                delayLineR[delayWritePos] = inputSample;
                float delayedSample = readDelayLine(delayLineR, delayInSamples);
                channelData[sample] = applySaturation(delayedSample);
            }
        }
        
        // Advance write position
        delayWritePos = (delayWritePos + 1) % delayLineL.size();
    }
    
    // Apply tone filter
    toneFilter.process(context);
}

float TapeModule::readDelayLine(const std::vector<float>& delayLine, float delayInSamples)
{
    int delaySize = static_cast<int>(delayLine.size());
    
    // Calculate read position with fractional part for interpolation
    float readPosFloat = delayWritePos - delayInSamples;
    if (readPosFloat < 0.0f)
        readPosFloat += delaySize;
    
    int readPos1 = static_cast<int>(readPosFloat);
    int readPos2 = (readPos1 + 1) % delaySize;
    float frac = readPosFloat - readPos1;
    
    // Linear interpolation
    return delayLine[readPos1] * (1.0f - frac) + delayLine[readPos2] * frac;
}

void TapeModule::reset()
{
    wowOscillator.reset();
    flutterOscillator.reset();
    toneFilter.reset();
    
    // Clear delay lines
    std::fill(delayLineL.begin(), delayLineL.end(), 0.0f);
    std::fill(delayLineR.begin(), delayLineR.end(), 0.0f);
    delayWritePos = 0;
}

void TapeModule::setWowFlutterRate(float rate)
{
    wowFlutterRate = juce::jlimit(0.0f, 1.0f, rate);
    DBG("TapeModule: WowFlutterRate set to " + juce::String(wowFlutterRate));
}

void TapeModule::setSaturation(float sat)
{
    saturation = juce::jlimit(0.0f, 1.0f, sat);
    DBG("TapeModule: Saturation set to " + juce::String(saturation));
}

void TapeModule::setToneCutoff(float cutoff)
{
    toneCutoff = juce::jlimit(0.0f, 1.0f, cutoff);
    updateFilters();
    DBG("TapeModule: ToneCutoff set to " + juce::String(toneCutoff));
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
    // MUCH stronger drive so XY pad (Y axis) is VERY audible
    float drive = 1.0f + saturation * 8.0f; // was 4.0f - now up to 9x drive!
    float driven = sample * drive;
    float saturated = std::tanh(driven);
    // Full wet mix when saturation is high
    return sample * (1.0f - saturation) + saturated * saturation;
}
