#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "DelayModule.h"

#if MATILDA_HAS_BINARY_DATA
  #include "BinaryData.h"
#endif

namespace
{
    // Assets are loaded from: (1) BinaryData, (2) app bundle Resources (Standalone only),
    // (3) ~/Documents/MatildaPiano/Assets, (4) project cwd/Assets.
    // When running as Standalone, currentApplicationFile is the .app bundle; we look in Contents/Resources/Assets.
    // For AU in a host (e.g. GarageBand), currentApplicationFile is the host app, so bundle path is skipped.
    juce::File assetsRoot()
    {
        auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        // Standalone: currentApplicationFile is usually the .app bundle on macOS; sometimes the executable inside it.
        juce::File bundleRoot = appFile;
        if (!appFile.getFileName().endsWithIgnoreCase(".app"))
        {
            // Executable is at .../Contents/MacOS/Matilda Piano; bundle is two levels up.
            auto contents = appFile.getParentDirectory();
            if (contents.getFileName() == "MacOS")
                bundleRoot = contents.getParentDirectory().getParentDirectory();
        }
        if (bundleRoot.getFileName().endsWithIgnoreCase(".app"))
        {
            auto bundleAssets = bundleRoot.getChildFile("Contents").getChildFile("Resources").getChildFile("Assets");
            if (bundleAssets.exists())
                return bundleAssets;
        }
        auto userAssets = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                              .getChildFile("MatildaPiano").getChildFile("Assets");
        if (userAssets.exists())
            return userAssets;
        return userAssets;
    }

    juce::Image loadImageFromBinaryData(const void* data, int size)
    {
        if (data == nullptr || size <= 0) return {};
        juce::MemoryInputStream mem(data, static_cast<size_t>(size), false);
        return juce::ImageFileFormat::loadFrom(mem);
    }

    juce::Image loadLeftPanelImage()
    {
#if MATILDA_HAS_BINARY_DATA
        int size = 0;
        const char* names[] = { "background_left_png", "background-left_png", "Images_background_left_png" };
        for (const char* name : names)
        {
            const void* data = BinaryData::getNamedResource(name, size);
            if (data != nullptr && size > 0)
                return loadImageFromBinaryData(data, size);
        }
#endif
        auto tryLoad = [](const juce::File& f) -> juce::Image {
            if (f.existsAsFile()) return juce::ImageFileFormat::loadFrom(f); return {}; };
        // User folder
        auto file = assetsRoot().getChildFile("Images").getChildFile("background-left.png");
        if (auto img = tryLoad(file); img.isValid()) return img;
        // Project Assets (when run from project root, e.g. dev)
        file = juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Images").getChildFile("background-left.png");
        if (auto img = tryLoad(file); img.isValid()) return img;
        return {};
    }

    juce::Image loadXyPadImage()
    {
#if MATILDA_HAS_BINARY_DATA
        int size = 0;
        const void* data = BinaryData::getNamedResource("xy_pad_png", size);
        if (data != nullptr && size > 0)
            return loadImageFromBinaryData(data, size);
        data = BinaryData::getNamedResource("xy-pad_png", size);
        if (data != nullptr && size > 0)
            return loadImageFromBinaryData(data, size);
#endif
        auto file = assetsRoot().getChildFile("Images").getChildFile("xy-pad.png");
        if (file.existsAsFile())
            return juce::ImageFileFormat::loadFrom(file);
        return {};
    }

    juce::Image loadKeyboardImage()
    {
#if MATILDA_HAS_BINARY_DATA
        int size = 0;
        const char* names[] = { "keyboard_png", "Images_keyboard_png" };
        for (const char* name : names)
        {
            const void* data = BinaryData::getNamedResource(name, size);
            if (data != nullptr && size > 0)
                return loadImageFromBinaryData(data, size);
        }
#endif
        auto tryLoad = [](const juce::File& f) -> juce::Image {
            if (f.existsAsFile()) return juce::ImageFileFormat::loadFrom(f); return {}; };
        juce::Image img = tryLoad(assetsRoot().getChildFile("Images").getChildFile("keyboard.png"));
        if (img.isValid()) return img;
        return tryLoad(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Images").getChildFile("keyboard.png"));
    }

    std::unique_ptr<juce::Drawable> loadGrandPianoUnderlineDrawable()
    {
        auto loadFromFile = [](const juce::File& file) -> std::unique_ptr<juce::Drawable> {
            if (!file.existsAsFile()) return nullptr;
            juce::String svgText = file.loadFileAsString();
            if (svgText.isEmpty()) return nullptr;
            auto xml = juce::parseXML(svgText);
            if (xml == nullptr) return nullptr;
            return juce::Drawable::createFromSVG(*xml);
        };
#if MATILDA_HAS_BINARY_DATA
        int size = 0;
        const char* names[] = { "grand_piano_underline_svg", "Images_grand_piano_underline_svg", "grandpiano_underline_svg" };
        for (const char* name : names)
        {
            const void* data = BinaryData::getNamedResource(name, size);
            if (data != nullptr && size > 0)
            {
                juce::String svgText(static_cast<const char*>(data), static_cast<size_t>(size));
                auto xml = juce::parseXML(svgText);
                if (xml != nullptr)
                    return juce::Drawable::createFromSVG(*xml);
            }
        }
#endif
        auto drawable = loadFromFile(assetsRoot().getChildFile("Images").getChildFile("grand-piano-underline.svg"));
        if (drawable != nullptr) return drawable;
        return loadFromFile(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Images").getChildFile("grand-piano-underline.svg"));
    }

    juce::Font loadFontFromData(const void* data, size_t size, float height)
    {
        if (data == nullptr || size == 0) return juce::Font(juce::FontOptions(height));
        auto typeface = juce::Typeface::createSystemTypefaceFor(data, size);
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wdeprecated-declarations")
        auto font = juce::Font(typeface).withHeight(height);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        return font;
    }

    juce::Font loadFontFromFile(const juce::File& file, float height)
    {
        if (!file.existsAsFile()) return juce::Font(juce::FontOptions(height));
        juce::FileInputStream is(file);
        if (!is.openedOk()) return juce::Font(juce::FontOptions(height));
        juce::MemoryBlock block;
        if (is.readIntoMemoryBlock(block) == 0) return juce::Font(juce::FontOptions(height));
        return loadFontFromData(block.getData(), block.getSize(), height);
    }

    void drawInnerShadowApprox(juce::Graphics& g, int w, int h, float /*cornerRadius*/)
    {
        // Subtle inner shadow (softer than Figma spec to avoid extreme edges)
        juce::Colour shadowCol(0x18000000); // ~0.09 alpha
        const int band = 6;
        g.setColour(shadowCol);
        g.fillRect(0, 0, w, band);
        g.fillRect(0, 0, band, h);
        g.fillRect(w - band, 0, band, h);
    }
}

MatildaPianoAudioProcessorEditor::MatildaPianoAudioProcessorEditor(MatildaPianoAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      attackAttachment(p.getValueTreeState(), Parameters::ATTACK, attackSlider),
      decayAttachment(p.getValueTreeState(), Parameters::DECAY, decaySlider),
      sustainAttachment(p.getValueTreeState(), Parameters::SUSTAIN, sustainSlider),
      releaseAttachment(p.getValueTreeState(), Parameters::RELEASE, releaseSlider),
      reverbAttachment(p.getValueTreeState(), Parameters::REVERB, reverbSlider),
      delayTimeAttachment(p.getValueTreeState(), Parameters::DELAY_TIME, delayTimeSlider),
      masterVolAttachment(p.getValueTreeState(), Parameters::MASTER_VOL, masterVolSlider),
      keyboardComponent(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Create look and feel objects
    whiteKnobLookAndFeel = std::make_unique<ChickenHeadKnobLookAndFeel>(true);
    greenKnobLookAndFeel = std::make_unique<ChickenHeadKnobLookAndFeel>(false);
    
    // ADSR knobs: chicken-head at zero = 7 o'clock (Figma). JUCE "clockwise from top": 210° = min, +270° = max.
    const float rotaryStartRad = 210.0f * juce::MathConstants<float>::pi / 180.0f;
    const float rotaryEndRad   = rotaryStartRad + 270.0f * juce::MathConstants<float>::pi / 180.0f;
    for (auto* slider : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider })
    {
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setRotaryParameters(rotaryStartRad, rotaryEndRad, true);
        slider->setLookAndFeel(whiteKnobLookAndFeel.get());
        addAndMakeVisible(slider);
    }
    
    attackSlider.setRange(Parameters::ATTACK_MIN, Parameters::ATTACK_MAX, 0.01);
    decaySlider.setRange(Parameters::DECAY_MIN, Parameters::DECAY_MAX, 0.01);
    sustainSlider.setRange(Parameters::SUSTAIN_MIN, Parameters::SUSTAIN_MAX, 0.01);
    releaseSlider.setRange(Parameters::RELEASE_MIN, Parameters::RELEASE_MAX, 0.01);
    
    // FX knobs: same zero at 7 o'clock (chicken-head aligned to minimum)
    for (auto* slider : { &reverbSlider, &delayTimeSlider, &masterVolSlider })
    {
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setRotaryParameters(rotaryStartRad, rotaryEndRad, true);
        slider->setLookAndFeel(greenKnobLookAndFeel.get());
        addAndMakeVisible(slider);
    }
    
    reverbSlider.setRange(Parameters::REVERB_MIN, Parameters::REVERB_MAX, 0.01);
    delayTimeSlider.setRange(Parameters::DELAY_TIME_MIN, Parameters::DELAY_TIME_MAX, 0.01);
    masterVolSlider.setRange(Parameters::MASTER_VOL_MIN, Parameters::MASTER_VOL_MAX, 0.01);
    
    auto initSmallLabel = [&](juce::Label& lbl, const juce::String& text, bool uppercase = false)
    {
        lbl.setText(uppercase ? text.toUpperCase() : text, juce::dontSendNotification);
        lbl.setColour(juce::Label::textColourId, juce::Colours::white);
        lbl.setJustificationType(juce::Justification::centred);
        lbl.setFont(juce::Font(juce::FontOptions(12.0f)));
        lbl.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(lbl);
    };

    // Knob labels (match Figma text nodes)
    initSmallLabel(attackLabel, "Attack");
    initSmallLabel(decayLabel, "Decay");
    initSmallLabel(sustainLabel, "Sustain");
    initSmallLabel(releaseLabel, "Release");
    initSmallLabel(reverbLabel, "Reverb");
    initSmallLabel(masterVolLabel, "Master\nvol."); // Two lines like Delay label

    // Delay time label
    delayTimeLabel.setText("Delay\n(1/4)", juce::dontSendNotification);
    delayTimeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    delayTimeLabel.setFont(juce::Font(juce::FontOptions(12.0f)));
    delayTimeLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(delayTimeLabel);
    
    // Update delay time label when slider changes
    delayTimeSlider.onValueChange = [this] { updateDelayTimeLabel(); };
    
    // XY Pad
    xyPad = std::make_unique<XYPadComponent>(p.getValueTreeState());
    addAndMakeVisible(xyPad.get());

    // Keyboard: C0–C7 (MIDI 12–96). Full width and design colours (white / black keys).
    keyboardComponent.setAvailableRange(12, 96);
    keyboardComponent.setLowestVisibleKey(12);
    keyboardComponent.setOctaveForMiddleC(4);
    // Keybed background: transparent so parent can draw keyboard.png; keys drawn by MatildaKeyboardComponent (Figma style).
    keyboardComponent.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colours::transparentBlack);
    keyboardComponent.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour(0xFF3D3D3D)); // Figma black keys
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour(0xFF2a6080)); // Darker line on keybed
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour(0x55ffffff));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour(0x22ffffff));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::textLabelColourId, juce::Colours::white);
    addAndMakeVisible(keyboardComponent);

    // Background and assets: gradient + left panel, XY pad, keyboard image, grand piano underline
    leftPanelImage = loadLeftPanelImage();
    xyPadBackgroundImage = loadXyPadImage();
    keyboardImage = loadKeyboardImage();
    grandPianoUnderline = loadGrandPianoUnderlineDrawable();
    if (xyPadBackgroundImage.isValid())
        xyPad->setBackgroundImage(xyPadBackgroundImage);

    // Fonts: Figma Jacquard 24, Kode Mono, Inter (or system fallback). Try BinaryData then Assets.
    auto tryFontBinary = [](const char* names[], int numNames, float height) -> juce::Font {
#if MATILDA_HAS_BINARY_DATA
        for (int i = 0; i < numNames; ++i)
        {
            int size = 0;
            const void* data = BinaryData::getNamedResource(names[i], size);
            if (data && size > 0)
                return loadFontFromData(data, static_cast<size_t>(size), height);
        }
#endif
        return juce::Font(juce::FontOptions(height));
    };
    // BinaryData names match juce_add_binary_data mangling: "Jacquard24-Regular.ttf" -> Jacquard24Regular_ttf, etc.
    const char* jacquardNames[] = { "Jacquard24Regular_ttf", "Jacquard24_Regular_ttf", "Jacquard_24_Jacquard24_Regular_ttf" };
    const char* kodeNames[] = { "KodeMonoVariableFont_wght_ttf", "KodeMono_Bold_ttf", "KodeMono_VariableFont_wght_ttf", "static_KodeMono_Bold_ttf" };

    // Jacquard 24: try BinaryData, then nested path (Jacquard_24/), then flat (Fonts/ root) for repo/bundle layout.
    auto tryJacquard = [&](float h) -> juce::Font {
        juce::Font f = tryFontBinary(jacquardNames, 3, h);
        if (!f.getTypefaceName().isEmpty()) return f;
        auto roots = { assetsRoot(), juce::File::getCurrentWorkingDirectory().getChildFile("Assets") };
        for (const auto& root : roots)
        {
            f = loadFontFromFile(root.getChildFile("Fonts").getChildFile("Jacquard_24").getChildFile("Jacquard24-Regular.ttf"), h);
            if (!f.getTypefaceName().isEmpty()) return f;
            f = loadFontFromFile(root.getChildFile("Fonts").getChildFile("Jacquard24-Regular.ttf"), h);
            if (!f.getTypefaceName().isEmpty()) return f;
        }
        return juce::Font(juce::FontOptions(h));
    };
    fontTitle = tryJacquard(60.0f);
    fontVersion = tryJacquard(20.0f);
    fontBrackets = fontVersion;

    // Kode Mono: try BinaryData, then Kode_Mono/static/KodeMono-Bold.ttf, then flat VariableFont or Kode_Mono/ variable font.
    auto tryKodeMono = [&](float h) -> juce::Font {
        juce::Font f = tryFontBinary(kodeNames, 4, h);
        if (!f.getTypefaceName().isEmpty()) return f;
        auto roots = { assetsRoot(), juce::File::getCurrentWorkingDirectory().getChildFile("Assets") };
        const char* paths[] = {
            "Fonts/Kode_Mono/static/KodeMono-Bold.ttf",
            "Fonts/Kode_Mono/KodeMono-VariableFont_wght.ttf",
            "Fonts/KodeMono-VariableFont_wght.ttf"
        };
        for (const auto& root : roots)
            for (const char* p : paths)
            {
                f = loadFontFromFile(root.getChildFile(p), h);
                if (!f.getTypefaceName().isEmpty()) return f;
            }
        return juce::Font(juce::FontOptions(h));
    };
    fontGrandPiano = tryKodeMono(16.0f);

    const char* interNames[] = { "Inter_Regular_ttf", "Inter_ttf" };
    fontLabels = tryFontBinary(interNames, 2, 12.0f);
    if (fontLabels.getTypefaceName().isEmpty())
        fontLabels = loadFontFromFile(assetsRoot().getChildFile("Fonts").getChildFile("Inter").getChildFile("Inter-Regular.ttf"), 12.0f);
    if (fontLabels.getTypefaceName().isEmpty())
        fontLabels = juce::Font(juce::FontOptions(12.0f));

    for (auto* lbl : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel,
                       &reverbLabel, &masterVolLabel, &delayTimeLabel })
        lbl->setFont(fontLabels);

    setSize(editorWidth, editorHeight);
}

float MatildaPianoAudioProcessorEditor::getFigmaScale() const
{
    const float w = static_cast<float>(getWidth());
    const float h = static_cast<float>(getHeight());
    if (w <= 0 || h <= 0) return 1.0f;
    return juce::jmin(w / static_cast<float>(editorWidth), h / static_cast<float>(editorHeight));
}

MatildaPianoAudioProcessorEditor::~MatildaPianoAudioProcessorEditor()
{
    // Remove look and feel before destruction
    for (auto* slider : { &attackSlider, &decaySlider, &sustainSlider, &releaseSlider })
    {
        slider->setLookAndFeel(nullptr);
    }
    
    for (auto* slider : { &reverbSlider, &delayTimeSlider, &masterVolSlider })
    {
        slider->setLookAndFeel(nullptr);
    }
}

void MatildaPianoAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const float cornerRadius = 50.0f;
    const int w = getWidth();
    const int h = getHeight();

    g.saveState();
    g.reduceClipRegion(juce::Rectangle<int>(0, 0, w, h));
    {
        juce::Path clipPath;
        clipPath.addRoundedRectangle(bounds, cornerRadius);
        g.reduceClipRegion(clipPath);

        // 1) Figma gradient: #1c1f45 (top) -> #2b477d @ 46.154% -> #61b2bf (bottom)
        juce::ColourGradient gradient(
            juce::Colour(0xFF1c1f45), 0.0f, 0.0f,
            juce::Colour(0xFF61b2bf), 0.0f, bounds.getHeight(),
            false
        );
        gradient.addColour(0.46154f, juce::Colour(0xFF2b477d));
        g.setGradientFill(gradient);
        g.fillAll();

        // 2) Left panel image (Figma "pic": -3, 0, 409×483) — scaled to current size
        if (leftPanelImage.isValid())
        {
            const float scaleX = bounds.getWidth() / static_cast<float>(editorWidth);
            const float scaleY = bounds.getHeight() / static_cast<float>(editorHeight);
            const float panelW = 409.0f * scaleX;
            const float panelH = bounds.getHeight();
            g.drawImage(leftPanelImage, -3.0f * scaleX, 0.0f, panelW, panelH, 0, 0, leftPanelImage.getWidth(), leftPanelImage.getHeight());
        }

        // 2b) Keyboard bed (Figma 4203:94361): visible keybed first (#5190B3), then optional keyboard.png
        const float keyboardY = 364.0f * (bounds.getHeight() / static_cast<float>(editorHeight));
        const float keyboardH = bounds.getHeight() - keyboardY;
        const float keybedRadius = 12.0f * (bounds.getHeight() / static_cast<float>(editorHeight));
        const juce::Rectangle<float> keybedRect(0.0f, keyboardY, bounds.getWidth(), keyboardH);
        g.setColour(juce::Colour(0xFF5190B3)); // Figma keybed blue-teal
        g.fillRoundedRectangle(keybedRect, keybedRadius);
        g.setColour(juce::Colour(0xFF6ba3c4)); // Lighter inner bevel at top
        g.drawHorizontalLine(static_cast<int>(keyboardY), keybedRect.getX(), keybedRect.getRight());
        if (keyboardImage.isValid())
            g.drawImage(keyboardImage, 0, static_cast<int>(keyboardY), getWidth(), static_cast<int>(keyboardH), 0, 0, keyboardImage.getWidth(), keyboardImage.getHeight());

        // 3) Inner shadow approximation
        drawInnerShadowApprox(g, w, h, cornerRadius);
    }
    g.restoreState();

    // Font sizes from Figma (60, 20, 16, 12, 11) scaled by design interpolation
    const float scale = getFigmaScale();

    // 4) Top-right label: "Matilda" (60px), "v1.0" (20px) — Figma Jacquard 24
    g.setColour(juce::Colours::white);
    g.setFont(fontTitle.withHeight(60.0f * scale));
    g.drawText("Matilda", 853.0f * scale, 15.0f * scale, 160.0f * scale, 42.0f * scale, juce::Justification::centredRight);
    g.setFont(fontVersion.withHeight(20.0f * scale));
    g.drawText("v1.0", 853.0f * scale, 55.0f * scale, 160.0f * scale, 22.0f * scale, juce::Justification::centredRight);

    // 5) Grand Piano: decorative underline SVG behind label (5% bigger, stretched wider), then "<" "GRAND PIANO" ">"
    const float centreX = getWidth() / 2.0f;
    const float labelCentreY = 32.0f * scale + 11.0f;
    const float sizeScale = 1.05f;        // 5% bigger than original
    const float stretchWidth = 1.35f;     // Stretch SVG horizontally behind the label
    const float underlineW = 180.0f * scale * sizeScale * stretchWidth;
    const float underlineH = 28.0f * scale * sizeScale;
    if (grandPianoUnderline != nullptr)
    {
        grandPianoUnderline->setTransformToFit(juce::Rectangle<float>(centreX - underlineW * 0.5f, labelCentreY - 4.0f * scale, underlineW, underlineH), juce::RectanglePlacement::centred);
        grandPianoUnderline->draw(g, 1.0f);
    }
    g.setFont(fontBrackets.withHeight(20.0f * scale));
    g.drawText("<", (centreX - 90.0f * scale), 30.0f * scale, 20.0f * scale, 26.0f * scale, juce::Justification::centredRight);
    g.drawText(">", (centreX + 70.0f * scale), 30.0f * scale, 20.0f * scale, 26.0f * scale, juce::Justification::centredLeft);
    g.setFont(fontGrandPiano.withHeight(16.0f * scale));
    g.drawText("GRAND PIANO", (centreX - 70.0f * scale), 32.0f * scale, 140.0f * scale, 22.0f * scale, juce::Justification::centred);

    // 6) Sample load status (11px)
    auto status = audioProcessor.getSampleLoadStatus();
    if (status.isNotEmpty())
    {
        g.setColour(juce::Colour(0xFFffaa00));
        g.setFont(juce::Font(juce::FontOptions(11.0f * scale)));
        g.drawText(status, 20.0f * scale, (getHeight() - 22.0f * scale), getWidth() - 40.0f * scale, 18.0f * scale, juce::Justification::centredLeft, true);
    }
}

void MatildaPianoAudioProcessorEditor::resized()
{
    const float scale = getFigmaScale();
    for (auto* lbl : { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel,
                       &reverbLabel, &masterVolLabel, &delayTimeLabel })
        lbl->setFont(fontLabels.withHeight(12.0f * scale));

    // Figma frame `4203:94317` (1074x483) — positions/sizes scaled by getFigmaScale()
    const int knobSize = juce::jmax(1, juce::roundToInt(51.0f * scale));
    const int groupX = juce::roundToInt(467.0f * scale);
    const int groupY = juce::roundToInt(124.0f * scale);
    const int dx81 = juce::roundToInt(81.0f * scale);
    const int dx162 = juce::roundToInt(162.0f * scale);
    const int dx243 = juce::roundToInt(243.0f * scale);
    const int rowGap = juce::roundToInt(97.0f * scale);
    const int labelY = juce::roundToInt(62.0f * scale);
    const int labelH = juce::jmax(1, juce::roundToInt(15.0f * scale));
    const int labelH30 = juce::jmax(1, juce::roundToInt(30.0f * scale));

    // ADSR row: knob x offsets 0, 81, 162, 243
    attackSlider.setBounds(groupX + 0,        groupY + 0, knobSize, knobSize);
    decaySlider.setBounds(groupX + dx81,      groupY + 0, knobSize, knobSize);
    sustainSlider.setBounds(groupX + dx162,  groupY + 0, knobSize, knobSize);
    releaseSlider.setBounds(groupX + dx243,   groupY + 0, knobSize, knobSize);

    attackLabel.setBounds(groupX + 0,      groupY + labelY, knobSize, labelH);
    decayLabel.setBounds(groupX + dx81,    groupY + labelY, knobSize, labelH);
    sustainLabel.setBounds(groupX + dx162, groupY + labelY, knobSize, labelH);
    releaseLabel.setBounds(groupX + dx243, groupY + labelY, knobSize, labelH);

    // FX row: y += 97
    const int fxY = groupY + rowGap;
    reverbSlider.setBounds(groupX + 0,     fxY, knobSize, knobSize);
    delayTimeSlider.setBounds(groupX + dx81, fxY, knobSize, knobSize);
    masterVolSlider.setBounds(groupX + dx162, fxY, knobSize, knobSize);

    reverbLabel.setBounds(groupX + 0,     fxY + labelY, knobSize, labelH);
    delayTimeLabel.setBounds(groupX + dx81, fxY + labelY, knobSize, labelH30);
    masterVolLabel.setBounds(groupX + dx162, fxY + labelY, knobSize, labelH30);

    // XY pad: Figma 834, 119, 198, 192
    xyPad->setBounds(juce::roundToInt(834.0f * scale), juce::roundToInt(119.0f * scale),
                     juce::roundToInt(198.0f * scale), juce::roundToInt(192.0f * scale));

    // Keyboard: keybed starts at Figma y=364
    const int keyboardY = juce::roundToInt(364.0f * scale);
    const int keyboardH = juce::jmax(1, getHeight() - keyboardY);
    keyboardComponent.setBounds(0, keyboardY, getWidth(), keyboardH);
    const int numWhiteKeys = 50;
    keyboardComponent.setKeyWidth(static_cast<float>(getWidth()) / static_cast<float>(numWhiteKeys));
}

void MatildaPianoAudioProcessorEditor::updateDelayTimeLabel()
{
    float delayTime = delayTimeSlider.getValue();
    const float offThreshold = 0.05f;
    juce::String display;
    if (delayTime <= offThreshold)
        display = "Off";
    else
    {
        float t = (delayTime - offThreshold) / (1.0f - offThreshold);
        const char* subdivisions[] = { "1/64", "1/32", "1/16", "1/8", "1/8T", "1/4", "1/4T", "1/2", "1" };
        int index = juce::jlimit(0, 8, static_cast<int>(t * 8.99f));
        display = subdivisions[index];
    }
    delayTimeLabel.setText("Delay\n(" + display + ")", juce::dontSendNotification);
}
