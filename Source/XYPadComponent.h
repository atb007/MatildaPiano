#pragma once

#include <JuceHeader.h>

class XYPadComponent : public juce::Component
{
public:
    XYPadComponent(juce::AudioProcessorValueTreeState& apvts);
    ~XYPadComponent() override = default;

    void setBackgroundImage(const juce::Image& image) { backgroundImage = image; repaint(); }

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Image backgroundImage;
    
    juce::Slider xSlider;
    juce::Slider ySlider;
    
    juce::AudioProcessorValueTreeState::SliderAttachment xAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment yAttachment;
    
    float xValue = 0.5f;
    float yValue = 0.5f;
    
    void updateXYFromMousePosition(juce::Point<int> position);
    juce::Point<float> getXYFromValue();
    void setXYValue(float x, float y);
};
