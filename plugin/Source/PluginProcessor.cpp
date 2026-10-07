#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
    static const juce::String sync     { "sync" };
    static const juce::String timeMs   { "timeMs" };
    static const juce::String division { "division" };
    static const juce::String feedback { "feedback" };
    static const juce::String pingpong { "pingpong" };
    static const juce::String lowCut   { "lowCut" };
    static const juce::String highCut  { "highCut" };
    static const juce::String mix      { "mix" };
    static const juce::String output   { "output" };
}

static const juce::StringArray divisionNames {
    "1/1", "1/2 D", "1/2", "1/2 T", "1/4 D", "1/4", "1/4 T",
    "1/8 D", "1/8", "1/8 T", "1/16 D", "1/16", "1/16 T", "1/32"
};

double FalconEcho2026Processor::divisionToBeats (int i)
{
    // Beats (quarter notes) per division. D = dotted (1.5x), T = triplet (2/3x).
    static const double beats[] = {
        4.0,            // 1/1
        2.0 * 1.5,      // 1/2 D
        2.0,            // 1/2
        2.0 * 2.0/3.0,  // 1/2 T
        1.0 * 1.5,      // 1/4 D
        1.0,            // 1/4
        1.0 * 2.0/3.0,  // 1/4 T
        0.5 * 1.5,      // 1/8 D
        0.5,            // 1/8
        0.5 * 2.0/3.0,  // 1/8 T
        0.25 * 1.5,     // 1/16 D
        0.25,           // 1/16
        0.25 * 2.0/3.0, // 1/16 T
        0.125           // 1/32
    };
    return beats[juce::jlimit (0, (int) std::size (beats) - 1, i)];
}

juce::AudioProcessorValueTreeState::ParameterLayout FalconEcho2026Processor::createParameterLayout()
{
    using P = juce::AudioProcessorValueTreeState;
    P::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { IDs::sync, 1 }, "Tempo Sync", true));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::timeMs, 1 }, "Time",
        juce::NormalisableRange<float> (1.0f, 2000.0f, 0.1f, 0.4f), 350.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { IDs::division, 1 }, "Division", divisionNames, 5)); // default 1/4

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::feedback, 1 }, "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.001f), 0.40f));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { IDs::pingpong, 1 }, "Ping Pong", false));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::lowCut, 1 }, "Low Cut",
        juce::NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.5f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::highCut, 1 }, "High Cut",
        juce::NormalisableRange<float> (1000.0f, 20000.0f, 1.0f, 0.5f), 12000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::mix, 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.35f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { IDs::output, 1 }, "Output",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    return layout;
}

FalconEcho2026Processor::FalconEcho2026Processor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    // Local file check only (fast) — safe during DAW plugin scans.
    licensed.store (FALCON_LICENSE_DISABLED != 0 || licenses.isActivated());
}

bool FalconEcho2026Processor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void FalconEcho2026Processor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRateHz = sampleRate;

    juce::dsp::ProcessSpec monoSpec { sampleRate, (juce::uint32) samplesPerBlock, 1 };

    const int maxSamples = (int) std::ceil (maxDelaySeconds * sampleRate) + 8;

    for (auto* d : { &delayL, &delayR })
    {
        d->prepare (monoSpec);
        d->setMaximumDelayInSamples (maxSamples);
        d->reset();
    }

    for (auto* f : { &highPassL, &highPassR })
    {
        f->prepare (monoSpec);
        f->setType (juce::dsp::StateVariableTPTFilterType::highpass);
        f->reset();
    }

    for (auto* f : { &lowPassL, &lowPassR })
    {
        f->prepare (monoSpec);
        f->setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        f->reset();
    }

    delayTimeSm.reset (sampleRate, 0.08);   // gentle tape-style glide on time changes
    feedbackSm.reset (sampleRate, 0.02);
    mixSm.reset (sampleRate, 0.02);
    outGainSm.reset (sampleRate, 0.02);
}

void FalconEcho2026Processor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Unlicensed: pass the dry signal through untouched
    if (! licensed.load (std::memory_order_relaxed))
        return;

    // --- Read host tempo when available (Standalone has no playhead -> keep last/default)
    if (auto* playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto bpm = position->getBpm())
                if (*bpm > 0.0)
                    currentBpm.store ((float) *bpm);

    const bool  sync      = apvts.getRawParameterValue (IDs::sync)->load() > 0.5f;
    const float timeMs    = apvts.getRawParameterValue (IDs::timeMs)->load();
    const int   division  = (int) apvts.getRawParameterValue (IDs::division)->load();
    const float feedback  = apvts.getRawParameterValue (IDs::feedback)->load();
    const bool  pingpong  = apvts.getRawParameterValue (IDs::pingpong)->load() > 0.5f;
    const float lowCutHz  = apvts.getRawParameterValue (IDs::lowCut)->load();
    const float highCutHz = apvts.getRawParameterValue (IDs::highCut)->load();
    const float mix       = apvts.getRawParameterValue (IDs::mix)->load();
    const float outDb     = apvts.getRawParameterValue (IDs::output)->load();

    // --- Target delay time in samples
    double delaySeconds;
    if (sync)
        delaySeconds = divisionToBeats (division) * 60.0 / (double) currentBpm.load();
    else
        delaySeconds = timeMs * 0.001;

    delaySeconds = juce::jlimit (0.001, maxDelaySeconds, delaySeconds);

    delayTimeSm.setTargetValue ((float) (delaySeconds * sampleRateHz));
    feedbackSm.setTargetValue (feedback);
    mixSm.setTargetValue (mix);
    outGainSm.setTargetValue (juce::Decibels::decibelsToGain (outDb));

    highPassL.setCutoffFrequency (lowCutHz);
    highPassR.setCutoffFrequency (lowCutHz);
    lowPassL.setCutoffFrequency (highCutHz);
    lowPassR.setCutoffFrequency (highCutHz);

    const int numSamples = buffer.getNumSamples();
    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);

    float blockPeakL = 0.0f, blockPeakR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float delaySamples = delayTimeSm.getNextValue();
        const float fb           = feedbackSm.getNextValue();
        const float wet          = mixSm.getNextValue();
        const float outGain      = outGainSm.getNextValue();

        const float inL = left[i];
        const float inR = right[i];

        const float wetL = delayL.popSample (0, delaySamples, true);
        const float wetR = delayR.popSample (0, delaySamples, true);

        // Filter the feedback path so repeats get darker/thinner naturally
        const float fbL = lowPassL.processSample (0, highPassL.processSample (0, wetL)) * fb;
        const float fbR = lowPassR.processSample (0, highPassR.processSample (0, wetR)) * fb;

        if (pingpong)
        {
            // Mono input feeds the left line; repeats bounce L -> R -> L
            const float monoIn = 0.5f * (inL + inR);
            delayL.pushSample (0, monoIn + fbR);
            delayR.pushSample (0, fbL);
        }
        else
        {
            delayL.pushSample (0, inL + fbL);
            delayR.pushSample (0, inR + fbR);
        }

        const float outL = (inL * (1.0f - wet) + wetL * wet) * outGain;
        const float outR = (inR * (1.0f - wet) + wetR * wet) * outGain;

        left[i]  = outL;
        right[i] = outR;

        blockPeakL = juce::jmax (blockPeakL, std::abs (outL));
        blockPeakR = juce::jmax (blockPeakR, std::abs (outR));
    }

    // Meter values with simple peak-hold semantics (GUI applies decay)
    if (blockPeakL > peakL.load()) peakL.store (blockPeakL);
    if (blockPeakR > peakR.load()) peakR.store (blockPeakR);
}

void FalconEcho2026Processor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void FalconEcho2026Processor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* FalconEcho2026Processor::createEditor()
{
    return new FalconEcho2026Editor (*this);
}

// This creates the plugin instance for the host
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FalconEcho2026Processor();
}
