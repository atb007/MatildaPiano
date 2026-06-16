#pragma once

#include <array>
#include <vector>

namespace PhysicalBlend
{
    inline const std::vector<float>& frequencyRatiosForMidi(int midiNote)
    {
        static const std::vector<float> g1f = { 2.00f, 3.01f, 4.01f, 5.02f, 7.03f, 9.07f, 10.09f, 12.13f,
                                              16.34f, 18.47f, 23.97f, 43.46f, 44.6f };
        static const std::vector<float> g2f = { 2.00f, 3.01f, 4.01f, 6.01f, 9.04f, 11.09f, 13.13f, 14.15f,
                                              14.19f, 15.19f, 20.47f, 22.57f, 23.69f };
        static const std::vector<float> g3f = { 1.0f, 2.0f, 3.01f, 4.01f, 5.03f, 6.04f, 7.05f, 8.07f,
                                              9.1f, 10.13f, 11.17f, 13.25f, 15.38f };

        if (midiNote <= 30)
            return g1f;
        if (midiNote <= 50)
            return g2f;
        return g3f;
    }

    inline const std::vector<float>& amplitudesForMidi(int midiNote)
    {
        static const std::vector<float> g1a = { 0.08361312127094107f, 0.17722663981890502f, 0.0785634447859186f,
            0.055689120378443015f, 0.07626867298344961f, 0.06812575280265179f, 0.08195590921387327f,
            0.12262551050085131f, 0.047938598896454104f, 0.05283410342757357f, 0.06610008150892285f,
            0.04377700225646506f, 0.045282042155550685f };
        static const std::vector<float> g2a = { 0.24269384285644227f, 0.07052851765757988f, 0.047061190142169355f,
            0.10840876708699342f, 0.09188640321466986f, 0.08144203829761783f, 0.04228555577561662f,
            0.09610226462853295f, 0.03785757321294113f, 0.06726106660383388f, 0.03668751097990114f,
            0.041375189678632414f, 0.036410079865069396f };
        static const std::vector<float> g3a = { 0.10197161354012407f, 0.25818838322735743f, 0.06275858234133298f,
            0.14671081868086436f, 0.05433834743401193f, 0.0374010296158477f, 0.13903293707516398f,
            0.06196062709021847f, 0.05305020221441158f, 0.03110025141297645f, 0.02143888118545714f,
            0.0159564601968566f, 0.01609186598537729f };

        if (midiNote <= 30)
            return g1a;
        if (midiNote <= 50)
            return g2a;
        return g3a;
    }

    /** Low notes → more physical (reference alpha/beta from Voices.cpp). */
    inline float neuralBlendWeight(float normalizedPitch, float cabinetResonance01, bool sustainPedalDown)
    {
        const float alpha = 1.0f - (1.0f - normalizedPitch) * 0.95f;
        const float physicalWeight = (1.0f - alpha) * (0.35f + cabinetResonance01 * 0.65f);
        return juce::jlimit(0.0f, 0.45f, physicalWeight);
    }
}
