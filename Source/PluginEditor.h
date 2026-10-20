#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TrapVoxAIEditor final : public juce::AudioProcessorEditor
{
public:
    explicit TrapVoxAIEditor(TrapVoxAIProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TrapVoxAIProcessor& audioProcessor;
    juce::ComboBox styleBox;
    juce::Label title, subtitle;
    juce::Slider inputGain, highpass, presence, comp, deess, drive, reverb, delay, delayMs;
    juce::Label inputGainLabel, highpassLabel, presenceLabel, compLabel, deessLabel,
        driveLabel, reverbLabel, delayLabel, delayMsLabel;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<ComboAttachment> styleAttachment;
    std::unique_ptr<SliderAttachment> inputGainAttachment, highpassAttachment, presenceAttachment,
        compAttachment, deessAttachment, driveAttachment, reverbAttachment, delayAttachment, delayMsAttachment;

    void setupSlider(juce::Slider&, juce::Label&, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrapVoxAIEditor)
};
