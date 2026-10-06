#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "FalconLicense.h"

class FalconDelayProcessor : public juce::AudioProcessor
{
public:
    FalconDelayProcessor();
    ~FalconDelayProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Falcon Delay"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Licensing: the delay stays bypassed (dry passthrough) until activated
    LicenseManager licenses { FALCON_PRODUCT_ID, FALCON_LICENSE_URL };
    std::atomic<bool> licensed { false };

    // For GUI meters / readouts (audio thread writes, GUI thread reads)
    std::atomic<float> peakL { 0.0f }, peakR { 0.0f };
    std::atomic<float> currentBpm { 120.0f };

    static constexpr double maxDelaySeconds = 10.0;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Delay time in beats for each sync division choice
    static double divisionToBeats (int divisionIndex);

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL { 48000 * 12 },
                                                                                     delayR { 48000 * 12 };
    juce::dsp::StateVariableTPTFilter<float> highPassL, highPassR, lowPassL, lowPassR;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayTimeSm, feedbackSm, mixSm, outGainSm;

    double sampleRateHz = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FalconDelayProcessor)
};
