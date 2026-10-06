#include "PluginEditor.h"

FalconDelayEditor::FalconDelayEditor (FalconDelayProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&lnf);

    setupKnob (timeSlider,     "timeMs");
    setupKnob (feedbackSlider, "feedback");
    setupKnob (lowCutSlider,   "lowCut");
    setupKnob (highCutSlider,  "highCut");
    setupKnob (mixSlider,      "mix");
    setupKnob (outputSlider,   "output");

    // Division combo shares the "TIME" cell with the free-time knob
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (
            processor.apvts.getParameter ("division")))
        divisionBox.addItemList (choice->choices, 1);

    addAndMakeVisible (divisionBox);
    divisionAttachment = std::make_unique<ComboAttachment> (processor.apvts, "division", divisionBox);

    addAndMakeVisible (syncButton);
    syncAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "sync", syncButton);

    addAndMakeVisible (pingPongButton);
    pingPongAttachment = std::make_unique<ButtonAttachment> (processor.apvts, "pingpong", pingPongButton);

    addAndMakeVisible (meter);

    websiteLink.setFont (juce::Font (juce::FontOptions (11.0f)), false,
                         juce::Justification::centredRight);
    websiteLink.setColour (juce::HyperlinkButton::textColourId,
                           falcon::accent.withAlpha (0.75f));
    addAndMakeVisible (websiteLink);

    // License gate: overlay covers the UI until the plugin is activated
    if (FALCON_LICENSE_DISABLED == 0 && ! processor.licensed.load())
    {
        activation = std::make_unique<ActivationOverlay> (processor.licenses,
            [this] { processor.licensed.store (true); });
        addAndMakeVisible (*activation);
    }

    setSize (720, 375);
    startTimerHz (30);
}

FalconDelayEditor::~FalconDelayEditor()
{
    setLookAndFeel (nullptr);
}

void FalconDelayEditor::setupKnob (juce::Slider& slider, const juce::String& paramID)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 82, 18);
    addAndMakeVisible (slider);
    sliderAttachments.push_back (
        std::make_unique<SliderAttachment> (processor.apvts, paramID, slider));
}

void FalconDelayEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background: deep vertical gradient with a subtle vignette
    falcon::paintBackground (g, bounds, animPhase);

    // Audio-reactive glow behind the title
    if (titleGlow > 0.01f)
    {
        juce::ColourGradient glow (falcon::accent.withAlpha (0.10f + 0.14f * titleGlow),
                                   130.0f, 30.0f,
                                   falcon::accent.withAlpha (0.0f), 380.0f, 30.0f, true);
        g.setGradientFill (glow);
        g.fillEllipse (0.0f, -40.0f, 480.0f, 140.0f);
    }

    // Title + brand (two-tone split of the product name)
    juce::Font titleFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.setFont (titleFont);
    const juce::String titleLeft ("FALCON"), titleRight ("DELAY");
    const int titleLeftW = juce::roundToInt (
        juce::GlyphArrangement::getStringWidth (titleFont, titleLeft));
    g.setColour (falcon::textBright);
    g.drawText (titleLeft, 24, 16, titleLeftW + 4, 30, juce::Justification::centredLeft);
    g.setColour (falcon::accent);
    g.drawText (titleRight, 24 + titleLeftW + 10, 16, getWidth() - titleLeftW - 44, 30,
                juce::Justification::centredLeft);

    g.setColour (falcon::textDim);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("FALCON EYE CORPORATION", getWidth() - 220, 20, 196, 14,
                juce::Justification::centredRight);

    // BPM readout (only meaningful while host is playing / provides tempo)
    const bool sync = processor.apvts.getRawParameterValue ("sync")->load() > 0.5f;
    if (sync)
    {
        g.setColour (falcon::accent.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        g.drawText (juce::String (processor.currentBpm.load(), 1) + " BPM",
                    getWidth() - 220, 36, 196, 14, juce::Justification::centredRight);
    }

    // Section divider
    g.setColour (falcon::trackDark);
    g.drawLine (20.0f, 62.0f, (float) getWidth() - 20.0f, 62.0f, 1.0f);

    // Knob captions
    auto caption = [&g] (juce::Rectangle<int> cell, const juce::String& text)
    {
        g.setColour (falcon::textDim);
        g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        g.drawText (text, cell.getX(), cell.getBottom() + 1, cell.getWidth(), 16,
                    juce::Justification::centred);
    };

    caption (timeCell, sync ? "DIVISION" : "TIME");
    caption (feedbackSlider.getBounds(), "FEEDBACK");
    caption (lowCutSlider.getBounds(),   "LOW CUT");
    caption (highCutSlider.getBounds(),  "HIGH CUT");
    caption (mixSlider.getBounds(),      "MIX");
    caption (outputSlider.getBounds(),   "OUTPUT");

    g.setColour (falcon::textDim);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("v1.0.0", 24, getHeight() - 24, 60, 14, juce::Justification::centredLeft);
}

void FalconDelayEditor::resized()
{
    const int knobY = 100;
    const int knobH = 150;
    const int knobW = 104;
    const int startX = 20;

    timeCell = { startX, knobY, knobW, knobH };
    timeSlider.setBounds (timeCell);

    // Division combo centred inside the same cell
    divisionBox.setBounds (timeCell.withSizeKeepingCentre (92, 34));

    feedbackSlider.setBounds (startX + knobW * 1, knobY, knobW, knobH);
    lowCutSlider  .setBounds (startX + knobW * 2, knobY, knobW, knobH);
    highCutSlider .setBounds (startX + knobW * 3, knobY, knobW, knobH);
    mixSlider     .setBounds (startX + knobW * 4, knobY, knobW, knobH);
    outputSlider  .setBounds (startX + knobW * 5, knobY, knobW, knobH);

    meter.setBounds (getWidth() - 60, knobY, 36, knobH + 60);

    syncButton.setBounds     (startX + 8,  292, 150, 40);
    pingPongButton.setBounds (startX + 208, 292, 180, 40);

    websiteLink.setBounds (getWidth() - 220, getHeight() - 26, 196, 16);

    if (activation != nullptr)
        activation->setBounds (getLocalBounds());
}

void FalconDelayEditor::timerCallback()
{
    // Pull peaks from the audio thread, apply GUI-side decay
    const float newL = processor.peakL.exchange (0.0f);
    const float newR = processor.peakR.exchange (0.0f);

    meterL = juce::jmax (newL, meterL * 0.82f);
    meterR = juce::jmax (newR, meterR * 0.82f);
    meter.setLevels (meterL, meterR);

    titleGlow = juce::jmax (juce::jmax (meterL, meterR), titleGlow * 0.9f);

    // Swap TIME knob <-> DIVISION combo based on sync state
    const bool sync = processor.apvts.getRawParameterValue ("sync")->load() > 0.5f;
    timeSlider.setVisible (! sync);
    divisionBox.setVisible (sync);

    if (falcon::bgAnimated())
    {
        animPhase += 0.033f;
        repaint();
    }
    else
        repaint (0, 0, getWidth(), 64);          // header (glow + BPM)
    repaint (timeCell.getX(), timeCell.getY() - 20, timeCell.getWidth(), 20); // caption swap
}
