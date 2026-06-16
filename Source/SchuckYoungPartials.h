#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace SchuckYoung
{
    /** Reference beta at ~50% inharmonicity knob (PianoForte Voices.h). */
    constexpr float kReferenceBeta = 0.000033f;

    inline float fundamentalHzFromMidi(float midiNote)
    {
        return 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
    }

    /** Map 0..1 knob to stiffness coefficient B. */
    inline float betaFromNormalized(float inharmonicity01)
    {
        const float t = juce::jlimit(0.0f, 1.0f, inharmonicity01);
        return kReferenceBeta * (0.05f + t * 3.0f);
    }

    /** Partial index is 1-based (first partial = 1). */
    inline float partialHz(float fundamentalHz, int partialIndex, float beta)
    {
        const float n = static_cast<float>(juce::jmax(1, partialIndex));
        return fundamentalHz * n * std::sqrt(1.0f + beta * n * n);
    }

    inline float phaseIncrementPerSample(float frequencyHz, float sampleRate)
    {
        return juce::MathConstants<float>::twoPi * frequencyHz / juce::jmax(1.0f, sampleRate);
    }

    /** Fewer partials for high register — keeps ONNX + synthesis bounded. */
    inline int activePartialCountForMidi(int midiNote, int modelOutputSize)
    {
        if (midiNote > 80)
            return juce::jmin(8, modelOutputSize);
        if (midiNote > 60)
            return juce::jmin(16, modelOutputSize);
        return modelOutputSize;
    }
}
