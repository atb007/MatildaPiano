#include "Parameters.h"

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // ADSR parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ATTACK, "Attack",
        juce::NormalisableRange<float>(ATTACK_MIN, ATTACK_MAX, 0.01f),
        ATTACK_DEFAULT,
        "s"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        DECAY, "Decay",
        juce::NormalisableRange<float>(DECAY_MIN, DECAY_MAX, 0.01f),
        DECAY_DEFAULT,
        "s"
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        SUSTAIN, "Sustain",
        juce::NormalisableRange<float>(SUSTAIN_MIN, SUSTAIN_MAX, 0.01f),
        SUSTAIN_DEFAULT,
        ""
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        RELEASE, "Release",
        juce::NormalisableRange<float>(RELEASE_MIN, RELEASE_MAX, 0.01f),
        RELEASE_DEFAULT,
        "s"
    ));
    
    // Effects parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        REVERB, "Reverb",
        juce::NormalisableRange<float>(REVERB_MIN, REVERB_MAX, 0.01f),
        REVERB_DEFAULT,
        ""
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        DELAY_TIME, "Delay Time",
        juce::NormalisableRange<float>(DELAY_TIME_MIN, DELAY_TIME_MAX, 0.01f),
        DELAY_TIME_DEFAULT,
        ""
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        MASTER_VOL, "Master Volume",
        juce::NormalisableRange<float>(MASTER_VOL_MIN, MASTER_VOL_MAX, 0.01f),
        MASTER_VOL_DEFAULT,
        ""
    ));
    
    // XY Pad parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        XY_X, "XY X",
        juce::NormalisableRange<float>(XY_X_MIN, XY_X_MAX, 0.01f),
        XY_X_DEFAULT,
        ""
    ));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        XY_Y, "XY Y",
        juce::NormalisableRange<float>(XY_Y_MIN, XY_Y_MAX, 0.01f),
        XY_Y_DEFAULT,
        ""
    ));
    
    return { params.begin(), params.end() };
}
