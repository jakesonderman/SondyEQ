#include "PluginProcessor.h"
#include "PluginEditor.h"

SondyEQAudioProcessor::SondyEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
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

void SondyEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    eqInterface.setSampleRate(sampleRate);
}

void SondyEQAudioProcessor::releaseResources()
{
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

void SondyEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    eqInterface.process(buffer);
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