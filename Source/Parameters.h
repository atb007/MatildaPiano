#pragma once

#include <JuceHeader.h>

namespace Parameters
{
    // Parameter IDs
    constexpr const char* ATTACK = "attack";
    constexpr const char* DECAY = "decay";
    constexpr const char* SUSTAIN = "sustain";
    constexpr const char* RELEASE = "release";
    
    constexpr const char* REVERB = "reverb";
    constexpr const char* DELAY_TIME = "delayTime";
    constexpr const char* MASTER_VOL = "masterVol";
    
    constexpr const char* XY_X = "xyX";
    constexpr const char* XY_Y = "xyY";
    
    // Parameter ranges and defaults
    constexpr float ATTACK_MIN = 0.0f;
    constexpr float ATTACK_MAX = 5.0f;
    constexpr float ATTACK_DEFAULT = 0.1f;
    
    constexpr float DECAY_MIN = 0.0f;
    constexpr float DECAY_MAX = 5.0f;
    constexpr float DECAY_DEFAULT = 0.3f;
    
    constexpr float SUSTAIN_MIN = 0.0f;
    constexpr float SUSTAIN_MAX = 1.0f;
    constexpr float SUSTAIN_DEFAULT = 0.7f;
    
    constexpr float RELEASE_MIN = 0.0f;
    constexpr float RELEASE_MAX = 5.0f;
    constexpr float RELEASE_DEFAULT = 0.5f;
    
    constexpr float REVERB_MIN = 0.0f;
    constexpr float REVERB_MAX = 1.0f;
    constexpr float REVERB_DEFAULT = 0.3f;
    
    constexpr float DELAY_TIME_MIN = 0.0f;
    constexpr float DELAY_TIME_MAX = 1.0f;
    constexpr float DELAY_TIME_DEFAULT = 0.5f;
    
    constexpr float MASTER_VOL_MIN = 0.0f;
    constexpr float MASTER_VOL_MAX = 1.0f;
    constexpr float MASTER_VOL_DEFAULT = 0.8f;
    
    constexpr float XY_X_MIN = 0.0f;
    constexpr float XY_X_MAX = 1.0f;
    constexpr float XY_X_DEFAULT = 0.5f;
    
    constexpr float XY_Y_MIN = 0.0f;
    constexpr float XY_Y_MAX = 1.0f;
    constexpr float XY_Y_DEFAULT = 0.5f;
    
    // Create parameter layout for AudioProcessorValueTreeState
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
}
