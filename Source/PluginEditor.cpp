#include "PluginProcessor.h"
#include "PluginEditor.h"

SondyEQAudioProcessorEditor::SondyEQAudioProcessorEditor (SondyEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    eqInterface.setProcessor(&audioProcessor);
    addAndMakeVisible(eqInterface);
    setSize (800, 400);
}

SondyEQAudioProcessorEditor::~SondyEQAudioProcessorEditor()
{
}

void SondyEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void SondyEQAudioProcessorEditor::resized()
{
    eqInterface.setBounds(getLocalBounds());
} 