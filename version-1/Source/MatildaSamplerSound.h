#pragma once

#include <JuceHeader.h>

class MatildaSamplerSound : public juce::SamplerSound
{
public:
    MatildaSamplerSound(const juce::String& name,
                       juce::AudioFormatReader& source,
                       const juce::BigInteger& notes,
                       int midiNoteForNormalPitch,
                       double attackTimeSecs,
                       double releaseTimeSecs,
                       double maxSampleLengthSeconds);
    
    ~MatildaSamplerSound() override = default;
    
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaSamplerSound)
};
