#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/EQBand.h"
#include "DSP/AnalogSaturator.h"

class AnalogAPIEQAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int maxBands = 12;

    AnalogAPIEQAudioProcessor();
    ~AnalogAPIEQAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Analog API EQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Acceso directo para la GUI (curva de respuesta, spectrum, etc.)
    std::array<EQBand, maxBands>& getBands() { return bands; }
    double getCurrentSampleRate() const { return currentSampleRate; }

    // FIFO simple para que el analizador de espectro de la GUI pueda leer audio post-proceso
    static constexpr int fftOrder = 11; // 2048 puntos
    static constexpr int fftSize = 1 << fftOrder;
    juce::AbstractFifo audioFifo { fftSize * 4 };
    juce::AudioBuffer<float> fifoBuffer { 1, fftSize * 4 };

    static juce::String getBandActiveParamID (int index)         { return "band" + juce::String (index) + "_active"; }
    static juce::String getBandTypeParamID (int index)           { return "band" + juce::String (index) + "_type"; }
    static juce::String getBandFreqParamID (int index)           { return "band" + juce::String (index) + "_freq"; }
    static juce::String getBandGainParamID (int index)           { return "band" + juce::String (index) + "_gain"; }
    static juce::String getBandQParamID (int index)              { return "band" + juce::String (index) + "_q"; }
    static juce::String getBandProportionalParamID (int index)   { return "band" + juce::String (index) + "_prop"; }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateBandFromParameters (int index);

    std::array<EQBand, maxBands> bands;
    AnalogSaturator saturator;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogAPIEQAudioProcessor)
};
