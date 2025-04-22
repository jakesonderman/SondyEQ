#include "PluginProcessor.h"
#include "PluginEditor.h"

SondyEQAudioProcessor::SondyEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize with some default bands
    auto lowShelf = std::make_unique<EQBand>();
    lowShelf->setFrequency(100.0f);
    lowShelf->setType(FilterType::LowShelf);
    lowShelf->setGain(0.0f);  // Initialize gain
    bands.push_back(std::move(lowShelf));
    
    auto peak = std::make_unique<EQBand>();
    peak->setFrequency(1000.0f);
    peak->setType(FilterType::Peak);
    peak->setGain(0.0f);  // Initialize gain
    bands.push_back(std::move(peak));
    
    auto highShelf = std::make_unique<EQBand>();
    highShelf->setFrequency(5000.0f);
    highShelf->setType(FilterType::HighShelf);
    highShelf->setGain(0.0f);  // Initialize gain
    bands.push_back(std::move(highShelf));
}

SondyEQAudioProcessor::~SondyEQAudioProcessor()
{
}

const juce::String SondyEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SondyEQAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SondyEQAudioProcessor::producesMidi() const
{
    return false;
}

bool SondyEQAudioProcessor::isMidiEffect() const
{
    return false;
}

double SondyEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SondyEQAudioProcessor::getNumPrograms()
{
    return 1;
}

int SondyEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SondyEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SondyEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SondyEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void SondyEQAudioProcessor::addBand(std::unique_ptr<EQBand> band)
{
    band->prepare(spec);
    bands.push_back(std::move(band));
}

void SondyEQAudioProcessor::removeBand(EQBand* band)
{
    bands.erase(std::remove_if(bands.begin(), bands.end(),
                              [band](const auto& b) { return b.get() == band; }),
                bands.end());
}

void SondyEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize ProcessSpec
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = static_cast<size_t>(getTotalNumOutputChannels());

    // Prepare each band
    for (auto& band : bands)
    {
        band->prepare(spec);
    }
}

void SondyEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this to free up any spare memory, etc.
}

bool SondyEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SondyEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = static_cast<size_t>(getTotalNumInputChannels());
    auto totalNumOutputChannels = static_cast<size_t>(getTotalNumOutputChannels());

    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (static_cast<int>(i), 0, buffer.getNumSamples());

    // Create an AudioBlock for the entire buffer
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process through each band
    for (auto& band : bands)
    {
        band->process(context);
    }
}

bool SondyEQAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SondyEQAudioProcessor::createEditor()
{
    return new SondyEQAudioProcessorEditor (*this);
}

void SondyEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void SondyEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SondyEQAudioProcessor();
}