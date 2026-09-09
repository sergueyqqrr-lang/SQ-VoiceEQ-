#include "BandPanel.h"
#include "LookAndFeel.h"

BandStrip::BandStrip (AnalogAPIEQAudioProcessor& proc, int bandIndex)
    : processor (proc), index (bandIndex)
{
    using APVTS = juce::AudioProcessorValueTreeState;

    addAndMakeVisible (activeButton);
    activeAttach = std::make_unique<APVTS::ButtonAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandActiveParamID (index), activeButton);

    typeBox.addItemList ({ "High Pass", "Low Shelf", "Bell", "High Shelf", "Low Pass", "Notch" }, 1);
    addAndMakeVisible (typeBox);
    typeAttach = std::make_unique<APVTS::ComboBoxAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandTypeParamID (index), typeBox);

    for (auto* s : { &freqSlider, &gainSlider, &qSlider })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
        addAndMakeVisible (s);
    }

    freqAttach = std::make_unique<APVTS::SliderAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandFreqParamID (index), freqSlider);
    gainAttach = std::make_unique<APVTS::SliderAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandGainParamID (index), gainSlider);
    qAttach = std::make_unique<APVTS::SliderAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandQParamID (index), qSlider);

    for (auto* l : { &freqLabel, &gainLabel, &qLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (10.0f));
        addAndMakeVisible (l);
    }

    addAndMakeVisible (proportionalButton);
    propAttach = std::make_unique<APVTS::ButtonAttachment> (
        processor.apvts, AnalogAPIEQAudioProcessor::getBandProportionalParamID (index), proportionalButton);
}

void BandStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    auto colour = juce::Colour::fromHSV ((float) index / (float) AnalogAPIEQAudioProcessor::maxBands, 0.55f, 1.0f, 1.0f);

    g.setColour (AnalogColours::panel);
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (colour.withAlpha (0.5f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.5f);

    g.setColour (colour);
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("BAND " + juce::String (index + 1), bounds.removeFromTop (16.0f), juce::Justification::centred);
}

void BandStrip::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (16); // espacio para el título pintado en paint()

    auto topRow = area.removeFromTop (24);
    activeButton.setBounds (topRow.removeFromLeft (40));
    typeBox.setBounds (topRow.reduced (2, 0));

    auto knobsRow = area.removeFromTop (70);
    auto w = knobsRow.getWidth() / 3;
    freqSlider.setBounds (knobsRow.removeFromLeft (w));
    gainSlider.setBounds (knobsRow.removeFromLeft (w));
    qSlider.setBounds (knobsRow);

    proportionalButton.setBounds (area.removeFromTop (22));
}

BandPanel::BandPanel (AnalogAPIEQAudioProcessor& proc)
{
    for (int i = 0; i < AnalogAPIEQAudioProcessor::maxBands; ++i)
    {
        auto* strip = new BandStrip (proc, i);
        strips.add (strip);
        content.addAndMakeVisible (strip);
    }

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (false, true);
    addAndMakeVisible (viewport);
}

void BandPanel::resized()
{
    viewport.setBounds (getLocalBounds());

    const int stripWidth = 150;
    content.setSize (stripWidth * strips.size(), getHeight());

    for (int i = 0; i < strips.size(); ++i)
        strips[i]->setBounds (i * stripWidth, 0, stripWidth - 4, getHeight());
}
