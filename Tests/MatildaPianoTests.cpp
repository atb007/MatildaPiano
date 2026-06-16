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


static int runDenseMidiRenderTest()
{
    MatildaPianoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    juce::AudioBuffer<float> buffer(2, 512);
    float maxPeak = 0.0f;
    for (int block = 0; block < 40; ++block)
    {
        buffer.clear();
        juce::MidiBuffer midi;
        if (block == 0)
        {
            for (int note = 48; note < 48 + 20; ++note)
                midi.addEvent(juce::MidiMessage::noteOn(1, note, (juce::uint8) 80), 0);
        }
        processor.processBlock(buffer, midi);
        maxPeak = juce::jmax(maxPeak, buffer.getMagnitude(0, 512));
    }
    processor.releaseResources();
    if (maxPeak < 0.0001f) { std::cerr << "FAIL dense midi silence peak="<<maxPeak<<"\n"; return 1; }
    std::cout << "Dense MIDI peak: " << maxPeak << "\n";
    return 0;
}

static int runOutOfRangeNoteTest()
{
    MatildaPianoAudioProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 15, (juce::uint8) 100), 0);
    processor.processBlock(buffer, midi);
    processor.releaseResources();
    const float peak = buffer.getMagnitude(0, 512);
    if (peak < 0.0001f)
    {
        std::cerr << "FAIL: note 15 silent peak=" << peak << "\n";
        return 1;
    }
    std::cout << "Note 15 peak: " << peak << "\n";
    return 0;
}

static int runMidiRenderTest()
{
    MatildaPianoAudioProcessor processor;
    if (processor.getSampleLoadStatus().containsIgnoreCase("ERROR"))
    {
        std::cerr << "FAIL: neural engine not loaded: " << processor.getSampleLoadStatus().toStdString() << "\n";
        return 1;
    }

    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> buffer(2, 512);
    float maxPeak = 0.0f;

    for (int block = 0; block < 80; ++block)
    {
        buffer.clear();
        juce::MidiBuffer midi;
        if (block == 0)
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100), 0);
        if (block == 60)
            midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);

        processor.processBlock(buffer, midi);
        maxPeak = juce::jmax(maxPeak, buffer.getMagnitude(0, 512));
        maxPeak = juce::jmax(maxPeak, buffer.getMagnitude(1, 512));
    }

    processor.releaseResources();

    if (maxPeak < 0.0001f)
    {
        std::cerr << "FAIL: processBlock produced near silence, peak=" << maxPeak << "\n";
        return 1;
    }

    std::cout << "MIDI render peak: " << maxPeak << "\n";
    return 0;
}

int main(int argc, char* argv[])
{
    juce::ignoreUnused(argc, argv);
    juce::ScopedJuceInitialiser_GUI init;

    int failed = 0;
    failed += runParameterLayoutTests();
    failed += runMidiRenderTest();
    failed += runDenseMidiRenderTest();
    failed += runOutOfRangeNoteTest();

    if (failed > 0)
    {
        std::cerr << "Total failures: " << failed << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
