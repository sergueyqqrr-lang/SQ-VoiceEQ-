#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace AnalogColours
{
    const juce::Colour background   { 0xff17181c };
    const juce::Colour panel        { 0xff1f2126 };
    const juce::Colour gridLine     { 0xff33363d };
    const juce::Colour gridLineMain { 0xff45484f };
    const juce::Colour text         { 0xffd8d9dc };
    const juce::Colour textDim      { 0xff8a8d94 };
    const juce::Colour accent       { 0xffff9248 }; // ámbar cálido, look "analógico"
    const juce::Colour accentDim    { 0xffcc7638 };
    const juce::Colour curve        { 0xffffb066 };
    const juce::Colour spectrum     { 0xff4a5568 };
}

class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;
};
