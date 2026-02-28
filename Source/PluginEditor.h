#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ChickenHeadKnob.h"
#include "XYPadComponent.h"
#include "MatildaKeyboardComponent.h"
#include "DelayModule.h"

class MatildaPianoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MatildaPianoAudioProcessorEditor(MatildaPianoAudioProcessor&);
    ~MatildaPianoAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MatildaPianoAudioProcessor& audioProcessor;

    // Background: gradient + left panel (Figma spec). No single baked image.
    juce::Image leftPanelImage;
    juce::Image xyPadBackgroundImage;
    juce::Image keyboardImage;
    std::unique_ptr<juce::Drawable> grandPianoUnderline;

    // Fonts (Figma: Jacquard 24, Kode Mono, Inter). Fallback to system if not in Assets.
    juce::Font fontTitle;      // Jacquard 24, 60px - "Matilda"
    juce::Font fontVersion;    // Jacquard 24, 20px - "v1.0"
    juce::Font fontGrandPiano; // Kode Mono Bold, 16px - "GRAND PIANO"
    juce::Font fontBrackets;   // Jacquard 24, 20px - "<" ">"
    juce::Font fontLabels;     // 12px - knob labels (Inter or system sans)

    // Text labels (JUCE-rendered)
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    juce::Label reverbLabel;
    juce::Label masterVolLabel;
    
    // ADSR knobs
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;
    
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment decayAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment sustainAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment releaseAttachment;
    
    // FX knobs
    juce::Slider reverbSlider;
    juce::Slider delayTimeSlider;
    juce::Slider masterVolSlider;
    
    juce::AudioProcessorValueTreeState::SliderAttachment reverbAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment delayTimeAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment masterVolAttachment;
    
    // Labels
    juce::Label delayTimeLabel;
    
    // XY Pad
    std::unique_ptr<XYPadComponent> xyPad;
    
    // Keyboard (Figma-style keys via MatildaKeyboardComponent)
    MatildaKeyboardComponent keyboardComponent;
    
    // Look and feel
    std::unique_ptr<ChickenHeadKnobLookAndFeel> whiteKnobLookAndFeel;
    std::unique_ptr<ChickenHeadKnobLookAndFeel> greenKnobLookAndFeel;
    
    // UI dimensions (matching Figma design frame 4203:94317)
    static constexpr int editorWidth = 1074;
    static constexpr int editorHeight = 483;

    // Scale factor from current size to design size; used for dynamic font/layout interpolation
    float getFigmaScale() const;
    
    void updateDelayTimeLabel();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaPianoAudioProcessorEditor)
};
