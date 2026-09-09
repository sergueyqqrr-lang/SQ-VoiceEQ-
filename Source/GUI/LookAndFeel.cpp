#include "LookAndFeel.h"

AnalogLookAndFeel::AnalogLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, AnalogColours::background);
    setColour (juce::Slider::thumbColourId, AnalogColours::accent);
    setColour (juce::Slider::trackColourId, AnalogColours::accent);
    setColour (juce::Slider::backgroundColourId, AnalogColours::gridLine);
    setColour (juce::ComboBox::backgroundColourId, AnalogColours::panel);
    setColour (juce::ComboBox::textColourId, AnalogColours::text);
    setColour (juce::ComboBox::outlineColourId, AnalogColours::gridLine);
    setColour (juce::Label::textColourId, AnalogColours::text);
    setColour (juce::TextButton::buttonColourId, AnalogColours::panel);
    setColour (juce::TextButton::textColourOffId, AnalogColours::textDim);
    setColour (juce::TextButton::textColourOnId, AnalogColours::accent);
    setColour (juce::ToggleButton::textColourId, AnalogColours::text);
}

void AnalogLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centre = bounds.getCentre();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmax (2.0f, radius * 0.11f);
    auto arcRadius = radius - lineW * 0.5f;

    // Fondo del arco (traza completa)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                  rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (AnalogColours::gridLine);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Arco de valor actual, color ámbar
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (slider.isEnabled() ? AnalogColours::accent : AnalogColours::textDim);
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Cuerpo del knob
    auto knobRadius = radius * 0.62f;
    g.setColour (AnalogColours::panel);
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    g.setColour (AnalogColours::gridLineMain);
    g.drawEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    // Indicador de posición
    juce::Path pointer;
    auto pointerLength = knobRadius * 0.75f;
    pointer.addRectangle (-1.5f, -knobRadius, 3.0f, pointerLength);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (AnalogColours::accent);
    g.fillPath (pointer);
}

void AnalogLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool highlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    auto on = button.getToggleState();

    g.setColour (on ? AnalogColours::accent.withAlpha (0.18f) : AnalogColours::panel);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (on ? AnalogColours::accent : (highlighted ? AnalogColours::gridLineMain : AnalogColours::gridLine));
    g.drawRoundedRectangle (bounds, 4.0f, 1.2f);

    g.setColour (on ? AnalogColours::accent : AnalogColours::textDim);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
}

void AnalogLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour&, bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    auto baseColour = AnalogColours::panel;
    if (down) baseColour = AnalogColours::accentDim.withAlpha (0.3f);
    else if (highlighted) baseColour = AnalogColours::gridLine;

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (AnalogColours::gridLineMain);
    g.drawRoundedRectangle (bounds, 5.0f, 1.0f);
}

juce::Font AnalogLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (13.0f);
}

juce::Font AnalogLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (13.0f);
}
