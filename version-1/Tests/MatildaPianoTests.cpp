/**
 * Matilda Piano — unit tests.
 * Build and run: cmake --build build --target MatildaPianoTests && build/MatildaPiano_artefacts/Release/MatildaPianoTests (or Debug)
 * Or add as a run target in your IDE.
 */
#include <JuceHeader.h>
#include "../Source/Parameters.h"
#include "../Source/PluginProcessor.h"
#include <cstdlib>
#include <iostream>

static int runParameterLayoutTests()
{
    using namespace juce;
    int failed = 0;

    MatildaPianoAudioProcessor processor;
    const auto& params = processor.getParameters();

    // Expected parameter count (ADSR=4, Reverb, Delay, Master, XY_X, XY_Y = 9)
    if (params.size() != 9)
    {
        std::cerr << "FAIL: expected 9 parameters, got " << params.size() << "\n";
        ++failed;
    }

    const char* expectedIds[] = {
        Parameters::ATTACK, Parameters::DECAY, Parameters::SUSTAIN, Parameters::RELEASE,
        Parameters::REVERB, Parameters::DELAY_TIME, Parameters::MASTER_VOL,
        Parameters::XY_X, Parameters::XY_Y
    };
    for (int i = 0; i < params.size() && i < 9; ++i)
    {
        auto* p = params[i];
        if (p == nullptr)
        {
            std::cerr << "FAIL: parameter " << i << " is null\n";
            ++failed;
            continue;
        }
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(p);
        if (ranged == nullptr)
        {
            std::cerr << "FAIL: parameter " << i << " is not a RangedAudioParameter\n";
            ++failed;
            continue;
        }
        auto id = ranged->getParameterID();
        if (id != expectedIds[i])
        {
            std::cerr << "FAIL: parameter " << i << " expected ID \"" << expectedIds[i]
                      << "\", got \"" << id.toStdString() << "\"\n";
            ++failed;
        }
    }

    // Sanity: default values in range (for float parameters)
    for (int i = 0; i < params.size(); ++i)
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(params[i]);
        if (ranged == nullptr) continue;
        auto* p = dynamic_cast<AudioParameterFloat*>(params[i]);
        if (p == nullptr) continue;
        float v = p->get();
        auto range = ranged->getNormalisableRange();
        float lo = range.start;
        float hi = range.end;
        if (v < lo - 0.0001f || v > hi + 0.0001f)
        {
            std::cerr << "FAIL: parameter " << ranged->getParameterID().toStdString()
                      << " default " << v << " outside range [" << lo << ", " << hi << "]\n";
            ++failed;
        }
    }

    return failed;
}

int main(int argc, char* argv[])
{
    juce::ignoreUnused(argc, argv);
    juce::ScopedJuceInitialiser_GUI init;

    int failed = 0;
    failed += runParameterLayoutTests();

    if (failed > 0)
    {
        std::cerr << "Total failures: " << failed << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
