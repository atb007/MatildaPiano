#include "MatildaKeyboardComponent.h"

namespace
{
    constexpr float kCornerRadius = 3.0f;
    constexpr float kWhiteKeyTopHighlightRatio = 0.25f;
    constexpr uint32_t kKeybedColour = 0xFF5190B3;   // Figma keybed blue-teal
    constexpr uint32_t kKeybedBevelColour = 0xFF6ba3c4;
    constexpr float kKeybedCornerRadius = 12.0f;
}

void MatildaKeyboardComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float radius = juce::jmin(kKeybedCornerRadius, bounds.getWidth() * 0.02f, bounds.getHeight() * 0.08f);
    g.setColour(juce::Colour(kKeybedColour));
    g.fillRoundedRectangle(bounds, radius);
    g.setColour(juce::Colour(kKeybedBevelColour));
    g.drawHorizontalLine(0, 0.0f, bounds.getWidth());
    MidiKeyboardComponent::paint(g);
}

void MatildaKeyboardComponent::drawWhiteNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                              bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    drawFigmaStyleWhiteKey(g, area, isDown, isOver, lineColour, textColour, midiNoteNumber);
}

void MatildaKeyboardComponent::drawBlackNote(int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                             bool isDown, bool isOver, juce::Colour noteFillColour)
{
    if (getOrientation() != horizontalKeyboard)
    {
        MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, noteFillColour);
        return;
    }
    drawFigmaStyleBlackKey(g, area, isDown, isOver, noteFillColour);
}

void MatildaKeyboardComponent::drawFigmaStyleWhiteKey(juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                                      juce::Colour lineColour, juce::Colour textColour, int midiNoteNumber)
{
    if (getOrientation() != horizontalKeyboard)
    {
        MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
        return;
    }

    const float w = area.getWidth();
    const float h = area.getHeight();
    const float radius = juce::jmin(kCornerRadius, w * 0.2f, h * 0.15f);

    juce::Path keyPath;
    keyPath.addRoundedRectangle(area.getX(), area.getY(), w, h, radius, radius, true, false, true, false);

    juce::Colour baseWhite(0xFFF0F0F0);   // Figma white keys
    juce::Colour topHighlight(0xFFe5e8ec); // Slight gradient for 3D
    juce::Colour keyOutline(0xFF2a6080);   // Darker line on keybed

    if (!isDown && !isOver)
    {
        juce::ColourGradient grad(topHighlight, area.getX(), area.getY(),
                                  baseWhite, area.getX(), area.getY() + h * kWhiteKeyTopHighlightRatio,
                                  false);
        g.setGradientFill(grad);
        g.fillPath(keyPath);
        g.setColour(keyOutline);
        g.strokePath(keyPath, juce::PathStrokeType(0.5f));
    }
    else
    {
        g.setColour(baseWhite);
        g.fillPath(keyPath);
        if (isDown)
            g.setColour(findColour(keyDownOverlayColourId));
        else
            g.setColour(findColour(mouseOverKeyOverlayColourId));
        g.fillPath(keyPath);
        g.setColour(keyOutline);
        g.strokePath(keyPath, juce::PathStrokeType(0.5f));
    }

    if (!lineColour.isTransparent())
    {
        g.setColour(lineColour);
        g.fillRect(area.getX(), area.getBottom() - 1.0f, w, 1.0f);
        g.fillRect(area.getX(), area.getY(), 1.0f, h);
    }

    auto text = getWhiteNoteText(midiNoteNumber);
    if (text.isNotEmpty())
    {
        g.setColour(textColour);
        g.setFont(juce::Font(juce::FontOptions(juce::jmin(12.0f, getKeyWidth() * 0.9f))).withHorizontalScale(0.8f));
        g.drawText(text, area.withTrimmedLeft(1.0f).withTrimmedBottom(2.0f), juce::Justification::centredBottom, false);
    }
}

void MatildaKeyboardComponent::drawFigmaStyleBlackKey(juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                                      juce::Colour baseColour)
{
    const float w = area.getWidth();
    const float h = area.getHeight();
    const float radius = juce::jmin(kCornerRadius, w * 0.3f, h * 0.12f);

    juce::Path keyPath;
    keyPath.addRoundedRectangle(area.getX(), area.getY(), w, h, radius, radius, true, false, true, false);

    juce::Colour fillCol = baseColour;
    if (fillCol.getPerceivedBrightness() > 0.5f)
        fillCol = juce::Colour(0xFF3D3D3D); // Figma black keys

    if (!isDown && !isOver)
    {
        g.setColour(fillCol);
        g.fillPath(keyPath);
        g.setColour(fillCol.brighter(0.15f));
        g.fillRect(area.reduced(w * 0.125f, 0).removeFromTop(h * 0.15f));
    }
    else
    {
        g.setColour(fillCol);
        g.fillPath(keyPath);
        if (isDown)
            g.setColour(findColour(keyDownOverlayColourId));
        else
            g.setColour(findColour(mouseOverKeyOverlayColourId));
        g.fillPath(keyPath);
    }
}
