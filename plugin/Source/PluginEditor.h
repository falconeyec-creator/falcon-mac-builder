#pragma once

#include "PluginProcessor.h"
#include "FalconLookAndFeel.h"

// Animated stereo peak meter fed from the processor's atomics
class StereoMeter : public juce::Component
{
public:
    void setLevels (float newL, float newR)
    {
        levelL = newL;
        levelR = newR;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float gap = 4.0f;
        const float barW = (bounds.getWidth() - gap) / 2.0f;

        drawBar (g, { bounds.getX(),              bounds.getY(), barW, bounds.getHeight() }, levelL);
        drawBar (g, { bounds.getX() + barW + gap, bounds.getY(), barW, bounds.getHeight() }, levelR);
    }

private:
    static float levelToProportion (float gain)
    {
        auto db = juce::Decibels::gainToDecibels (gain, -60.0f);
        return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    }

    void drawBar (juce::Graphics& g, juce::Rectangle<float> r, float level)
    {
        g.setColour (falcon::trackDark.withAlpha (0.7f));
        g.fillRoundedRectangle (r, 3.0f);

        auto prop = levelToProportion (level);
        if (prop <= 0.001f)
            return;

        auto fill = r.withTop (r.getBottom() - r.getHeight() * prop);

        juce::ColourGradient grad (juce::Colour (0xff35d47f), r.getX(), r.getBottom(),
                                   juce::Colour (0xffff5252), r.getX(), r.getY(), false);
        grad.addColour (0.72, falcon::accent);

        g.setGradientFill (grad);
        g.fillRoundedRectangle (fill, 3.0f);
    }

    float levelL = 0.0f, levelR = 0.0f;
};

class FalconDelayEditor : public juce::AudioProcessorEditor,
                          private juce::Timer
{
public:
    explicit FalconDelayEditor (FalconDelayProcessor&);
    ~FalconDelayEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob (juce::Slider&, const juce::String& paramID);

    FalconDelayProcessor& processor;
    FalconLookAndFeel lnf;

    juce::Slider timeSlider, feedbackSlider, lowCutSlider, highCutSlider, mixSlider, outputSlider;
    juce::ComboBox divisionBox;
    juce::ToggleButton syncButton { "SYNC" }, pingPongButton { "PING-PONG" };
    juce::HyperlinkButton websiteLink { "www.falconeyesl.com",
                                        juce::URL ("https://www.falconeyesl.com") };

    StereoMeter meter;
    std::unique_ptr<ActivationOverlay> activation;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::unique_ptr<ComboAttachment> divisionAttachment;
    std::unique_ptr<ButtonAttachment> syncAttachment, pingPongAttachment;

    // Smoothed meter levels (GUI-side decay)
    float meterL = 0.0f, meterR = 0.0f;
    float titleGlow = 0.0f;
    float animPhase = 0.0f;

    juce::Rectangle<int> timeCell; // shared cell for time knob / division combo

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FalconDelayEditor)
};
