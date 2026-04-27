#include "MatildaSamplerSound.h"

MatildaSamplerSound::MatildaSamplerSound(const juce::String& name,
                                         juce::AudioFormatReader& source,
                                         const juce::BigInteger& notes,
                                         int midiNoteForNormalPitch,
                                         double attackTimeSecs,
                                         double releaseTimeSecs,
                                         double maxSampleLengthSeconds)
    : juce::SamplerSound(name, source, notes, midiNoteForNormalPitch,
                        attackTimeSecs, releaseTimeSecs, maxSampleLengthSeconds)
{
}

bool MatildaSamplerSound::appliesToNote(int midiNoteNumber)
{
    return juce::SamplerSound::appliesToNote(midiNoteNumber);
}

bool MatildaSamplerSound::appliesToChannel(int midiChannel)
{
    return juce::SamplerSound::appliesToChannel(midiChannel);
}
