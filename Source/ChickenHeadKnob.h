#pragma once

#include <JuceHeader.h>

class ChickenHeadKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ChickenHeadKnobLookAndFeel(bool isWhiteKnob);
    ~ChickenHeadKnobLookAndFeel() override = default;
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
    
private:
    bool whiteKnob; // true for ADSR knobs (white), false for FX knobs (green)
    juce::Colour knobColor;
    juce::Colour indicatorColor;
};
