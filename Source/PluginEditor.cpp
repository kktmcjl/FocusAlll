#include "PluginEditor.h"

TrapVoxAIEditor::TrapVoxAIEditor(TrapVoxAIProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(620, 440);
    title.setText("TRAPVOX AI", juce::dontSendNotification);
    title.setFont(juce::Font(28.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe8e8f2));
    addAndMakeVisible(title);
    subtitle.setText("MELODIC TRAP VOCAL CHAIN  •  VST3", juce::dontSendNotification);
    subtitle.setFont(juce::Font(12.0f));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffa9abc2));
    addAndMakeVisible(subtitle);

    setupSlider(inputGain, inputGainLabel, "Input (dB)");
    setupSlider(highpass, highpassLabel, "High-pass (Hz)");
    setupSlider(presence, presenceLabel, "Presence (dB)");
    setupSlider(comp, compLabel, "Compression");
    setupSlider(deess, deessLabel, "De-ess");
    setupSlider(drive, driveLabel, "Saturation");
    setupSlider(reverb, reverbLabel, "Reverb");
    setupSlider(delay, delayLabel, "Delay mix");
    setupSlider(delayMs, delayMsLabel, "Delay time (ms)");

    styleBox.addItem("Natural", 1);
    styleBox.addItem("Melodic Trap", 2);
    styleBox.addItem("Aggressive Trap", 3);
    addAndMakeVisible(styleBox);
    styleAttachment = std::make_unique<ComboAttachment>(audioProcessor.parameters, "preset", styleBox);

    auto attach = [this](juce::Slider& s, const char* id) {
        return std::make_unique<SliderAttachment>(audioProcessor.parameters, id, s);
    };
    inputGainAttachment = attach(inputGain, "inputGain");
    highpassAttachment = attach(highpass, "highpass");
    presenceAttachment = attach(presence, "presence");
    compAttachment = attach(comp, "comp");
    deessAttachment = attach(deess, "deess");
    driveAttachment = attach(drive, "drive");
    reverbAttachment = attach(reverb, "reverb");
    delayAttachment = attach(delay, "delay");
    delayMsAttachment = attach(delayMs, "delayMs");
}

void TrapVoxAIEditor::setupSlider(juce::Slider& s, juce::Label& l, const juce::String& name)
{
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colour(0xffd7d8e6));
    addAndMakeVisible(l);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb18cff));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff3efff));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff3efff));
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff29283a));
    addAndMakeVisible(s);
}

void TrapVoxAIEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff171722));
    g.setColour(juce::Colour(0xff29283a));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10.0f), 14.0f);
    g.setColour(juce::Colour(0xffb18cff));
    g.fillRoundedRectangle(20.0f, 18.0f, 5.0f, 54.0f, 2.0f);
    g.setColour(juce::Colour(0xffa9abc2));
    g.setFont(11.0f);
    g.drawFittedText("NOTE: Pitch correction / autotune is the next engine milestone; this build is the vocal FX foundation.",
        22, getHeight() - 28, getWidth() - 44, 18, juce::Justification::centredLeft, 1);
}

void TrapVoxAIEditor::resized()
{
    title.setBounds(34, 16, 260, 36);
    subtitle.setBounds(36, 50, 360, 22);
    styleBox.setBounds(getWidth() - 185, 28, 145, 28);

    juce::Slider* sliders[] = { &inputGain, &highpass, &presence, &comp, &deess, &drive, &reverb, &delay, &delayMs };
    juce::Label* labels[] = { &inputGainLabel, &highpassLabel, &presenceLabel, &compLabel, &deessLabel, &driveLabel, &reverbLabel, &delayLabel, &delayMsLabel };
    const int left = 24, top = 92, cellW = 190, cellH = 104;
    for (int i = 0; i < 9; ++i)
    {
        const int row = i / 3, col = i % 3;
        const int x = left + col * cellW;
        const int y = top + row * cellH;
        labels[i]->setBounds(x, y, 170, 20);
        sliders[i]->setBounds(x + 18, y + 16, 134, 82);
    }
}
