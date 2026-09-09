#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/EQCurveComponent.h"
#include "GUI/BandPanel.h"

class AnalogAPIEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AnalogAPIEQAudioProcessorEditor (AnalogAPIEQAudioProcessor&);
    ~AnalogAPIEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AnalogAPIEQAudioProcessor& processorRef;

    AnalogLookAndFeel analogLookAndFeel;

    EQCurveComponent curveComponent;
    BandPanel bandPanel;

    juce::Slider saturationSlider, outputGainSlider;
    juce::Label saturationLabel { {}, "Analog Drive" }, outputLabel { {}, "Output" };
    juce::ToggleButton bypassButton { "Bypass" };
    juce::Label titleLabel { {}, "ANALOG API EQ" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttach, outputAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogAPIEQAudioProcessorEditor)
};
