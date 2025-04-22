#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "EQBand.h"

// Forward declaration
class SondyEQAudioProcessor;

class EQInterface : public juce::Component
{
public:
    EQInterface();
    ~EQInterface() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    
    void setProcessor(SondyEQAudioProcessor* processor) { audioProcessor = processor; }
    void updateBands();
    
    void setSampleRate(double newSampleRate);
    void process(juce::AudioBuffer<float>& buffer);
    
    float calculateTotalGain(float frequency) const;

private:
    SondyEQAudioProcessor* audioProcessor = nullptr;
    EQBand* selectedBand = nullptr;
    double sampleRate = 44100.0;
    
    juce::Path frequencyResponsePath;
    void updateFrequencyResponse();
    
    float frequencyToX(float freq) const;
    float gainToY(float gain) const;
    float xToFrequency(float x) const;
    float yToGain(float y) const;
    
    const float minFrequency = 20.0f;
    const float maxFrequency = 20000.0f;
    const float minGain = -24.0f;
    const float maxGain = 24.0f;
    
    void addBand(const juce::Point<float>& position);
    void removeBand(EQBand* band);
    void updateBandPosition(EQBand* band, const juce::Point<float>& newPosition);
}; 