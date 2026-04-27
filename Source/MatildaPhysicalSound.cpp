#include "MatildaPhysicalSound.h"

bool MatildaPhysicalSound::appliesToNote(int midiNoteNumber)
{
    return midiNoteNumber >= 0 && midiNoteNumber <= 127;
}

bool MatildaPhysicalSound::appliesToChannel(int midiChannel)
{
    juce::ignoreUnused(midiChannel);
    return true;
}
