#pragma once

#include <JuceHeader.h>

/** Piano keyboard with Figma-style keys: rounded top corners, subtle gradients.
 *  Replaces default JUCE key drawing so keys are visible and match the design
 *  (node 4301:138096 — sample Keys). Keybed background is drawn by the parent
 *  (keyboard.png); this component draws only the keys.
 */
class MatildaKeyboardComponent : public juce::MidiKeyboardComponent
{
public:
    using MidiKeyboardComponent::MidiKeyboardComponent;

    void paint(juce::Graphics& g) override;

    void drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                       bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;
    void drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                      bool isDown, bool isOver, juce::Colour noteFillColour) override;

private:
    void drawFigmaStyleWhiteKey(juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                juce::Colour lineColour, juce::Colour textColour, int midiNoteNumber);
    void drawFigmaStyleBlackKey(juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                juce::Colour baseColour);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaKeyboardComponent)
};
