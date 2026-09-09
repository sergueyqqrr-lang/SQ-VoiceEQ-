#pragma once
#include <juce_dsp/juce_dsp.h>

/**
    Representa UNA banda de EQ (bell, shelf, HP/LP, notch).
    Implementa un modelo aproximado de "Proportional Q" estilo API:
    en los EQs API reales (ej. 550/560), al aumentar la ganancia de boost/cut
    el ancho de banda se ESTRECHA automáticamente (el Q sube), imitando
    el comportamiento de las bobinas e inductores analógicos reales.
    Esto es una aproximación matemática razonable, no un modelo de circuito.
*/
class EQBand
{
public:
    enum class Type
    {
        HighPass = 0,
        LowShelf,
        Bell,
        HighShelf,
        LowPass,
        Notch
    };

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        for (auto& f : filters)
            f.prepare (spec);
        sampleRate = spec.sampleRate;
    }

    void reset()
    {
        for (auto& f : filters)
            f.reset();
    }

    // Calcula el Q efectivo aplicando el modelo "proportional Q" si está activo
    static float getEffectiveQ (float baseQ, float gainDb, bool proportional)
    {
        if (! proportional)
            return baseQ;

        // A mayor |gainDb|, mayor Q (banda más estrecha), como en consolas API.
        // A 0 dB de ganancia el Q queda igual al base; a +/-24dB puede casi triplicarse.
        auto factor = 1.0f + (std::abs (gainDb) / 24.0f) * 2.0f;
        return juce::jlimit (0.1f, 18.0f, baseQ * factor);
    }

    void update (Type type, float freqHz, float gainDb, float q, bool proportionalQ)
    {
        currentType = type;
        currentFreq = freqHz;
        currentGainDb = gainDb;
        currentQ = q;
        currentProportional = proportionalQ;

        auto effectiveQ = getEffectiveQ (q, gainDb, proportionalQ);
        auto gainLinear = juce::Decibels::decibelsToGain (gainDb);

        juce::dsp::IIR::Coefficients<float>::Ptr coeffs;

        switch (type)
        {
            case Type::HighPass:
                coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freqHz, effectiveQ);
                break;
            case Type::LowShelf:
                coeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf (sampleRate, freqHz, effectiveQ, gainLinear);
                break;
            case Type::Bell:
                coeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter (sampleRate, freqHz, effectiveQ, gainLinear);
                break;
            case Type::HighShelf:
                coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, freqHz, effectiveQ, gainLinear);
                break;
            case Type::LowPass:
                coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, freqHz, effectiveQ);
                break;
            case Type::Notch:
                coeffs = juce::dsp::IIR::Coefficients<float>::makeNotch (sampleRate, freqHz, effectiveQ);
                break;
        }

        currentCoefficients = coeffs;
        for (auto& f : filters)
            f.coefficients = coeffs;
    }

    void process (juce::dsp::AudioBlock<float>& block)
    {
        for (size_t ch = 0; ch < block.getNumChannels() && ch < filters.size(); ++ch)
        {
            auto singleChannel = block.getSingleChannelBlock (ch);
            juce::dsp::ProcessContextReplacing<float> ctx (singleChannel);
            filters[ch].process (ctx);
        }
    }

    // Magnitud (en dB) de la respuesta de esta banda a una frecuencia dada, para dibujar la curva
    float getMagnitudeForFrequency (double freqHz) const
    {
        if (currentCoefficients == nullptr)
            return 0.0f;

        return (float) juce::Decibels::gainToDecibels (
            currentCoefficients->getMagnitudeForFrequency (freqHz, sampleRate));
    }

    Type getType() const noexcept          { return currentType; }
    float getFrequency() const noexcept    { return currentFreq; }
    float getGainDb() const noexcept       { return currentGainDb; }
    float getQ() const noexcept            { return currentQ; }
    bool isProportional() const noexcept   { return currentProportional; }

private:
    std::array<juce::dsp::IIR::Filter<float>, 2> filters;
    juce::dsp::IIR::Coefficients<float>::Ptr currentCoefficients;
    double sampleRate = 44100.0;

    Type currentType = Type::Bell;
    float currentFreq = 1000.0f;
    float currentGainDb = 0.0f;
    float currentQ = 1.0f;
    bool currentProportional = true;
};
