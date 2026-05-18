#include "MatildaKeyboardComponent.h"

namespace
{
    constexpr float kCornerRadius = 3.0f;
    constexpr float kWhiteKeyTopHighlightRatio = 0.25f;
    constexpr uint32_t kKeybedColour = 0xFF5190B3;   // Figma keybed blue-teal
    constexpr uint32_t kKeybedBevelColour = 0xFF6ba3c4;
    constexpr float kKeybedCornerRadius = 12.0f;
    
    // Figma white key colors
    constexpr uint32_t kWhiteKeyIdle = 0xFFFFFFFF;        // Pure white (idle state)
    constexpr uint32_t kWhiteKeyHover = 0xFFF5F5F5;       // Very light gray (hover state)
    constexpr uint32_t kWhiteKeyPressed = 0xFF5190B3;     // Blue-teal (pressed state)
    constexpr uint32_t kWhiteKeyOutline = 0xFFDDDDDD;     // Light outline
    
    // Figma black key colors
    constexpr uint32_t kBlackKeyIdle = 0xFF2A2A2A;        // Dark gray (idle state)
    constexpr uint32_t kBlackKeyHover = 0xFF3D3D3D;       // Medium gray (hover state)
    constexpr uint32_t kBlackKeyPressed = 0xFF5190B3;     // Blue-teal (pressed state)
    constexpr uint32_t kBlackKeyHighlight = 0xFF4A4A4A;   // Top highlight
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
    const float w = area.getWidth();
    const float h = area.getHeight();
    const float radius = juce::jmin(kCornerRadius, w * 0.15f);

    // Create rounded rectangle path (rounded top corners only - piano key style)
    juce::Path keyPath;
    keyPath.addRoundedRectangle(area.getX(), area.getY(), w, h, 
                                radius, radius, 
                                true, true, false, false);

    // Select color based on state (Figma: Idle, Hover, Pressed)
    juce::Colour fillColor;
    if (isDown)
        fillColor = juce::Colour(kWhiteKeyPressed);  // Pressed: blue-teal
    else if (isOver)
        fillColor = juce::Colour(kWhiteKeyHover);    // Hover: very light gray
    else
        fillColor = juce::Colour(kWhiteKeyIdle);     // Idle: pure white

    // Fill the key with solid color
    g.setColour(fillColor);
    g.fillPath(keyPath);

    // Draw subtle outline for definition
    g.setColour(juce::Colour(kWhiteKeyOutline));
    g.strokePath(keyPath, juce::PathStrokeType(1.0f));

    // Draw note label at bottom
    auto text = getWhiteNoteText(midiNoteNumber);
    if (text.isNotEmpty())
    {
        g.setColour(juce::Colours::black.withAlpha(isDown ? 0.8f : 0.4f));
        g.setFont(juce::Font(juce::FontOptions(juce::jmin(11.0f, w * 0.85f))).withHorizontalScale(0.8f));
        g.drawText(text, area.reduced(2.0f).removeFromBottom(14.0f), 
                  juce::Justification::centred, false);
    }
}

void MatildaKeyboardComponent::drawFigmaStyleBlackKey(juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                                      juce::Colour baseColour)
{
    const float w = area.getWidth();
    const float h = area.getHeight();
    const float radius = juce::jmin(kCornerRadius, w * 0.25f);

    // Create rounded rectangle path (rounded bottom corners only - black key style)
    juce::Path keyPath;
    keyPath.addRoundedRectangle(area.getX(), area.getY(), w, h, 
                                radius, radius, 
                                false, false, true, true);

    // Select color based on state (Figma: Idle, Hover, Pressed)
    juce::Colour fillColor;
    if (isDown)
        fillColor = juce::Colour(kBlackKeyPressed);  // Pressed: blue-teal
    else if (isOver)
        fillColor = juce::Colour(kBlackKeyHover);    // Hover: medium gray
    else
        fillColor = juce::Colour(kBlackKeyIdle);     // Idle: dark gray

    // Fill the key
    g.setColour(fillColor);
    g.fillPath(keyPath);

    // Add subtle top highlight for 3D effect (only when not pressed)
    if (!isDown && h > 20.0f)
    {
        g.setColour(juce::Colour(kBlackKeyHighlight).withAlpha(0.25f));
        auto highlightArea = area.reduced(w * 0.15f, 0.0f).removeFromTop(h * 0.12f);
        g.fillRect(highlightArea);
    }
}
