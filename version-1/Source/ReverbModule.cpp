#include "ReverbModule.h"

ReverbModule::ReverbModule()
{
    updateReverbParameters();
}

void ReverbModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    reverb.prepare(spec);
    updateReverbParameters();

    wetBuffer.setSize(static_cast<int>(spec.numChannels),
                      static_cast<int>(spec.maximumBlockSize),
                      false,  // keepExistingContent
                      false,  // clearExtraSpace
                      true);  // avoidReallocating
}

void ReverbModule::process(juce::dsp::AudioBlock<float>& block)
{
    const auto numChannels = static_cast<int>(block.getNumChannels());
    const auto numSamples = static_cast<int>(block.getNumSamples());

    if (wetBuffer.getNumChannels() < numChannels || wetBuffer.getNumSamples() < numSamples)
        wetBuffer.setSize(numChannels, numSamples, false, false, true);

    // Copy dry -> wetBuffer, then apply reverb to wetBuffer only.
    for (int ch = 0; ch < numChannels; ++ch)
        wetBuffer.copyFrom(ch, 0, block.getChannelPointer(static_cast<size_t>(ch)), numSamples);

    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    auto wetContext = juce::dsp::ProcessContextReplacing<float>(wetBlock);
    reverb.process(wetContext);

    // Mix wetBuffer back into the original block.
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* dryData = block.getChannelPointer(static_cast<size_t>(ch));
        const auto* wetData = wetBuffer.getReadPointer(ch);

        for (int i = 0; i < numSamples; ++i)
            dryData[i] = dryData[i] * (1.0f - mix) + wetData[i] * mix;
    }
}

void ReverbModule::reset()
{
    reverb.reset();
}

void ReverbModule::setMix(float mixValue)
{
    mix = juce::jlimit(0.0f, 1.0f, mixValue);
}

void ReverbModule::updateReverbParameters()
{
    reverbParams.roomSize = 0.7f;
    reverbParams.damping = 0.3f;
    reverbParams.width = 1.0f;
    reverbParams.wetLevel = 0.4f;
    reverbParams.dryLevel = 0.6f;
    reverbParams.freezeMode = 0.0f;
    
    reverb.setParameters(reverbParams);
}
