#pragma once
#include <JuceHeader.h>

/// MatildaNeuralSound - Sound definition for neural piano synthesis
/// Applies to standard 88-key piano range (MIDI 21-108: A0 to C8)
class MatildaNeuralSound : public juce::SynthesiserSound
{
public:
    MatildaNeuralSound() = default;
    
    bool appliesToNote(int midiNoteNumber) override
    {
        // 88 piano keys: A0 (21) to C8 (108)
        return (midiNoteNumber >= 21 && midiNoteNumber <= 108);
    }
    
    bool appliesToChannel(int /*midiChannel*/) override
    {
        return true;
    }
};
