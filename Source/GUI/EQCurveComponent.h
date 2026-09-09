#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "SpectrumAnalyzer.h"

/**
    Panel central: rejilla de frecuencia/ganancia, curva de respuesta combinada,
    nodos arrastrables por banda (estilo FabFilter Pro-Q) y analizador de espectro de fondo.

    Interacción:
      - Arrastrar nodo verticalmente -> ganancia
      - Arrastrar nodo horizontalmente -> frecuencia
      - Rueda del mouse sobre un nodo -> Q
      - Doble click en la curva -> activa la siguiente banda libre en esa freq/gain
      - Click derecho en un nodo -> menú (tipo de filtro, proportional Q, desactivar)
*/
class EQCurveComponent : public juce::Component, private juce::Timer
{
public:
    explicit EQCurveComponent (AnalogAPIEQAudioProcessor& proc);
    ~EQCurveComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;

    float freqToX (float freq) const;
    float xToFreq (float x) const;
    float gainToY (float gain) const;
    float yToGain (float y) const;

    int findNodeUnder (juce::Point<float> pos) const;
    void drawGrid (juce::Graphics&);
    void drawResponseCurve (juce::Graphics&);
    void drawNodes (juce::Graphics&);

    AnalogAPIEQAudioProcessor& processor;
    SpectrumAnalyzer spectrumAnalyzer;

    int draggingBand = -1;
    juce::Rectangle<float> plotArea;

    static constexpr float minFreq = 20.0f;
    static constexpr float maxFreq = 20000.0f;
    static constexpr float minGain = -24.0f;
    static constexpr float maxGain = 24.0f;
};
