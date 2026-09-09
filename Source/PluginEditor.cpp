#include "PluginEditor.h"

AnalogAPIEQAudioProcessorEditor::AnalogAPIEQAudioProcessorEditor (AnalogAPIEQAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), curveComponent (p), bandPanel (p)
{
    setLookAndFeel (&analogLookAndFeel);

    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, AnalogColours::accent);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    addAndMakeVisible (curveComponent);
    addAndMakeVisible (bandPanel);

    for (auto* s : { &saturationSlider, &outputGainSlider })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        addAndMakeVisible (s);
    }

    for (auto* l : { &saturationLabel, &outputLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (11.0f));
        addAndMakeVisible (l);
    }

    addAndMakeVisible (bypassButton);

    using APVTS = juce::AudioProcessorValueTreeState;
    saturationAttach = std::make_unique<APVTS::SliderAttachment> (p.apvts, "saturation", saturationSlider);
    outputAttach = std::make_unique<APVTS::SliderAttachment> (p.apvts, "outputGain", outputGainSlider);
    bypassAttach = std::make_unique<APVTS::ButtonAttachment> (p.apvts, "bypass", bypassButton);

    setResizable (true, true);
    setResizeLimits (900, 560, 1600, 1000);
    setSize (1100, 680);
}

AnalogAPIEQAudioProcessorEditor::~AnalogAPIEQAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AnalogAPIEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (AnalogColours::background);
}

void AnalogAPIEQAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop (44);
    titleLabel.setBounds (header.removeFromLeft (300).reduced (12, 0));

    auto controlsWidth = 220;
    auto sideControls = header.removeFromRight (controlsWidth);
    bypassButton.setBounds (sideControls.removeFromLeft (80).reduced (4));
    auto satArea = sideControls.removeFromLeft (70);
    saturationSlider.setBounds (satArea.reduced (2));
    auto outArea = sideControls;
    outputGainSlider.setBounds (outArea.reduced (2));

    auto bottomPanel = area.removeFromBottom (170);
    bandPanel.setBounds (bottomPanel.reduced (4));

    curveComponent.setBounds (area.reduced (4));

    saturationLabel.setBounds (0, 0, 1, 1); // las etiquetas se integran en el texto de los sliders
    outputLabel.setBounds (0, 0, 1, 1);
}
