#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogAPIEQAudioProcessor::AnalogAPIEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

AnalogAPIEQAudioProcessor::~AnalogAPIEQAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AnalogAPIEQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray typeChoices { "High Pass", "Low Shelf", "Bell", "High Shelf", "Low Pass", "Notch" };

    for (int i = 0; i < maxBands; ++i)
    {
        // Solo las 2 primeras bandas activas por defecto, para arrancar simple
        bool defaultActive = (i < 2);
        float defaultFreq = (i == 0) ? 100.0f : (i == 1 ? 3000.0f : 1000.0f);
        int defaultType = (i == 0) ? (int) EQBand::Type::HighPass
                         : (i == 1) ? (int) EQBand::Type::Bell
                         : (int) EQBand::Type::Bell;

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            getBandActiveParamID (i), "Band " + juce::String (i + 1) + " Active", defaultActive));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            getBandTypeParamID (i), "Band " + juce::String (i + 1) + " Type", typeChoices, defaultType));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            getBandFreqParamID (i), "Band " + juce::String (i + 1) + " Freq",
            juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), defaultFreq));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            getBandGainParamID (i), "Band " + juce::String (i + 1) + " Gain",
            juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            getBandQParamID (i), "Band " + juce::String (i + 1) + " Q",
            juce::NormalisableRange<float> (0.1f, 18.0f, 0.01f, 0.4f), 0.7f));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            getBandProportionalParamID (i), "Band " + juce::String (i + 1) + " Proportional Q", true));
    }

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "saturation", "Analog Saturation", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.15f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "outputGain", "Output Gain", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> ("bypass", "Bypass", false));

    return { params.begin(), params.end() };
}

void AnalogAPIEQAudioProcessor::updateBandFromParameters (int i)
{
    bool active = apvts.getRawParameterValue (getBandActiveParamID (i))->load() > 0.5f;
    if (! active)
        return;

    int typeIdx = (int) apvts.getRawParameterValue (getBandTypeParamID (i))->load();
    float freq  = apvts.getRawParameterValue (getBandFreqParamID (i))->load();
    float gain  = apvts.getRawParameterValue (getBandGainParamID (i))->load();
    float q     = apvts.getRawParameterValue (getBandQParamID (i))->load();
    bool prop   = apvts.getRawParameterValue (getBandProportionalParamID (i))->load() > 0.5f;

    bands[(size_t) i].update ((EQBand::Type) typeIdx, freq, gain, q, prop);
}

void AnalogAPIEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();

    for (auto& band : bands)
        band.prepare (spec);

    saturator.prepare (spec);

    for (int i = 0; i < maxBands; ++i)
        updateBandFromParameters (i);
}

void AnalogAPIEQAudioProcessor::releaseResources() {}

bool AnalogAPIEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void AnalogAPIEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    bool bypass = apvts.getRawParameterValue ("bypass")->load() > 0.5f;

    for (int i = 0; i < maxBands; ++i)
        updateBandFromParameters (i);

    if (! bypass)
    {
        juce::dsp::AudioBlock<float> block (buffer);

        for (int i = 0; i < maxBands; ++i)
        {
            bool active = apvts.getRawParameterValue (getBandActiveParamID (i))->load() > 0.5f;
            if (active)
                bands[(size_t) i].process (block);
        }

        float drive = apvts.getRawParameterValue ("saturation")->load();
        saturator.process (block, drive);

        float outGainDb = apvts.getRawParameterValue ("outputGain")->load();
        block.multiplyBy (juce::Decibels::decibelsToGain (outGainDb));
    }

    // Alimenta el FIFO del analizador de espectro (mono, canal 0) para la GUI
    auto numSamples = buffer.getNumSamples();
    auto* channelData = buffer.getReadPointer (0);
    auto scope = audioFifo.write (numSamples);

    if (scope.blockSize1 > 0)
        fifoBuffer.copyFrom (0, scope.startIndex1, channelData, scope.blockSize1);
    if (scope.blockSize2 > 0)
        fifoBuffer.copyFrom (0, scope.startIndex2, channelData + scope.blockSize1, scope.blockSize2);
}

juce::AudioProcessorEditor* AnalogAPIEQAudioProcessor::createEditor()
{
    return new AnalogAPIEQAudioProcessorEditor (*this);
}

void AnalogAPIEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AnalogAPIEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// Punto de entrada requerido por JUCE para crear el plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogAPIEQAudioProcessor();
}
