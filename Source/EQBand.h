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

enum class FilterType
{
    LowShelf,
    HighShelf,
    Peak,
    Notch,
    LowPass,
    HighPass
};

class EQBand
{
public:
    EQBand();
    ~EQBand();

    void prepare(double sampleRate);
    void process(juce::AudioBuffer<float>& buffer);
    
    void setFrequency(float newFrequency);
    void setGain(float newGain);
    void setQ(float newQ);
    void setType(FilterType newType);
    
    float getFrequency() const { return frequency; }
    float getGain() const { return gain; }
    float getQ() const { return q; }
    FilterType getType() const { return type; }
    
    juce::Point<float> getPosition() const { return position; }
    void setPosition(juce::Point<float> newPosition) { position = newPosition; }
    
    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; updateFilter(); }

    float calculateGain(float frequency) const;

private:
    FilterType type;
    float frequency;
    float gain;
    float q;
    double sampleRate;
    juce::Point<float> position;
    
    juce::dsp::IIR::Filter<float> filter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filterProcessor;
    
    void updateFilter();
}; 