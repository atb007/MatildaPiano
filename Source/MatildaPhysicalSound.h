#pragma once

#include <JuceHeader.h>

/** Single sound that covers the full MIDI range for the physical string engine (v2). */
class MatildaPhysicalSound final : public juce::SynthesiserSound
{
public:
    MatildaPhysicalSound() = default;
    ~MatildaPhysicalSound() override = default;

    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaPhysicalSound)
};
