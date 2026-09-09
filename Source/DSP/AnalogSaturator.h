#pragma once
#include <juce_dsp/juce_dsp.h>

/**
    Etapa de saturación suave que simula el "calor" de una consola analógica:
    genera armónicos pares/impares de bajo orden de forma sutil.
    Se sobremuestrea x2 para reducir aliasing del waveshaper.
*/
class AnalogSaturator
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
            spec.numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
        oversampler->initProcessing (spec.maximumBlockSize);

        waveShaper.functionToUse = [] (float x)
        {
            // Mezcla de tanh (saturación suave, tipo transformador) con un poco
            // de asimetría para generar armónicos pares (calidez "analógica").
            auto y = std::tanh (x * 1.5f);
            auto asym = 0.08f * x * x * (x < 0.0f ? -1.0f : 1.0f);
            return y + asym;
        };
    }

    void reset()
    {
        if (oversampler)
            oversampler->reset();
    }

    // drive: 0.0 (sin efecto) a 1.0 (saturación notoria)
    void process (juce::dsp::AudioBlock<float>& block, float drive)
    {
        if (drive <= 0.001f || oversampler == nullptr)
            return;

        auto driveGain = juce::jmap (drive, 0.0f, 1.0f, 1.0f, 6.0f);
        auto makeupGain = 1.0f / std::sqrt (driveGain);

        block.multiplyBy (driveGain);

        auto oversampledBlock = oversampler->processSamplesUp (block);
        juce::dsp::ProcessContextReplacing<float> ctx (oversampledBlock);
        waveShaper.process (ctx);
        oversampler->processSamplesDown (block);

        block.multiplyBy (makeupGain);
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::WaveShaper<float> waveShaper;
};
