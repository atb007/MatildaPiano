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
        // ADSR knobs: White base with purple/blue indicator
        knobFillColor = juce::Colour(0xFFFFFFFF);           // White fill
        inactiveIndicatorColor = juce::Colour(0xFF686DD1);  // Purple/blue indicator (#686DD1)
        activeIndicatorColor = juce::Colour(0xFF686DD1);    // Purple/blue indicator (#686DD1)
        activeArcColor = juce::Colour(0xFFFDB813);          // Golden arc indicator (unused for now)
    }
    else
    {
        // Effect knobs: Black base with golden indicator
        knobFillColor = juce::Colour(0xFF1a1a1a);           // Black/dark fill
        inactiveIndicatorColor = juce::Colour(0xFFFDB813);  // Golden yellow indicator
        activeIndicatorColor = juce::Colour(0xFFFDB813);    // Golden yellow indicator
        activeArcColor = juce::Colour(0xFFFDB813);          // Golden arc indicator (unused for now)
    }
}

void ChickenHeadKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                                   float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                                   juce::Slider&)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto centre = bounds.getCentre();
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    
    // Draw main knob circle (white for ADSR, black for FX)
    g.setColour(knobFillColor);
    g.fillEllipse(bounds);
    
    // Draw subtle outer border (very thin)
    g.setColour(whiteKnob ? juce::Colour(0xFFE0E0E0) : juce::Colour(0xFF2a2a2a));
    g.drawEllipse(bounds, 0.5f);
    
    // Calculate indicator line position
    float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    float toAngleMath = clockwiseFromTopToMath(toAngle);
    
    // Draw indicator line - same color and length at all positions
    const float indicatorLength = radius * 0.55f; // Fixed length
    const float indicatorWidth = 2.5f; // Fixed width
    const float indicatorStartRadius = radius * 0.15f; // Start from near center
    
    auto startX = centre.x + indicatorStartRadius * std::cos(toAngleMath);
    auto startY = centre.y + indicatorStartRadius * std::sin(toAngleMath);
    auto endX = centre.x + indicatorLength * std::cos(toAngleMath);
    auto endY = centre.y + indicatorLength * std::sin(toAngleMath);
    
    // Always use the active indicator color (purple for ADSR, golden for FX)
    g.setColour(activeIndicatorColor);
    g.drawLine(startX, startY, endX, endY, indicatorWidth);
}
