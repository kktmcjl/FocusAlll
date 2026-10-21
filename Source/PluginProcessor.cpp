
#include "PluginProcessor.h"
#include "PluginEditor.h"

TrapVoxAIProcessor::APVTS::ParameterLayout TrapVoxAIProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    using Float = juce::AudioParameterFloat;
    using Choice = juce::AudioParameterChoice;

    p.add(std::make_unique<Float>("inputGain", "Input", juce::NormalisableRange<float>(-18.0f, 12.0f, 0.1f), 0.0f));
    p.add(std::make_unique<Float>("highpass", "High-pass", juce::NormalisableRange<float>(40.0f, 180.0f, 1.0f), 85.0f));
    p.add(std::make_unique<Float>("presence", "Presence", juce::NormalisableRange<float>(-6.0f, 6.0f, 0.1f), 2.0f));
    p.add(std::make_unique<Float>("comp", "Compression", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));
    p.add(std::make_unique<Float>("deess", "De-ess", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));
    p.add(std::make_unique<Float>("drive", "Saturation", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.18f));
    p.add(std::make_unique<Float>("reverb", "Reverb", juce::NormalisableRange<float>(0.0f, 0.5f, 0.001f), 0.12f));
    p.add(std::make_unique<Float>("delay", "Delay", juce::NormalisableRange<float>(0.0f, 0.5f, 0.001f), 0.12f));
    p.add(std::make_unique<Float>("delayMs", "Delay time (ms)", juce::NormalisableRange<float>(60.0f, 700.0f, 1.0f), 320.0f));

    p.add(std::make_unique<Choice>(
        "preset", "Style",
        juce::StringArray { "Natural", "Melodic Trap", "Aggressive Trap" }, 1));

    return p;
}

TrapVoxAIProcessor::TrapVoxAIProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

bool TrapVoxAIProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void TrapVoxAIProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        static_cast<juce::uint32>(getTotalNumOutputChannels())
    };

    highPass.prepare(spec);
    highPass.reset();

    *highPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate,
        *parameters.getRawParameterValue("highpass"));

    reverb.reset();

    const int maxDelay = static_cast<int>(sampleRate * 0.8) + samplesPerBlock + 1;
    delayBuffer.setSize(2, maxDelay);
    delayBuffer.clear();

    delayWritePosition = 0;
    envelope = 0.0f;
}

void TrapVoxAIProcessor::releaseResources()
{
    delayBuffer.setSize(0, 0);
}

void TrapVoxAIProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();

    if (channels == 0 || delayBuffer.getNumSamples() < 2)
        return;

    const auto inputGain = juce::Decibels::decibelsToGain(
        *parameters.getRawParameterValue("inputGain"));

    const float hp = *parameters.getRawParameterValue("highpass");
    const float presence = *parameters.getRawParameterValue("presence");
    const float comp = *parameters.getRawParameterValue("comp");
    const float deess = *parameters.getRawParameterValue("deess");
    const float drive = *parameters.getRawParameterValue("drive");
    const float wetReverb = *parameters.getRawParameterValue("reverb");
    const float wetDelay = *parameters.getRawParameterValue("delay");
    const float delayMs = *parameters.getRawParameterValue("delayMs");

    const int style = static_cast<int>(
        parameters.getRawParameterValue("preset")->load());

    const float styleDrive = drive * (style == 2 ? 1.5f : 1.0f);
    const float styleComp = juce::jlimit(
        0.0f, 1.0f, comp + (style == 2 ? 0.18f : 0.0f));

    *highPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        currentSampleRate, hp);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    highPass.process(context);

    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : left;

    const float attack = std::exp(
        -1.0f / static_cast<float>(currentSampleRate * 0.008));

    const float release = std::exp(
        -1.0f / static_cast<float>(currentSampleRate * 0.09));

    const int delaySize = delayBuffer.getNumSamples();

    const int delaySamples = juce::jlimit(
        1,
        delaySize - 1,
        static_cast<int>(currentSampleRate * delayMs / 1000.0f));

    for (int i = 0; i < numSamples; ++i)
    {
        float monoAbs = 0.0f;

        for (int ch = 0; ch < channels; ++ch)
            monoAbs += std::abs(buffer.getSample(ch, i));

        monoAbs /= static_cast<float>(channels);

        const float coeff = monoAbs > envelope ? attack : release;
        envelope = coeff * envelope + (1.0f - coeff) * monoAbs;

        const float threshold = 0.22f - styleComp * 0.10f;
        const float over = juce::jmax(0.0f, envelope - threshold);

        const float compGain =
            1.0f / (1.0f + over * (2.0f + styleComp * 8.0f));

        const float deessGain = 1.0f - deess * juce::jlimit(
            0.0f, 0.35f, (monoAbs - 0.18f) * 0.6f);

        for (int ch = 0; ch < channels; ++ch)
        {
            float x = buffer.getSample(ch, i) * inputGain;

            x *= 1.0f + juce::jlimit(-0.15f, 0.15f, presence * 0.012f);
            x *= compGain * deessGain;

            const int readPosition =
                (delayWritePosition - delaySamples + delaySize) % delaySize;

            const float delayed = delayBuffer.getSample(ch, readPosition);

            const float shaped = std::tanh(x * (1.0f + styleDrive * 5.0f));

            x = x * (1.0f - styleDrive * 0.22f)
                + shaped * (styleDrive * 0.22f);

            x += delayed * wetDelay * 0.42f;

            delayBuffer.setSample(ch, delayWritePosition, x);
            buffer.setSample(ch, i, x);
        }

        if (++delayWritePosition >= delaySize)
            delayWritePosition = 0;
    }

    reverbParams.roomSize = 0.58f;
    reverbParams.damping = 0.42f;
    reverbParams.wetLevel = wetReverb;
    reverbParams.dryLevel = 1.0f - wetReverb * 0.45f;
    reverbParams.width = 0.9f;

    reverb.setParameters(reverbParams);

    // JUCE-compatible reverb processing.
    // Process only the actual output channels.
    juce::dsp::AudioBlock<float> reverbBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> reverbContext(reverbBlock);
    reverb.process(reverbContext);

    // Conservative output safety limiter.
    for (int ch = 0; ch < channels; ++ch)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer.setSample(
                ch, i,
                juce::jlimit(-0.98f, 0.98f, buffer.getSample(ch, i)));
        }
    }
}

juce::AudioProcessorEditor* TrapVoxAIProcessor::createEditor()
{
    return new TrapVoxAIEditor(*this);
}

void TrapVoxAIProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TrapVoxAIProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrapVoxAIProcessor();
}
