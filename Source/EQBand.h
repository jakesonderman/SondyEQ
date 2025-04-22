#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

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

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(juce::dsp::ProcessContextReplacing<float>& context);
    
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
    
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> filterProcessor;
    
    void updateFilter();
}; 