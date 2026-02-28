#include "XYPadComponent.h"

XYPadComponent::XYPadComponent(juce::AudioProcessorValueTreeState& apvts)
    : valueTreeState(apvts),
      xAttachment(apvts, "xyX", xSlider),
      yAttachment(apvts, "xyY", ySlider)
{
    // Sliders are value-only; we paint background image + dot. Hide default slider drawing.
    // Initial x/y will be set by attachment from APVTS; we sync xValue/yValue in resized() so dot matches params.
    xSlider.setSliderStyle(juce::Slider::LinearBarVertical);
    xSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    xSlider.setRange(0.0, 1.0, 0.01);
    xSlider.setValue(0.5);
    xSlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    xSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    xSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);

    ySlider.setSliderStyle(juce::Slider::LinearBarVertical);
    ySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    ySlider.setRange(0.0, 1.0, 0.01);
    ySlider.setValue(0.5);
    ySlider.setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
    ySlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    ySlider.setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);

    // Keep sliders off-screen so they don't paint (no rectangular outline / sharp edges).
    // We paint the pad and dot ourselves; mouse handlers update slider values.
    addChildComponent(xSlider);
    addChildComponent(ySlider);

    xValue = 0.5f;
    yValue = 0.5f;
}

void XYPadComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float cornerRadius = 12.0f;

    // Clip to rounded rect so no sharp corner frame is visible
    g.saveState();
    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);
    g.reduceClipRegion(clipPath);

    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, bounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    else
    {
        g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(bounds, cornerRadius);
        auto centre = bounds.getCentre();
        g.setColour(juce::Colour(0x60FFFFFF));
        g.drawLine(bounds.getX(), centre.y, bounds.getRight(), centre.y, 1.0f);
        g.drawLine(centre.x, bounds.getY(), centre.x, bounds.getBottom(), 1.0f);
    }

    // Keep dot in sync with parameter (preset load, automation)
    xValue = static_cast<float>(xSlider.getValue());
    yValue = static_cast<float>(ySlider.getValue());
    auto xy = getXYFromValue();
    float indX = bounds.getX() + xy.x * bounds.getWidth();
    float indY = bounds.getY() + xy.y * bounds.getHeight();
    const float dotRadius = 15.0f; // 30x30 px cursor circle
    g.setColour(juce::Colours::white);
    g.fillEllipse(indX - dotRadius, indY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    g.setColour(juce::Colour(0xFF2C2D31));
    g.drawEllipse(indX - dotRadius, indY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f, 1.0f);

    g.restoreState();
}

void XYPadComponent::resized()
{
    auto bounds = getLocalBounds();
    xSlider.setBounds(bounds);
    ySlider.setBounds(bounds);
    // Sync dot position from parameter state (e.g. after preset load or host automation)
    xValue = static_cast<float>(xSlider.getValue());
    yValue = static_cast<float>(ySlider.getValue());
}

void XYPadComponent::mouseDown(const juce::MouseEvent& e)
{
    updateXYFromMousePosition(e.getPosition());
}

void XYPadComponent::mouseDrag(const juce::MouseEvent& e)
{
    updateXYFromMousePosition(e.getPosition());
}

void XYPadComponent::updateXYFromMousePosition(juce::Point<int> position)
{
    auto bounds = getLocalBounds().toFloat();
    
    float x = juce::jlimit(0.0f, 1.0f, (position.x - bounds.getX()) / bounds.getWidth());
    float y = juce::jlimit(0.0f, 1.0f, (position.y - bounds.getY()) / bounds.getHeight());
    
    setXYValue(x, y);
}

juce::Point<float> XYPadComponent::getXYFromValue()
{
    return juce::Point<float>(xValue, yValue);
}

void XYPadComponent::setXYValue(float x, float y)
{
    xValue = juce::jlimit(0.0f, 1.0f, x);
    yValue = juce::jlimit(0.0f, 1.0f, y);
    
    xSlider.setValue(xValue, juce::sendNotificationSync);
    ySlider.setValue(yValue, juce::sendNotificationSync);
    
    repaint();
}
