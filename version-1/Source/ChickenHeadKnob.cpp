#include "ChickenHeadKnob.h"

namespace
{
    // JUCE rotary angles are "clockwise from top" (0 = 12 o'clock). Convert to math (0 = east, positive CCW) for drawing.
    inline float clockwiseFromTopToMath(float angleRad)
    {
        return -juce::MathConstants<float>::halfPi + angleRad;
    }
}

ChickenHeadKnobLookAndFeel::ChickenHeadKnobLookAndFeel(bool isWhiteKnob)
    : whiteKnob(isWhiteKnob)
{
    if (whiteKnob)
    {
        knobColor = juce::Colour(0xFFFFFFFF);       // White fill (ADSR)
        indicatorColor = juce::Colour(0xFF1a1a1a); // Black chicken-head (ADSR)
    }
    else
    {
        knobColor = juce::Colour(0xFF0d0d0d);       // Black/dark (Figma effect rack)
        indicatorColor = juce::Colour(0xFFE87C2C);  // Orange chicken-head
    }
}

void ChickenHeadKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                   float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                                   juce::Slider&)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2);
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto lineW = juce::jmin(6.0f, radius * 0.4f);
    auto arcRadius = radius - lineW * 0.5f;

    // JUCE passes angles "clockwise from top". Convert to math (0 = east, positive CCW) for indicator.
    float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    float toAngleMath = clockwiseFromTopToMath(toAngle);

    // Knob fill: solid white (ADSR) or black (FX)
    auto fillBounds = bounds.reduced(lineW);
    g.setColour(knobColor);
    g.fillEllipse(fillBounds);

    // Outer ring
    g.setColour(whiteKnob ? juce::Colour(0xFFcccccc) : juce::Colour(0xFF1a1a1a));
    g.drawEllipse(bounds.reduced(lineW * 0.5f), lineW * 0.5f);

    // Chicken-head indicator (aligned with value; zero = 7 o'clock when setRotaryParameters(210°, 480°) used)
    auto tipX = centre.x + arcRadius * std::cos(toAngleMath);
    auto tipY = centre.y + arcRadius * std::sin(toAngleMath);
    g.setColour(indicatorColor);
    g.drawLine(centre.x, centre.y, tipX, tipY, lineW * 0.9f);

    g.fillEllipse(centre.x - lineW * 0.5f, centre.y - lineW * 0.5f, lineW, lineW);
}
