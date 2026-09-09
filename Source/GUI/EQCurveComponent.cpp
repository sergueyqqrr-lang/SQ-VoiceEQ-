#include "EQCurveComponent.h"
#include "LookAndFeel.h"

EQCurveComponent::EQCurveComponent (AnalogAPIEQAudioProcessor& proc)
    : processor (proc), spectrumAnalyzer (proc)
{
    addAndMakeVisible (spectrumAnalyzer);
    startTimerHz (30);
    setWantsKeyboardFocus (false);
}

EQCurveComponent::~EQCurveComponent() { stopTimer(); }

void EQCurveComponent::resized()
{
    plotArea = getLocalBounds().toFloat().reduced (8.0f, 8.0f);
    spectrumAnalyzer.setBounds (getLocalBounds());
}

void EQCurveComponent::timerCallback() { repaint(); }

float EQCurveComponent::freqToX (float freq) const
{
    auto logMin = std::log10 (minFreq);
    auto logMax = std::log10 (maxFreq);
    auto logF = std::log10 (juce::jlimit (minFreq, maxFreq, freq));
    return plotArea.getX() + (logF - logMin) / (logMax - logMin) * plotArea.getWidth();
}

float EQCurveComponent::xToFreq (float x) const
{
    auto logMin = std::log10 (minFreq);
    auto logMax = std::log10 (maxFreq);
    auto prop = juce::jlimit (0.0f, 1.0f, (x - plotArea.getX()) / plotArea.getWidth());
    return std::pow (10.0f, logMin + prop * (logMax - logMin));
}

float EQCurveComponent::gainToY (float gain) const
{
    auto prop = (gain - minGain) / (maxGain - minGain);
    return plotArea.getBottom() - prop * plotArea.getHeight();
}

float EQCurveComponent::yToGain (float y) const
{
    auto prop = juce::jlimit (0.0f, 1.0f, (plotArea.getBottom() - y) / plotArea.getHeight());
    return minGain + prop * (maxGain - minGain);
}

void EQCurveComponent::paint (juce::Graphics& g)
{
    g.fillAll (AnalogColours::background);
    drawGrid (g);
    drawResponseCurve (g);
    drawNodes (g);
}

void EQCurveComponent::drawGrid (juce::Graphics& g)
{
    const float freqLines[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    g.setFont (10.0f);

    for (auto f : freqLines)
    {
        auto x = freqToX (f);
        bool major = (f == 100 || f == 1000 || f == 10000);
        g.setColour (major ? AnalogColours::gridLineMain : AnalogColours::gridLine);
        g.drawVerticalLine ((int) x, plotArea.getY(), plotArea.getBottom());

        g.setColour (AnalogColours::textDim);
        juce::String label = f >= 1000 ? juce::String (f / 1000.0f, (f == 1000 || f == 10000) ? 0 : 1) + "k"
                                        : juce::String ((int) f);
        g.drawText (label, (int) x - 15, (int) plotArea.getBottom() - 14, 30, 12, juce::Justification::centred);
    }

    for (float gdb = minGain; gdb <= maxGain; gdb += 6.0f)
    {
        auto y = gainToY (gdb);
        g.setColour (juce::approximatelyEqual (gdb, 0.0f) ? AnalogColours::gridLineMain : AnalogColours::gridLine);
        g.drawHorizontalLine ((int) y, plotArea.getX(), plotArea.getRight());

        g.setColour (AnalogColours::textDim);
        g.drawText (juce::String ((int) gdb), (int) plotArea.getX() + 2, (int) y - 12, 30, 12, juce::Justification::left);
    }
}

void EQCurveComponent::drawResponseCurve (juce::Graphics& g)
{
    auto& bands = processor.getBands();
    auto sr = processor.getCurrentSampleRate();

    juce::Path path;
    const int numPoints = 300;

    for (int i = 0; i < numPoints; ++i)
    {
        float prop = (float) i / (float) (numPoints - 1);
        float x = plotArea.getX() + prop * plotArea.getWidth();
        float freq = xToFreq (x);

        float totalDb = 0.0f;
        for (int b = 0; b < AnalogAPIEQAudioProcessor::maxBands; ++b)
        {
            bool active = processor.apvts.getRawParameterValue (
                AnalogAPIEQAudioProcessor::getBandActiveParamID (b))->load() > 0.5f;
            if (active)
                totalDb += bands[(size_t) b].getMagnitudeForFrequency (freq);
        }

        float y = gainToY (juce::jlimit (minGain, maxGain, totalDb));
        if (i == 0) path.startNewSubPath (x, y);
        else path.lineTo (x, y);
    }

    g.setColour (AnalogColours::curve);
    g.strokePath (path, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Relleno sutil bajo la curva para dar sensación "analógica" cálida
    juce::Path fillPath = path;
    fillPath.lineTo (plotArea.getRight(), gainToY (0.0f));
    fillPath.lineTo (plotArea.getX(), gainToY (0.0f));
    fillPath.closeSubPath();
    g.setColour (AnalogColours::curve.withAlpha (0.08f));
    g.fillPath (fillPath);
}

void EQCurveComponent::drawNodes (juce::Graphics& g)
{
    auto& bands = processor.getBands();

    for (int b = 0; b < AnalogAPIEQAudioProcessor::maxBands; ++b)
    {
        bool active = processor.apvts.getRawParameterValue (
            AnalogAPIEQAudioProcessor::getBandActiveParamID (b))->load() > 0.5f;
        if (! active) continue;

        auto freq = bands[(size_t) b].getFrequency();
        auto gain = bands[(size_t) b].getGainDb();

        auto x = freqToX (freq);
        auto y = gainToY (gain);

        auto colour = juce::Colour::fromHSV ((float) b / (float) AnalogAPIEQAudioProcessor::maxBands,
                                              0.55f, 1.0f, 1.0f);
        bool isDragging = (draggingBand == b);
        float radius = isDragging ? 9.0f : 7.0f;

        g.setColour (colour.withAlpha (0.25f));
        g.fillEllipse (x - radius - 3, y - radius - 3, (radius + 3) * 2, (radius + 3) * 2);

        g.setColour (colour);
        g.fillEllipse (x - radius, y - radius, radius * 2, radius * 2);
        g.setColour (AnalogColours::background);
        g.drawEllipse (x - radius, y - radius, radius * 2, radius * 2, 1.5f);

        g.setColour (colour);
        g.setFont (11.0f);
        g.drawText (juce::String (b + 1), (int) x - 10, (int) y - 22, 20, 14, juce::Justification::centred);
    }
}

int EQCurveComponent::findNodeUnder (juce::Point<float> pos) const
{
    auto& bands = processor.getBands();
    for (int b = 0; b < AnalogAPIEQAudioProcessor::maxBands; ++b)
    {
        bool active = processor.apvts.getRawParameterValue (
            AnalogAPIEQAudioProcessor::getBandActiveParamID (b))->load() > 0.5f;
        if (! active) continue;

        auto x = freqToX (bands[(size_t) b].getFrequency());
        auto y = gainToY (bands[(size_t) b].getGainDb());
        if (pos.getDistanceFrom ({ x, y }) < 12.0f)
            return b;
    }
    return -1;
}

void EQCurveComponent::mouseDown (const juce::MouseEvent& e)
{
    auto node = findNodeUnder (e.position);

    if (e.mods.isRightButtonDown() && node >= 0)
    {
        juce::PopupMenu menu;
        menu.addItem (1, "High Pass");
        menu.addItem (2, "Low Shelf");
        menu.addItem (3, "Bell");
        menu.addItem (4, "High Shelf");
        menu.addItem (5, "Low Pass");
        menu.addItem (6, "Notch");
        menu.addSeparator();
        bool prop = processor.apvts.getRawParameterValue (
            AnalogAPIEQAudioProcessor::getBandProportionalParamID (node))->load() > 0.5f;
        menu.addItem (7, "Proportional Q (API style)", true, prop);
        menu.addSeparator();
        menu.addItem (8, "Desactivar banda");

        menu.showMenuAsync (juce::PopupMenu::Options(), [this, node] (int result)
        {
            if (result >= 1 && result <= 6)
            {
                auto* p = processor.apvts.getParameter (AnalogAPIEQAudioProcessor::getBandTypeParamID (node));
                p->setValueNotifyingHost (p->convertTo0to1 ((float) (result - 1)));
            }
            else if (result == 7)
            {
                auto id = AnalogAPIEQAudioProcessor::getBandProportionalParamID (node);
                bool cur = processor.apvts.getRawParameterValue (id)->load() > 0.5f;
                auto* p = processor.apvts.getParameter (id);
                p->setValueNotifyingHost (cur ? 0.0f : 1.0f);
            }
            else if (result == 8)
            {
                auto* p = processor.apvts.getParameter (AnalogAPIEQAudioProcessor::getBandActiveParamID (node));
                p->setValueNotifyingHost (0.0f);
            }
        });
        return;
    }

    draggingBand = node;
}

void EQCurveComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (draggingBand < 0) return;

    auto freq = xToFreq (e.position.x);
    auto gain = yToGain (e.position.y);

    auto freqId = AnalogAPIEQAudioProcessor::getBandFreqParamID (draggingBand);
    auto gainId = AnalogAPIEQAudioProcessor::getBandGainParamID (draggingBand);

    processor.apvts.getParameter (freqId)->setValueNotifyingHost (
        processor.apvts.getParameter (freqId)->convertTo0to1 (freq));
    processor.apvts.getParameter (gainId)->setValueNotifyingHost (
        processor.apvts.getParameter (gainId)->convertTo0to1 (gain));
}

void EQCurveComponent::mouseUp (const juce::MouseEvent&) { draggingBand = -1; }

void EQCurveComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    // Busca la primera banda inactiva y la activa en la posición del click
    for (int b = 0; b < AnalogAPIEQAudioProcessor::maxBands; ++b)
    {
        auto activeId = AnalogAPIEQAudioProcessor::getBandActiveParamID (b);
        bool active = processor.apvts.getRawParameterValue (activeId)->load() > 0.5f;
        if (active) continue;

        auto freq = xToFreq (e.position.x);
        auto gain = yToGain (e.position.y);

        processor.apvts.getParameter (activeId)->setValueNotifyingHost (1.0f);

        auto freqId = AnalogAPIEQAudioProcessor::getBandFreqParamID (b);
        auto gainId = AnalogAPIEQAudioProcessor::getBandGainParamID (b);
        processor.apvts.getParameter (freqId)->setValueNotifyingHost (
            processor.apvts.getParameter (freqId)->convertTo0to1 (freq));
        processor.apvts.getParameter (gainId)->setValueNotifyingHost (
            processor.apvts.getParameter (gainId)->convertTo0to1 (gain));
        return;
    }
}

void EQCurveComponent::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    auto node = findNodeUnder (e.position);
    if (node < 0) return;

    auto qId = AnalogAPIEQAudioProcessor::getBandQParamID (node);
    auto* param = processor.apvts.getParameter (qId);
    auto currentQ = processor.apvts.getRawParameterValue (qId)->load();
    auto newQ = juce::jlimit (0.1f, 18.0f, currentQ + wheel.deltaY * 2.0f);
    param->setValueNotifyingHost (param->convertTo0to1 (newQ));
}
