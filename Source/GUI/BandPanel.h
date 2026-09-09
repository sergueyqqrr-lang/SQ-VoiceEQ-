#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/** Tira de controles precisos para UNA banda: activo, tipo, freq, gain, Q, proportional-Q. */
class BandStrip : public juce::Component
{
public:
    BandStrip (AnalogAPIEQAudioProcessor& proc, int bandIndex);

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    AnalogAPIEQAudioProcessor& processor;
    int index;

    juce::ToggleButton activeButton { "On" };
    juce::ComboBox typeBox;
    juce::Slider freqSlider, gainSlider, qSlider;
    juce::Label freqLabel { {}, "Freq" }, gainLabel { {}, "Gain" }, qLabel { {}, "Q" };
    juce::ToggleButton proportionalButton { "Prop Q" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttach, gainAttach, qAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> activeAttach, propAttach;
};

/** Fila horizontal con todas las bandas, dentro de un viewport con scroll. */
class BandPanel : public juce::Component
{
public:
    explicit BandPanel (AnalogAPIEQAudioProcessor& proc);

    void resized() override;

private:
    juce::Viewport viewport;
    juce::Component content;
    juce::OwnedArray<BandStrip> strips;
};
