#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BgImage.h"

// Style switches supplied by the Maker at generation time
#ifndef FALCON_KNOB_STYLE
 #define FALCON_KNOB_STYLE 0   // index into falcon::knobSpecs (30 styles)
#endif
#ifndef FALCON_BG_THEME
 #define FALCON_BG_THEME 0     // index into falcon::bgSpecs (30 themes)
#endif

namespace falcon
{
    // Falcon Eye Corporation palette
    const juce::Colour background   { 0xff10141c };
    const juce::Colour panel        { 0xff161a22 };
    const juce::Colour panelLight   { 0xff1e222a };
    const juce::Colour accent       { 0xffffb13b };
    const juce::Colour accentHot    { 0xffff7847 };
    const juce::Colour textBright   { 0xffe8ecf4 };
    const juce::Colour textDim      { 0xff7a8699 };
    const juce::Colour trackDark    { 0xff23272f };

    //==========================================================================
    //  KNOB STYLE ENGINE — every knob is described by a spec; 30 curated
    //  presets below. Order must match the Maker UI list.
    //==========================================================================
    struct KnobSpec
    {
        float trackW;    // arc stroke width
        bool  fullRing;  // closed ring outline instead of open track arc
        int   glow;      // halo layers under the value arc (0..3)
        bool  gradArc;   // gradient accent->accentHot arc vs solid accent
        int   cap;       // 0 none, 1 flat, 2 radial-gradient, 3 knurled, 4 dark inset, 5 dome
        float capRatio;  // cap radius relative to knob radius
        int   pointer;   // 0 rounded bar, 1 thin line, 2 tip dot, 3 needle, 4 edge dot, 5 none
        bool  ticks;     // outer tick marks
    };

    static const KnobSpec knobSpecs[30] =
    {
        //  trkW  ring   glow  grad   cap  capR   ptr  ticks
        {   5.0f, false, 2,    true,  2,   0.62f, 0,   false }, //  0 Falcon Glow
        {   3.0f, false, 0,    false, 1,   0.66f, 1,   false }, //  1 Minimal Flat
        {   2.8f, true,  2,    false, 4,   0.50f, 2,   false }, //  2 Neon Ring
        {   4.0f, false, 0,    false, 2,   0.60f, 3,   true  }, //  3 Vintage Deck
        {   6.5f, false, 3,    true,  4,   0.52f, 2,   false }, //  4 LED Halo
        {   3.5f, false, 0,    false, 3,   0.66f, 0,   true  }, //  5 Knurled Steel
        {   4.5f, false, 1,    true,  5,   0.64f, 0,   false }, //  6 Soft Dome
        {   2.2f, false, 0,    false, 1,   0.72f, 4,   false }, //  7 Dot Minimal
        {   8.0f, false, 0,    true,  0,   0.00f, 5,   false }, //  8 Fat Arc
        {   3.0f, true,  1,    false, 2,   0.58f, 3,   true  }, //  9 Pilot Gauge
        {   5.5f, false, 2,    false, 4,   0.55f, 2,   true  }, // 10 Night Console
        {   4.0f, false, 0,    true,  2,   0.62f, 1,   false }, // 11 Studio Classic
        {   2.5f, true,  3,    true,  0,   0.00f, 2,   false }, // 12 Plasma Ring
        {   3.5f, false, 1,    false, 5,   0.68f, 3,   false }, // 13 Cream Dome
        {   6.0f, false, 0,    false, 1,   0.58f, 0,   true  }, // 14 Industrial
        {   4.5f, true,  2,    true,  4,   0.46f, 2,   false }, // 15 Cyber Halo
        {   3.0f, false, 0,    false, 2,   0.70f, 3,   true  }, // 16 Hi-Fi Amp
        {   5.0f, false, 3,    true,  0,   0.00f, 2,   false }, // 17 Pure Glow
        {   2.5f, false, 0,    false, 4,   0.66f, 1,   false }, // 18 Shadow Flat
        {   4.0f, true,  1,    true,  3,   0.56f, 0,   true  }, // 19 Machined Ring
        {   3.5f, false, 2,    false, 2,   0.60f, 4,   false }, // 20 Ember Dot
        {   7.0f, false, 1,    true,  1,   0.48f, 5,   true  }, // 21 Big Meter
        {   2.8f, true,  0,    false, 5,   0.62f, 3,   false }, // 22 Porcelain
        {   4.5f, false, 2,    true,  3,   0.60f, 2,   true  }, // 23 Racing Dial
        {   3.0f, false, 1,    false, 4,   0.58f, 0,   false }, // 24 Dark Matter
        {   5.5f, true,  3,    true,  0,   0.00f, 5,   false }, // 25 Halo Only
        {   4.0f, false, 0,    false, 5,   0.64f, 1,   true  }, // 26 Broadcast
        {   3.5f, true,  2,    false, 1,   0.54f, 4,   false }, // 27 Orbit Dot
        {   6.0f, false, 2,    true,  2,   0.56f, 3,   true  }, // 28 Command Deck
        {   2.2f, false, 3,    false, 0,   0.00f, 2,   false }, // 29 Wisp
    };

    //==========================================================================
    //  BACKGROUND THEME ENGINE — base fill × pattern × animation layer.
    //==========================================================================
    struct BgSpec
    {
        int base;     // 0 vertical grad, 1 radial spotlight, 2 flat dark, 3 diagonal grad
        int pattern;  // 0 none, 1 diag stripes, 2 grid, 3 dot matrix, 4 hex rows, 5 scanlines, 6 rings, 7 checker
        int anim;     // 0 none, 1 particles, 2 starfield, 3 aurora, 4 waves, 5 rain, 6 pulse, 7 orbit, 8 pattern drift
    };

    static const BgSpec bgSpecs[30] =
    {
        { 0, 0, 0 }, //  0 Midnight Gradient
        { 0, 1, 0 }, //  1 Carbon Weave
        { 0, 0, 1 }, //  2 Particle Drift
        { 2, 0, 2 }, //  3 Starfield
        { 0, 0, 3 }, //  4 Aurora
        { 0, 0, 4 }, //  5 Sine Waves
        { 2, 0, 5 }, //  6 Digital Rain
        { 0, 0, 6 }, //  7 Pulse Glow
        { 1, 0, 7 }, //  8 Orbit
        { 2, 2, 0 }, //  9 Blueprint Grid
        { 0, 3, 0 }, // 10 Dot Matrix
        { 0, 4, 0 }, // 11 Hex Rows
        { 0, 5, 0 }, // 12 Scanlines
        { 1, 0, 0 }, // 13 Radial Spotlight
        { 1, 6, 0 }, // 14 Echo Rings
        { 0, 2, 8 }, // 15 Drifting Grid
        { 0, 5, 8 }, // 16 Rolling Scan
        { 0, 7, 0 }, // 17 Subtle Checker
        { 0, 2, 3 }, // 18 Aurora Grid
        { 2, 2, 2 }, // 19 Star Grid
        { 0, 1, 1 }, // 20 Particle Weave
        { 1, 6, 6 }, // 21 Pulse Rings
        { 0, 2, 4 }, // 22 Wave Grid
        { 2, 5, 5 }, // 23 Rainy Terminal
        { 1, 0, 6 }, // 24 Breathing Depth
        { 3, 1, 8 }, // 25 Diagonal Flow
        { 1, 0, 2 }, // 26 Cosmos
        { 3, 0, 4 }, // 27 Ocean
        { 2, 3, 8 }, // 28 Matrix Dots
        { 3, 0, 3 }, // 29 Falcon Sky
    };

    inline const KnobSpec& knobSpec()
    {
        constexpr int i = FALCON_KNOB_STYLE;
        return knobSpecs[(i >= 0 && i < 30) ? i : 0];
    }

    inline const BgSpec& bgSpec()
    {
        constexpr int i = FALCON_BG_THEME;
        return bgSpecs[(i >= 0 && i < 30) ? i : 0];
    }

    inline bool bgAnimated() { return bgSpec().anim != 0; }

    // Custom background image (optional) — decoded once and cached
    inline juce::Image& customBgImage()
    {
        static juce::Image img = falconbg::hasImage
            ? juce::ImageFileFormat::loadFrom (falconbg::imageData, (size_t) falconbg::imageDataSize)
            : juce::Image();
        return img;
    }

    inline bool hasCustomBg() { return falconbg::hasImage && customBgImage().isValid(); }

    // Deterministic pseudo-random for stateless animation layers
    inline float prand (int i, float salt = 1.0f)
    {
        return 0.5f + 0.5f * std::sin ((float) i * 12.9898f * salt + 78.233f);
    }

    //==========================================================================
    inline void paintBackground (juce::Graphics& g, juce::Rectangle<float> bounds, float phase)
    {
        const auto& spec = bgSpec();
        const float w = bounds.getWidth(), h = bounds.getHeight();

        // ---- Base fill: a user-supplied image wins over the preset base fill
        //      (pattern/animation layers below still render on top of it)
        if (hasCustomBg())
        {
            auto& img = customBgImage();
            const float ir = (float) img.getWidth() / (float) juce::jmax (1, img.getHeight());
            const float br = w / juce::jmax (1.0f, h);
            juce::Rectangle<float> dest;
            if (ir > br) { const float dw = h * ir; dest = { (w - dw) * 0.5f, 0.0f, dw, h }; }
            else         { const float dh = w / ir; dest = { 0.0f, (h - dh) * 0.5f, w, dh }; }

            g.drawImage (img, dest, juce::RectanglePlacement::fillDestination, false);
            g.setColour (background.withAlpha (0.42f));
            g.fillRect (bounds);
        }
        else switch (spec.base)
        {
            case 1: // radial spotlight
            {
                juce::ColourGradient rg (panel.brighter (0.06f), w * 0.5f, h * 0.32f,
                                         background.darker (0.15f), w * 0.5f, h * 1.4f, true);
                g.setGradientFill (rg);
                g.fillAll();
                break;
            }
            case 2: // flat dark
                g.fillAll (background.darker (0.1f));
                break;
            case 3: // diagonal gradient
            {
                juce::ColourGradient dg (panel, 0.0f, 0.0f, background, w, h, false);
                dg.addColour (0.55, background.interpolatedWith (panel, 0.4f));
                g.setGradientFill (dg);
                g.fillAll();
                break;
            }
            default: // vertical gradient
            {
                juce::ColourGradient bg (panel, w * 0.5f, 0.0f, background, w * 0.5f, h, false);
                g.setGradientFill (bg);
                g.fillAll();
                break;
            }
        }

        // ---- Pattern layer (drifts with phase when anim == 8)
        const float drift = (spec.anim == 8) ? std::fmod (phase * 6.0f, 48.0f) : 0.0f;

        switch (spec.pattern)
        {
            case 1: // diagonal stripes
                g.setColour (juce::Colours::white.withAlpha (0.018f));
                for (float x = -h - 48.0f + drift; x < w; x += 8.0f)
                    g.drawLine (x, h, x + h, 0.0f, 3.0f);
                g.setColour (juce::Colours::black.withAlpha (0.05f));
                for (float x = -h - 44.0f + drift; x < w; x += 8.0f)
                    g.drawLine (x, h, x + h, 0.0f, 1.5f);
                break;

            case 2: // grid
                g.setColour (accent.withAlpha (0.045f));
                for (float x = std::fmod (drift, 24.0f); x < w; x += 24.0f) g.drawVerticalLine ((int) x, 0.0f, h);
                for (float y = std::fmod (drift, 24.0f); y < h; y += 24.0f) g.drawHorizontalLine ((int) y, 0.0f, w);
                break;

            case 3: // dot matrix
                g.setColour (textDim.withAlpha (0.08f));
                for (float y = 8.0f + std::fmod (drift, 16.0f); y < h; y += 16.0f)
                    for (float x = 8.0f; x < w; x += 16.0f)
                        g.fillEllipse (x, y, 1.6f, 1.6f);
                break;

            case 4: // hex rows (offset dots)
            {
                g.setColour (accent.withAlpha (0.05f));
                int row = 0;
                for (float y = 6.0f; y < h; y += 13.0f, ++row)
                    for (float x = (row % 2 == 0 ? 6.0f : 13.5f); x < w; x += 15.0f)
                        g.fillEllipse (x, y, 2.2f, 2.2f);
                break;
            }

            case 5: // scanlines
                g.setColour (juce::Colours::black.withAlpha (0.10f));
                for (float y = std::fmod (drift, 4.0f); y < h; y += 4.0f)
                    g.drawHorizontalLine ((int) y, 0.0f, w);
                break;

            case 6: // concentric rings
                g.setColour (accent.withAlpha (0.035f));
                for (int i = 1; i <= 8; ++i)
                {
                    const float r = i * 70.0f + drift;
                    g.drawEllipse (w * 0.5f - r, h * 0.42f - r, r * 2.0f, r * 2.0f, 1.2f);
                }
                break;

            case 7: // subtle checker
                g.setColour (juce::Colours::white.withAlpha (0.012f));
                for (int yy = 0; yy < (int) (h / 20.0f) + 1; ++yy)
                    for (int xx = 0; xx < (int) (w / 20.0f) + 1; ++xx)
                        if ((xx + yy) % 2 == 0)
                            g.fillRect (xx * 20.0f, yy * 20.0f, 20.0f, 20.0f);
                break;

            default: break;
        }

        // ---- Animation layer
        switch (spec.anim)
        {
            case 1: // drifting particles
                for (int i = 0; i < 26; ++i)
                {
                    const float fi = (float) i, seed = fi * 37.719f;
                    const float px = w * (0.5f + 0.48f * std::sin (seed + phase * (0.10f + 0.017f * fi)));
                    const float py = h * (0.5f + 0.46f * std::cos (seed * 1.7f + phase * (0.13f + 0.011f * fi)));
                    const float sz = 2.0f + 3.4f * prand (i, 2.3f);
                    g.setColour (accent.withAlpha (0.05f + 0.09f * (0.5f + 0.5f * std::cos (seed + phase * 0.18f))));
                    g.fillEllipse (px - sz, py - sz, sz * 2.0f, sz * 2.0f);
                }
                break;

            case 2: // starfield (twinkle)
                for (int i = 0; i < 60; ++i)
                {
                    const float px = w * prand (i, 1.1f);
                    const float py = h * prand (i, 3.7f);
                    const float tw = 0.5f + 0.5f * std::sin (phase * (0.8f + prand (i, 5.1f)) + (float) i);
                    const float sz = 0.8f + 1.6f * prand (i, 7.9f);
                    g.setColour (textBright.withAlpha (0.05f + 0.16f * tw));
                    g.fillEllipse (px, py, sz, sz);
                }
                break;

            case 3: // aurora (two roaming glow blobs)
                for (int i = 0; i < 2; ++i)
                {
                    const float cx = w * (0.5f + 0.4f * std::sin (phase * 0.07f + i * 2.6f));
                    const float cy = h * (0.35f + 0.25f * std::cos (phase * 0.05f + i * 1.9f));
                    const auto  col = (i == 0 ? accent : accentHot).withAlpha (0.05f);
                    juce::ColourGradient blob (col, cx, cy, col.withAlpha (0.0f), cx + w * 0.45f, cy + h * 0.5f, true);
                    g.setGradientFill (blob);
                    g.fillEllipse (cx - w * 0.45f, cy - h * 0.4f, w * 0.9f, h * 0.8f);
                }
                break;

            case 4: // sine waves
                g.setColour (accent.withAlpha (0.06f));
                for (int wv = 0; wv < 3; ++wv)
                {
                    juce::Path path;
                    const float baseY = h * (0.55f + 0.14f * wv);
                    path.startNewSubPath (0.0f, baseY);
                    for (float x = 0.0f; x <= w; x += 8.0f)
                        path.lineTo (x, baseY + (10.0f + 4.0f * wv)
                                          * std::sin (x * 0.018f + phase * (0.5f + 0.17f * wv) + wv * 1.7f));
                    g.strokePath (path, juce::PathStrokeType (1.4f));
                }
                break;

            case 5: // digital rain
                for (int i = 0; i < 34; ++i)
                {
                    const float px  = w * prand (i, 1.3f);
                    const float spd = 60.0f + 90.0f * prand (i, 4.2f);
                    const float py  = std::fmod (phase * spd + h * prand (i, 6.6f), h + 40.0f) - 20.0f;
                    g.setColour (accent.withAlpha (0.10f + 0.08f * prand (i, 8.8f)));
                    g.drawLine (px, py, px, py + 14.0f, 1.2f);
                }
                break;

            case 6: // breathing pulse vignette
            {
                const float pulse = 0.5f + 0.5f * std::sin (phase * 0.9f);
                juce::ColourGradient pg (accent.withAlpha (0.035f + 0.05f * pulse), w * 0.5f, h * 0.4f,
                                         accent.withAlpha (0.0f), w * 0.5f, h * 1.15f, true);
                g.setGradientFill (pg);
                g.fillAll();
                break;
            }

            case 7: // orbiting dots
                for (int i = 0; i < 10; ++i)
                {
                    const float ang = phase * (0.25f + 0.05f * i) + i * 0.63f;
                    const float rad = 90.0f + 26.0f * i;
                    const float px  = w * 0.5f + rad * std::cos (ang);
                    const float py  = h * 0.45f + rad * 0.45f * std::sin (ang);
                    g.setColour (accent.withAlpha (0.10f));
                    g.fillEllipse (px - 2.2f, py - 2.2f, 4.4f, 4.4f);
                }
                break;

            default: break;
        }
    }
}

class FalconLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FalconLookAndFeel()
    {
        setColour (juce::Slider::thumbColourId, falcon::accent);
        setColour (juce::Slider::trackColourId, falcon::accent.withAlpha (0.55f));
        setColour (juce::Slider::backgroundColourId, falcon::trackDark);
        setColour (juce::Slider::textBoxTextColourId, falcon::accent);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, falcon::accent.withAlpha (0.3f));

        setColour (juce::Label::textColourId, falcon::textBright);

        setColour (juce::ComboBox::backgroundColourId, falcon::panelLight);
        setColour (juce::ComboBox::textColourId, falcon::accent);
        setColour (juce::ComboBox::outlineColourId, falcon::trackDark);
        setColour (juce::ComboBox::arrowColourId, falcon::textDim);

        setColour (juce::PopupMenu::backgroundColourId, falcon::panelLight);
        setColour (juce::PopupMenu::textColourId, falcon::textBright);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, falcon::accent.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, falcon::accent);

        setColour (juce::ToggleButton::textColourId, falcon::textDim);
    }

    // Borderless, clean value read-outs (no boxes / underlines)
    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        if (label.isBeingEdited())
            return;

        const bool isCombo = dynamic_cast<juce::ComboBox*> (label.getParentComponent()) != nullptr;
        g.setColour (isCombo ? falcon::accent : falcon::accent.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (isCombo ? 14.0f : 12.0f, juce::Font::bold)));
        g.drawText (label.getText(), label.getLocalBounds(),
                    label.getJustificationType().getOnlyHorizontalFlags() != 0
                        ? label.getJustificationType() : juce::Justification::centred,
                    true);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto& spec = falcon::knobSpec();

        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (8.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
        auto centre = bounds.getCentre();
        auto angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Bipolar knobs (e.g. PAN) fill their value arc from the 12-o'clock
        // centre outwards to the left or right, instead of from the left end.
        const bool bipolar = (bool) slider.getProperties().getWithDefault ("bipolar", false);
        const float valueFrom = bipolar ? (rotaryStartAngle + rotaryEndAngle) * 0.5f
                                        : rotaryStartAngle;

        // ---- Ticks
        if (spec.ticks)
        {
            g.setColour (falcon::textDim.withAlpha (0.45f));
            for (int i = 0; i <= 10; ++i)
            {
                const float a  = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (float) i / 10.0f;
                const float r1 = radius + 3.0f, r2 = radius + 7.0f;
                g.drawLine (centre.x + r1 * std::sin (a), centre.y - r1 * std::cos (a),
                            centre.x + r2 * std::sin (a), centre.y - r2 * std::cos (a),
                            i % 5 == 0 ? 1.6f : 0.9f);
            }
        }

        // ---- Track
        const float arcRadius = spec.ticks ? radius - 2.0f : radius;
        if (spec.fullRing)
        {
            g.setColour (falcon::trackDark.withAlpha (0.9f));
            g.drawEllipse (centre.x - arcRadius, centre.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f, 2.0f);
        }
        else
        {
            juce::Path track;
            track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
            g.setColour (falcon::trackDark);
            g.strokePath (track, juce::PathStrokeType (spec.trackW, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        // ---- Value arc (+ glow halo layers)
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             valueFrom, angle, true);

        if (spec.glow >= 3)
        {
            g.setColour (falcon::accent.withAlpha (0.08f));
            g.strokePath (value, juce::PathStrokeType (spec.trackW + 12.0f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }
        if (spec.glow >= 2)
        {
            g.setColour (falcon::accent.withAlpha (0.12f));
            g.strokePath (value, juce::PathStrokeType (spec.trackW + 7.5f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }
        if (spec.glow >= 1)
        {
            g.setColour (falcon::accent.withAlpha (0.30f));
            g.strokePath (value, juce::PathStrokeType (spec.trackW + 3.5f, juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));
        }

        if (spec.gradArc)
            g.setGradientFill (juce::ColourGradient (falcon::accent, centre.x, centre.y - arcRadius,
                                                     falcon::accentHot, centre.x, centre.y + arcRadius, false));
        else
            g.setColour (falcon::accent);

        g.strokePath (value, juce::PathStrokeType (spec.trackW, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

        // ---- Centre cap
        const float capRadius = arcRadius * spec.capRatio;
        if (spec.cap != 0 && capRadius > 2.0f)
        {
            switch (spec.cap)
            {
                case 1: // flat
                    g.setColour (falcon::panelLight);
                    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                    break;

                case 2: // radial gradient
                {
                    juce::ColourGradient capGrad (falcon::panelLight.brighter (0.15f),
                                                  centre.x - capRadius * 0.5f, centre.y - capRadius * 0.5f,
                                                  falcon::panel.darker (0.2f),
                                                  centre.x + capRadius, centre.y + capRadius, true);
                    g.setGradientFill (capGrad);
                    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                    g.setColour (falcon::trackDark.brighter (0.1f));
                    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);
                    break;
                }

                case 3: // knurled metal
                {
                    juce::ColourGradient capGrad (falcon::panelLight.brighter (0.25f),
                                                  centre.x, centre.y - capRadius,
                                                  falcon::panel.darker (0.3f),
                                                  centre.x, centre.y + capRadius, false);
                    g.setGradientFill (capGrad);
                    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                    g.setColour (falcon::background.withAlpha (0.5f));
                    for (int i = 0; i < 24; ++i)
                    {
                        const float a = juce::MathConstants<float>::twoPi * (float) i / 24.0f + angle * 0.5f;
                        g.drawLine (centre.x + capRadius * 0.75f * std::sin (a), centre.y - capRadius * 0.75f * std::cos (a),
                                    centre.x + capRadius * 0.95f * std::sin (a), centre.y - capRadius * 0.95f * std::cos (a), 1.0f);
                    }
                    break;
                }

                case 4: // dark inset
                    g.setColour (falcon::background.brighter (0.06f));
                    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                    g.setColour (juce::Colours::black.withAlpha (0.35f));
                    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.5f);
                    break;

                case 5: // dome with top-left highlight
                {
                    juce::ColourGradient dome (falcon::panelLight.brighter (0.5f),
                                               centre.x - capRadius * 0.6f, centre.y - capRadius * 0.6f,
                                               falcon::panel, centre.x + capRadius * 0.4f, centre.y + capRadius * 0.7f, true);
                    g.setGradientFill (dome);
                    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);
                    g.setColour (falcon::trackDark);
                    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);
                    break;
                }
            }
        }

        // ---- Pointer
        const float pr = (spec.cap != 0 && capRadius > 2.0f) ? capRadius : arcRadius * 0.62f;
        switch (spec.pointer)
        {
            case 0: // rounded bar
            {
                juce::Path p;
                p.addRoundedRectangle (-2.0f, -pr + 3.0f, 4.0f, pr * 0.45f, 2.0f);
                g.setColour (falcon::accent);
                g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
                break;
            }
            case 1: // thin line
            {
                juce::Path p;
                p.addRectangle (-1.2f, -pr + 4.0f, 2.4f, pr * 0.5f);
                g.setColour (falcon::accent);
                g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
                break;
            }
            case 2: // glowing tip dot on the arc
            {
                const float tipX = centre.x + arcRadius * std::sin (angle);
                const float tipY = centre.y - arcRadius * std::cos (angle);
                g.setColour (falcon::accentHot.withAlpha (0.35f));
                g.fillEllipse (tipX - 7.0f, tipY - 7.0f, 14.0f, 14.0f);
                g.setColour (falcon::textBright);
                g.fillEllipse (tipX - 2.6f, tipY - 2.6f, 5.2f, 5.2f);
                break;
            }
            case 3: // needle from centre
            {
                juce::Path p;
                p.addTriangle (0.0f, -pr + 2.0f, -3.0f, 0.0f, 3.0f, 0.0f);
                g.setColour (falcon::accent);
                g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
                break;
            }
            case 4: // dot near the cap edge
            {
                const float dr  = pr * 0.7f;
                const float dx  = centre.x + dr * std::sin (angle);
                const float dy  = centre.y - dr * std::cos (angle);
                g.setColour (falcon::accent);
                g.fillEllipse (dx - 3.0f, dy - 3.0f, 6.0f, 6.0f);
                break;
            }
            default: break; // none
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool) override
    {
        const bool on = button.getToggleState();
        auto bounds = button.getLocalBounds().toFloat();

        // Pill switch on the left (inset so the "on" glow ring never clips)
        auto pill = juce::Rectangle<float> (4.0f, bounds.getCentreY() - 11.0f, 42.0f, 22.0f);
        g.setColour (on ? falcon::accent.withAlpha (0.9f) : falcon::trackDark);
        g.fillRoundedRectangle (pill, 11.0f);

        if (on)
        {
            g.setColour (falcon::accent.withAlpha (0.25f));
            g.drawRoundedRectangle (pill.expanded (3.0f), 14.0f, 3.0f);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (falcon::textDim.withAlpha (0.4f));
            g.drawRoundedRectangle (pill, 11.0f, 1.0f);
        }

        auto knobX = on ? pill.getRight() - 19.0f : pill.getX() + 3.0f;
        g.setColour (on ? falcon::background : falcon::textDim);
        g.fillEllipse (knobX, pill.getY() + 3.0f, 16.0f, 16.0f);

        // Label text
        g.setColour (on ? falcon::accent : falcon::textDim);
        g.setFont (juce::Font (juce::FontOptions (13.0f, juce::Font::bold)));
        g.drawText (button.getButtonText(),
                    bounds.withTrimmedLeft (54.0f).toNearestInt(),
                    juce::Justification::centredLeft);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                       int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);
        g.setColour (falcon::panelLight);
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (box.hasKeyboardFocus (true) ? falcon::accent.withAlpha (0.6f) : falcon::trackDark);
        g.drawRoundedRectangle (bounds, 6.0f, 1.2f);

        // Chevron
        juce::Path chevron;
        auto cx = (float) width - 14.0f;
        auto cy = (float) height * 0.5f;
        chevron.startNewSubPath (cx - 4.0f, cy - 2.0f);
        chevron.lineTo (cx, cy + 3.0f);
        chevron.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (falcon::textDim);
        g.strokePath (chevron, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (juce::FontOptions (15.0f, juce::Font::bold));
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (8, 1, box.getWidth() - 26, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
        label.setJustificationType (juce::Justification::centred);
    }
};
