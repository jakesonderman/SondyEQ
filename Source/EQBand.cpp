#include "EQBand.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

EQBand::EQBand()
    : type(FilterType::Peak)
    , frequency(1000.0f)
    , gain(0.0f)
    , q(1.0f)
    , sampleRate(44100.0)
    , position(0.5f, 0.5f)
{
}

EQBand::~EQBand()
{
}

void EQBand::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    filterProcessor.prepare(spec);
    updateFilter();
    filterProcessor.reset();
}

void EQBand::process(juce::dsp::ProcessContextReplacing<float>& context)
{
    filterProcessor.process(context);
}

void EQBand::setFrequency(float newFrequency)
{
    frequency = newFrequency;
    updateFilter();
}

void EQBand::setGain(float newGain)
{
    gain = newGain;
    updateFilter();
}

void EQBand::setQ(float newQ)
{
    q = newQ;
    updateFilter();
}

void EQBand::setType(FilterType newType)
{
    type = newType;
    updateFilter();
}

void EQBand::updateFilter()
{
    switch (type)
    {
        case FilterType::LowShelf:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
                sampleRate, frequency, q, juce::Decibels::decibelsToGain(gain));
            break;
            
        case FilterType::HighShelf:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
                sampleRate, frequency, q, juce::Decibels::decibelsToGain(gain));
            break;
            
        case FilterType::Peak:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                sampleRate, frequency, q, juce::Decibels::decibelsToGain(gain));
            break;
            
        case FilterType::Notch:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makeNotch(
                sampleRate, frequency, q);
            break;
            
        case FilterType::LowPass:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
                sampleRate, frequency, q);
            break;
            
        case FilterType::HighPass:
            *filterProcessor.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, frequency, q);
            break;
    }
}

float EQBand::calculateGain(float frequency) const
{
    // Frequency ratio (input frequency / filter frequency)
    const float freqRatio = frequency / this->frequency;
    
    // Calculate response based on filter type
    switch (type)
    {
        case FilterType::Peak:
        {
            // Peak filter response using a simplified resonant filter equation
            float response = gain / (1.0f + std::pow((freqRatio - 1.0f/freqRatio) / q, 2));
            return response;
        }
        
        case FilterType::LowShelf:
        {
            // Butterworth lowshelf response
            float response = gain / (1.0f + std::pow(freqRatio / q, 2));
            return response;
        }
        
        case FilterType::HighShelf:
        {
            // Butterworth highshelf response
            float response = gain / (1.0f + std::pow(1.0f / (freqRatio * q), 2));
            return response;
        }
        
        case FilterType::LowPass:
        {
            // -12 dB/octave lowpass response
            float response = -12.0f * std::log2(std::max(freqRatio, 0.001f));
            return std::min(0.0f, response);
        }
        
        case FilterType::HighPass:
        {
            // -12 dB/octave highpass response
            float response = -12.0f * std::log2(std::max(1.0f/freqRatio, 0.001f));
            return std::min(0.0f, response);
        }
        
        case FilterType::Notch:
        {
            // Notch filter response
            float bw = 1.0f / q;  // bandwidth
            float response = -gain * (1.0f / (1.0f + std::pow((freqRatio - 1.0f/freqRatio) / bw, 2)));
            return response;
        }
        
        default:
            return 0.0f;
    }
} 