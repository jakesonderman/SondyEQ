/*
  ==============================================================================

    fft.h
    Created: 3 Feb 2025 7:21:52pm
    Author:  Jake Sonderman

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>


namespace SondyFFT {

class FFTSpectrumAnalyzer
{
public:
    /** Constructor.
     @param fftOrder_ The FFT order. The FFT size will be 2^fftOrder_.
     */
    FFTSpectrumAnalyzer (int fftOrder_)
    : fftOrder (fftOrder_),
    fftSize (1 << fftOrder_),
    fft (fftOrder_),
    window (fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        // Allocate space for FFT data.
        // We use a buffer of size 2*fftSize because JUCE's performRealOnlyForwardTransform expects that.
        fftData.resize (2 * fftSize, 0.0f);
        fifoBuffer.resize (fftSize, 0.0f);
    }
    
    /** Pushes the next sample into the internal FIFO.
     When the FIFO is full, an FFT is performed.
     */
    void pushNextSample (float sample)
    {
        fifoBuffer[fifoIndex++] = sample;
        
        if (fifoIndex == fftSize)
        {
            // Copy FIFO to fftData and apply windowing.
            std::copy (fifoBuffer.begin(), fifoBuffer.end(), fftData.begin());
            window.multiplyWithWindowingTable (fftData.data(), fftSize);
            
            // Perform the FFT in-place. The output is stored in fftData.
            fft.performRealOnlyForwardTransform (fftData.data());
            
            // Mark that new FFT data is available.
            newFFTDataAvailable = true;
            
            // Reset FIFO index for the next block.
            fifoIndex = 0;
        }
    }
    
    /** Returns true if new FFT data is available.
     Typically, your GUI thread might query this flag.
     */
    bool isNewDataAvailable() const
    {
        return newFFTDataAvailable;
    }
    
    /** Resets the new data flag.
     Call this after processing the current FFT data.
     */
    void resetNewDataFlag()
    {
        newFFTDataAvailable = false;
    }
    
    /** Returns the magnitude (or amplitude) for a given frequency bin.
     Note: For a real-input FFT performed with performRealOnlyForwardTransform,
     the format of the output data is a little different:
     - bin 0 is stored in fftData[0] (real only),
     - bin fftSize/2 is stored in fftData[1] (real only),
     - bins 1 .. (fftSize/2 - 1) are stored as real/imaginary pairs.
     */
    float getMagnitudeForBin (int binIndex) const
    {
        jassert (binIndex >= 0 && binIndex <= fftSize / 2);
        
        float magnitude;
        
        if (binIndex == 0)
            magnitude = std::abs(fftData[0]);
        else if (binIndex == fftSize / 2)
            magnitude = std::abs(fftData[1]);
        else
        {
            int realIndex = binIndex * 2;
            int imagIndex = realIndex + 1;
            float real = fftData[realIndex];
            float imag = fftData[imagIndex];
            
            magnitude = std::sqrt(real * real + imag * imag);
        }
        
        // Scale the magnitude by the FFT size to normalize
        return magnitude * 2.0f / fftSize;
    }
    
    /** Returns the size of the FFT (number of input samples per FFT).
     */
    int getFFTSize() const { return fftSize; }
    
    /** (Optional) Returns the entire FFT data vector.
     You might use this if you want to process or visualize all bins at once.
     */
    const std::vector<float>& getFFTData() const { return fftData; }
    
private:
    int fftOrder;
    int fftSize;
    
    // JUCE FFT and windowing objects.
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    
    // Buffers for input and FFT data.
    std::vector<float> fifoBuffer;
    std::vector<float> fftData;
    
    int fifoIndex { 0 };
    bool newFFTDataAvailable { false };
};

class SpectrumComponent : public juce::Component
{
public:
    /** Constructor.
     @param fftAnalyzer A reference to your FFT analyzer instance.
     */
    SpectrumComponent(){
        analyzer = nullptr;
    }
    SpectrumComponent (FFTSpectrumAnalyzer* fftAnalyzerPointer)
    : analyzer (fftAnalyzerPointer)
    {
    }
    
    ~SpectrumComponent() override
    {
    }
    
    void ConnectFFT(FFTSpectrumAnalyzer* fftAnalyzerPointer){
        analyzer = fftAnalyzerPointer;
    }
    
    /** Paints the FFT spectrum. */
    void paint (juce::Graphics& g) override
    {
        if (!analyzer) return;  // Safety check
        
        auto bounds = getLocalBounds();
        const int width  = bounds.getWidth();
        const int height = bounds.getHeight();
        
        // Get FFT parameters
        const int fftSize = analyzer->getFFTSize();
        const int numBins = fftSize / 2;
        
        juce::Path spectrumPath;
        bool firstPoint = true;
        
        // Iterate through the frequency bins
        for (int bin = 0; bin < numBins; ++bin)
        {
            // Get the magnitude for this bin
            float magnitude = analyzer->getMagnitudeForBin(bin);
            
            // Convert magnitude to decibels with a reference level adjustment
            float dB = juce::Decibels::gainToDecibels(magnitude, -100.0f);
            
            // Normalize the dB value to a 0...1 range with adjusted range
            float normalizedMagnitude = juce::jlimit(0.0f, 1.0f, (dB + 100.0f) / 100.0f);
            
            // Map the bin index to a frequency
            float freq = juce::mapToLog10(static_cast<float>(bin) / numBins, 20.0f, 20000.0f);
            
            // Map frequency to x position (logarithmic)
            float x = juce::mapFromLog10(freq, 20.0f, 20000.0f) * width;
            
            // Map the normalized magnitude to a y position with adjusted range
            float y = (1.0f - normalizedMagnitude) * height;
            
            if (firstPoint)
            {
                spectrumPath.startNewSubPath(x, y);
                firstPoint = false;
            }
            else
            {
                spectrumPath.lineTo(x, y);
            }
        }
        
        // Draw the path with increased opacity and thickness
        g.setColour(juce::Colours::white.withAlpha(1.0f));  // Increased opacity
        g.strokePath(spectrumPath, juce::PathStrokeType(2.0f));  // Increased thickness
    }
    
private:
    FFTSpectrumAnalyzer* analyzer;
};

class MultiChannelFFTSpectrumAnalyzer
{
public:
    /** Constructor.
        @param numChannels_ The number of audio channels.
        @param fftOrder_ The FFT order (FFT size = 2^fftOrder_).
    */
    MultiChannelFFTSpectrumAnalyzer (int numChannels_, int fftOrder_)
        : numChannels (numChannels_),
          fftOrder (fftOrder_),
          fftSize (1 << fftOrder_),
          sampleRate(44100.0f)  // Default sample rate
    {
        for (int i = 0; i < numChannels; ++i)
        {
            analyzers.push_back(std::make_unique<FFTSpectrumAnalyzer>(fftOrder));
        }
    }

    void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
    }

    float getSampleRate() const { return sampleRate; }

    /** Pushes one sample for a specific channel into its analyzer. */
    void pushNextSample (int channel, float sample)
    {
        jassert (channel < numChannels);
        analyzers[channel]->pushNextSample(sample);
    }

    /** Convenience method: Processes an entire audio buffer.
        It pushes each sample of each channel (up to numChannels) into the corresponding analyzer.
    */
    void processAudioBuffer (const juce::AudioBuffer<float>& buffer)
    {
        const int channels = buffer.getNumChannels();
        const int samples = buffer.getNumSamples();
        const int processChannels = juce::jmin (channels, numChannels);

        for (int ch = 0; ch < processChannels; ++ch)
        {
            const float* data = buffer.getReadPointer (ch);
            for (int i = 0; i < samples; ++i)
                pushNextSample (ch, data[i]);
        }
    }

    /** Returns the FFT analyzer for a given channel. */
    FFTSpectrumAnalyzer& getAnalyzer (int channel)
    {
        jassert (channel < numChannels);
        return *analyzers[channel];
    }
    
    const FFTSpectrumAnalyzer& getAnalyzer (int channel) const
    {
        jassert (channel < numChannels);
        return *analyzers[channel];
    }

    int getNumChannels() const { return numChannels; }
    int getFFTSize() const { return fftSize; }

    float getMagnitudeForBin(int channel, int binIndex) const
    {
        if (channel >= 0 && channel < numChannels)
        {
            return analyzers[channel]->getMagnitudeForBin(binIndex);
        }
        return 0.0f;
    }

private:
    int numChannels;
    int fftOrder;
    int fftSize;
    float sampleRate;
    std::vector<std::unique_ptr<FFTSpectrumAnalyzer>> analyzers;
};

class MultiChannelSpectrumComponent : public juce::Component
{
public:
    /** Constructor.
        @param analyzerRef A reference to the multi-channel FFT analyzer.
    */
    MultiChannelSpectrumComponent (MultiChannelFFTSpectrumAnalyzer& analyzerRef)
        : analyzer (analyzerRef)
    {
    }

    ~MultiChannelSpectrumComponent() override
    {
    }

    void setOverlayMode (bool overlay)
    {
        overlayMode = overlay;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float sampleRate = analyzer.getSampleRate();
        
        for (int channel = 0; channel < analyzer.getNumChannels(); ++channel)
        {
            juce::Path fftPath;
            bool pathStarted = false;
            
            for (int i = 0; i < analyzer.getFFTSize() / 2; ++i)
            {
                // Map bin index to x position using logarithmic scale
                const float binFreq = (float)i * sampleRate / analyzer.getFFTSize();
                const float normX = std::log10(1 + binFreq) / std::log10(1 + sampleRate / 2);
                const float x = normX * bounds.getWidth();
                
                // Get magnitude and convert to decibels
                const float magnitude = analyzer.getMagnitudeForBin(channel, i);
                const float decibels = 20.0f * std::log10(magnitude);
                
                // Map decibels to y position
                const float normY = juce::jmap(decibels, -60.0f, 0.0f, 0.0f, 1.0f);
                const float y = bounds.getHeight() * (1.0f - juce::jlimit(0.0f, 1.0f, normY));
                
                if (!pathStarted)
                {
                    fftPath.startNewSubPath(x, y);
                    pathStarted = true;
                }
                else
                {
                    fftPath.lineTo(x, y);
                }
            }
            
            g.setColour(getChannelColour(channel));
            g.strokePath(fftPath, juce::PathStrokeType(2.0f));
        }
    }

    juce::Colour getChannelColour(int channel) const
    {
        return channel == 0 ? juce::Colours::cyan : juce::Colours::magenta;
    }

private:
    MultiChannelFFTSpectrumAnalyzer& analyzer;
    bool overlayMode = true;
};



}
