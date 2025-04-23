#include "PluginProcessor.h"
#include "PluginEditor.h"

SondyEQAudioProcessorEditor::SondyEQAudioProcessorEditor (SondyEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up the interface
    eqInterface.setProcessor(&audioProcessor);
    addAndMakeVisible(eqInterface);
    
    // Add a default band if none exist
    if (audioProcessor.getBands().empty()) {
        auto defaultBand = std::make_unique<EQBand>();
        defaultBand->setFrequency(1000.0f);  // 1kHz
        defaultBand->setGain(0.0f);          // 0dB
        defaultBand->setType(FilterType::Peak);
        audioProcessor.addBand(std::move(defaultBand));
    }
    
    // Set initial size and make resizable
    setSize (1000, 600);
    setResizable(true, true);
    setResizeLimits(800, 400, 2000, 1200);
    
    // Make sure the interface fills the editor
    eqInterface.setBounds(getLocalBounds());
    
    // Update the frequency response
    eqInterface.updateBands();
}

SondyEQAudioProcessorEditor::~SondyEQAudioProcessorEditor()
{
}

void SondyEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill background with black
    g.fillAll(juce::Colours::black);
}

void SondyEQAudioProcessorEditor::resized()
{
    // Make sure the interface fills the entire editor window
    eqInterface.setBounds(getLocalBounds());
}

void SondyEQAudioProcessorEditor::processBlock(juce::AudioBuffer<float>& buffer)
{
    // Pass the audio data to the interface for visualization
    eqInterface.process(buffer);
} 