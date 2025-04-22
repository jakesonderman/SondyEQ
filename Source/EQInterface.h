#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_dsp/juce_dsp.h>
#include "EQBand.h"

class EQInterface : public juce::Component
{
public:
    EQInterface();
    ~EQInterface() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    void addBand(const juce::Point<float>& position);
    void removeBand(EQBand* band);
    void updateBandPosition(EQBand* band, const juce::Point<float>& newPosition);
    
    std::vector<std::unique_ptr<EQBand>>& getBands() { return bands; }
    
    void setSampleRate(double newSampleRate);
    void process(juce::AudioBuffer<float>& buffer);
    
    float calculateTotalGain(float frequency) const;

private:
    std::vector<std::unique_ptr<EQBand>> bands;
    EQBand* selectedBand = nullptr;
    double sampleRate = 44100.0;
    
    juce::Path frequencyResponsePath;
    void updateFrequencyResponse();
    
    float frequencyToX(float freq) const;
    float gainToY(float gain) const;
    float xToFrequency(float x) const;
    float yToGain(float y) const;
    
    static constexpr float minFrequency = 20.0f;
    static constexpr float maxFrequency = 20000.0f;
    static constexpr float minGain = -24.0f;
    static constexpr float maxGain = 24.0f;
}; 