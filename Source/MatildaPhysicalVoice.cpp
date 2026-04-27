#include "MatildaPhysicalVoice.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kMidiNoteA4 = 69.0f;
    constexpr float kFreqA4 = 440.0f;

    float midiNoteToHz(int note, float pitchRatio)
    {
        const float n = (float)note + std::log2(pitchRatio) * 12.0f;
        return kFreqA4 * std::pow(2.0f, (n - kMidiNoteA4) / 12.0f);
    }
}

MatildaPhysicalVoice::MatildaPhysicalVoice()
{
    adsrParams.attack = 0.1f;
    adsrParams.decay = 0.3f;
    adsrParams.sustain = 0.7f;
    adsrParams.release = 0.5f;
    adsr.setParameters(adsrParams);
}

bool MatildaPhysicalVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<MatildaPhysicalSound*>(sound) != nullptr;
}

void MatildaPhysicalVoice::setSampleRate(double sr)
{
    sampleRate = sr;
    adsr.setSampleRate(sr);
    updateADSRParameters();

    const auto len = (size_t)std::ceil(sr * 0.22) + 8;
    delayLine.assign(juce::jmax((size_t)2048, len), 0.0f);
    writeIndex = 0;
    loopLpState = 0.0f;
}

void MatildaPhysicalVoice::resetDelayLine()
{
    std::fill(delayLine.begin(), delayLine.end(), 0.0f);
    writeIndex = 0;
    loopLpState = 0.0f;
    loopLpState2 = 0.0f;
    dcBlockerX1 = 0.0f;
    dcBlockerY1 = 0.0f;
    energyTracker = 0.0f;
    badSampleCount = 0;
    hammerNoiseHpState = 0.0f;
    thumpDecay = 0.0f;
}

void MatildaPhysicalVoice::recomputeDelayLength()
{
    const float hz = juce::jlimit(20.0f, 16000.0f, midiNoteToHz(currentMidiNote, pitchWheelRatio));
    
    // Add inharmonicity: stretch the fundamental slightly lower to simulate stiff string
    // (overtones will be stretched higher in the frequency domain due to the shortened delay)
    const float inharmFactor = 1.0f + inharmonicity * 0.0003f;
    
    delayInSamples = (float)(sampleRate / ((double)hz * inharmFactor));
    const float maxD = (float)delayLine.size() - 2.0f;
    delayInSamples = juce::jlimit(2.0f, maxD, delayInSamples);
}

void MatildaPhysicalVoice::updateLoopTimbreForMidiNote()
{
    const float x = juce::jmap((float)currentMidiNote, 21.0f, 108.0f, 0.0f, 1.0f);
    
    // Moderate inharmonicity
    inharmonicity = juce::jmap(x, 0.0f, 1.0f, 2.5f, 0.5f);
    
    // Higher feedback for sustain and volume
    feedbackGain = juce::jmap(x, 0.0f, 1.0f, 0.983f, 0.990f);
    
    // Less aggressive damping (was killing volume)
    loopLowPassCoeff = juce::jmap(x, 0.0f, 1.0f, 0.18f, 0.50f);
    loopLowPassCoeff2 = juce::jmap(x, 0.0f, 1.0f, 0.55f, 0.78f);
}

float MatildaPhysicalVoice::readDelay(float delay) const
{
    if (delayLine.empty()) return 0.0f;
    const int N = (int)delayLine.size();
    double rp = (double)writeIndex - (double)delay;
    while (rp < 0.0) rp += N;
    const int i0 = (int)std::floor(rp) % N;
    int i1 = i0 - 1;
    if (i1 < 0) i1 += N;
    const float f = (float)(rp - std::floor(rp));
    return delayLine[(size_t)i0] * (1.0f - f) + delayLine[(size_t)i1] * f;
}

void MatildaPhysicalVoice::writeSample(float v)
{
    if (delayLine.empty()) return;
    const int N = (int)delayLine.size();
    delayLine[(size_t)(writeIndex % N)] = v;
    ++writeIndex;
}

void MatildaPhysicalVoice::startNote(int midiNoteNumber, float velocity,
                                     juce::SynthesiserSound* sound,
                                     int currentPitchWheelPosition)
{
    juce::ignoreUnused(sound);
    if (dynamic_cast<MatildaPhysicalSound*>(sound) == nullptr) return;

    currentMidiNote = midiNoteNumber;
    // Cap velocity aggressively for Musical Typing (prevents screeches from high velocity)
    currentVelocity = juce::jlimit(0.0f, 0.65f, velocity);
    isNoteOn = true;

    pitchWheelMoved(currentPitchWheelPosition);

    resetDelayLine();
    recomputeDelayLength();
    updateLoopTimbreForMidiNote();

    // Very short hammer contact: 0.5-2ms for piano
    const int minExc = juce::jmax(2, (int)std::round(sampleRate * 0.0005));
    const int maxExc = juce::jmax(minExc + 1, (int)std::round(sampleRate * 0.002));
    excitationTotal = juce::jlimit(minExc, maxExc,
                                   (int)std::round(delayInSamples * 0.08f));
    excitationSamplesLeft = excitationTotal;

    // Hammer level: balanced for short duration
    hammerLevel = juce::jmap(currentVelocity, 0.0f, 0.65f, 0.5f, 1.5f);
    
    // Initialize percussive body thump (key hitting frame)
    thumpDecay = currentVelocity * 1.2f;

    adsr.noteOn();
}

void MatildaPhysicalVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity, allowTailOff);
    if (isNoteOn)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        clearCurrentNote();
        isNoteOn = false;
    }
}

void MatildaPhysicalVoice::pitchWheelMoved(int newPitchWheelValue)
{
    const float semis = ((float)newPitchWheelValue - 8192.0f) / 8192.0f * 2.0f;
    pitchWheelRatio = std::pow(2.0f, semis / 12.0f);
    if (getCurrentlyPlayingNote() >= 0)
        recomputeDelayLength();
}

void MatildaPhysicalVoice::controllerMoved(int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused(controllerNumber, newControllerValue);
}

void MatildaPhysicalVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                           int startSample, int numSamples)
{
    if (dynamic_cast<MatildaPhysicalSound*>(getCurrentlyPlayingSound().get()) == nullptr)
        return;

    if (delayLine.empty()) return;

    for (int i = 0; i < numSamples; ++i)
    {
        float delayed = readDelay(delayInSamples);
        
        // Monitor energy and auto-reset if exploding (but be less aggressive)
        const float absDelayed = std::abs(delayed);
        energyTracker = 0.99f * energyTracker + 0.01f * absDelayed;
        
        if (absDelayed > 4.0f || energyTracker > 1.8f)
        {
            badSampleCount++;
            if (badSampleCount > 20)  // More tolerance before reset
            {
                // Emergency reset: clear the delay line
                resetDelayLine();
                adsr.noteOff();
                break;
            }
        }
        else
        {
            badSampleCount = juce::jmax(0, badSampleCount - 1);
        }
        
        // Soft clip the delayed signal
        delayed = std::tanh(delayed * 0.7f) / 0.7f;
        
        const float loopIn = delayed;

        // Two-pole damping (more realistic string losses)
        loopLpState = loopLowPassCoeff * loopIn + (1.0f - loopLowPassCoeff) * loopLpState;
        loopLpState2 = loopLowPassCoeff2 * loopLpState + (1.0f - loopLowPassCoeff2) * loopLpState2;
        float filtered = loopLpState2;
        
        // DC blocker (highpass at ~20 Hz to remove DC buildup)
        const float dcCoeff = 0.995f;
        float dcOut = filtered - dcBlockerX1 + dcCoeff * dcBlockerY1;
        dcBlockerX1 = filtered;
        dcBlockerY1 = dcOut;
        filtered = dcOut;

        float exc = 0.0f;
        if (excitationSamplesLeft > 0)
        {
            const float t = 1.0f - (float)excitationSamplesLeft / (float)juce::jmax(1, excitationTotal);
            
            // Very short Gaussian envelope (0.5-2ms contact)
            const float env = std::exp(-12.0f * (t - 0.3f) * (t - 0.3f));
            
            // Velocity-dependent spectral content
            const float brightness = juce::jmap(currentVelocity, 0.0f, 0.65f, 0.2f, 0.85f);
            float noise = rng.nextFloat() * 2.0f - 1.0f;
            
            // High-frequency emphasis for brightness
            float noiseHp = noise - 0.7f * hammerNoiseHpState;
            hammerNoiseHpState = noise;
            noise = brightness * noiseHp + (1.0f - brightness) * noise;
            
            exc = noise * hammerLevel * env;
            --excitationSamplesLeft;
        }

        float newS = filtered * feedbackGain + exc;
        
        // Soft clip before writing back (gentler than hard limit)
        newS = std::tanh(newS);
        writeSample(newS);

        // String output with boosted gain
        float out = delayed * (2.2f + currentVelocity * 1.5f);
        
        // Add percussive "thump" (key/soundboard impact) - separate from string!
        // This is what differentiates piano from plucked string
        if (thumpDecay > 0.001f)
        {
            out += thumpDecay * 0.35f * (rng.nextFloat() * 2.0f - 1.0f);
            thumpDecay *= 0.985f;  // Fast decay (few ms)
        }
        
        const float env = adsr.getNextSample();
        out *= env;

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample(ch, startSample + i, out);

        if (!adsr.isActive() && isNoteOn)
        {
            clearCurrentNote();
            isNoteOn = false;
            break;
        }
    }
}

void MatildaPhysicalVoice::setAttack(float attackSeconds)
{
    adsrParams.attack = attackSeconds;
    updateADSRParameters();
}

void MatildaPhysicalVoice::setDecay(float decaySeconds)
{
    adsrParams.decay = decaySeconds;
    updateADSRParameters();
}

void MatildaPhysicalVoice::setSustain(float sustainLevel)
{
    adsrParams.sustain = juce::jlimit(0.0f, 1.0f, sustainLevel);
    updateADSRParameters();
}

void MatildaPhysicalVoice::setRelease(float releaseSeconds)
{
    adsrParams.release = releaseSeconds;
    updateADSRParameters();
}

void MatildaPhysicalVoice::updateADSRParameters()
{
    adsr.setParameters(adsrParams);
}
