#include "EQInterface.h"
#include "PluginProcessor.h"

EQInterface::EQInterface()
{
    addMouseListener(this, true);
    // Initialize FFT analyzer with 2 channels and order 11 (2048 samples)
    fftAnalyzer = std::make_unique<SondyFFT::MultiChannelFFTSpectrumAnalyzer>(2, 11);
    spectrumComponent = std::make_unique<SondyFFT::MultiChannelSpectrumComponent>(*fftAnalyzer);
    spectrumComponent->setOverlayMode(true); // Overlay the channels
    addAndMakeVisible(spectrumComponent.get());
    
    // Start the timer for updates - reduced from 30Hz to 15Hz for smoother updates
    startTimerHz(15);
}

EQInterface::~EQInterface()
{
    stopTimer();
    spectrumComponent = nullptr;
    fftAnalyzer = nullptr;
}

void EQInterface::timerCallback()
{
    // Update the frequency response and display
    updateFrequencyResponse();
    repaint();
}

void EQInterface::updateBands()
{
    if (audioProcessor)
    {
        updateFrequencyResponse();
        repaint();
    }
}

void EQInterface::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::black);

    // Draw FFT spectrum first (background)
    if (spectrumComponent)
    {
        // Create a temporary graphics context with reduced opacity
        juce::Graphics::ScopedSaveState state(g);
        g.setOpacity(0.25f);  // Slightly reduced opacity for more subtle visualization
        spectrumComponent->paint(g);
    }

    // Draw grid lines
    drawGridLines(g);

    // Update and draw frequency response
    if (audioProcessor && !audioProcessor->getBands().empty())
    {
        updateFrequencyResponse();
        
        // Draw frequency response curve with solid white first (for visibility)
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.strokePath(frequencyResponsePath, juce::PathStrokeType(2.0f));
        
        // Then draw with gradient
        juce::ColourGradient gradient;
        const auto& processorBands = audioProcessor->getBands();
        
        // Add gradient stops for each band
        for (size_t i = 0; i < processorBands.size(); ++i)
        {
            const auto& band = processorBands[i];
            juce::Colour bandColor;
            switch (band->getType())
            {
                case FilterType::LowShelf:   bandColor = juce::Colours::blue; break;
                case FilterType::HighShelf:  bandColor = juce::Colours::red; break;
                case FilterType::Peak:       bandColor = juce::Colours::green; break;
                case FilterType::Notch:      bandColor = juce::Colours::yellow; break;
                case FilterType::LowPass:    bandColor = juce::Colours::cyan; break;
                case FilterType::HighPass:   bandColor = juce::Colours::magenta; break;
            }
            
            float brightness = juce::jmap(band->getGain(), minGain, maxGain, 0.3f, 1.0f);
            bandColor = bandColor.withBrightness(brightness);
            gradient.addColour(static_cast<float>(i) / (processorBands.size() - 1), bandColor);
        }
        
        if (processorBands.size() == 1)
        {
            gradient.addColour(1.0f, processorBands[0]->getType() == FilterType::Peak ? 
                             juce::Colours::green : juce::Colours::blue);
        }
        
        gradient.point1 = { 0.0f, 0.0f };
        gradient.point2 = { static_cast<float>(getWidth()), 0.0f };
        gradient.isRadial = false;
        
        g.setGradientFill(gradient);
        g.strokePath(frequencyResponsePath, juce::PathStrokeType(2.0f));
        
        // Draw band nodes
        for (const auto& band : processorBands)
        {
            float x = frequencyToX(band->getFrequency());
            float y = gainToY(band->getGain());
            
            juce::Colour bandColor;
            switch (band->getType())
            {
                case FilterType::LowShelf:   bandColor = juce::Colours::blue; break;
                case FilterType::HighShelf:  bandColor = juce::Colours::red; break;
                case FilterType::Peak:       bandColor = juce::Colours::green; break;
                case FilterType::Notch:      bandColor = juce::Colours::yellow; break;
                case FilterType::LowPass:    bandColor = juce::Colours::cyan; break;
                case FilterType::HighPass:   bandColor = juce::Colours::magenta; break;
            }
            
            float brightness = juce::jmap(band->getGain(), minGain, maxGain, 0.3f, 1.0f);
            bandColor = bandColor.withBrightness(brightness);
            
            // Draw the band node
            g.setColour(band.get() == selectedBand ? bandColor.brighter(0.5f) : bandColor);
            g.fillEllipse(x - 6, y - 6, 12, 12);
            
            // Draw band outline
            g.setColour(juce::Colours::black);
            g.drawEllipse(x - 6, y - 6, 12, 12, 1.0f);
            
            // Draw frequency and gain labels
            g.setColour(juce::Colours::white);
            g.setFont(12.0f);
            
            juce::String freqText = juce::String(band->getFrequency(), 0) + " Hz";
            g.drawText(freqText, x - 30, y - 25, 60, 20, juce::Justification::centred);
            
            juce::String gainText = juce::String(band->getGain(), 1) + " dB";
            g.drawText(gainText, x - 30, y + 5, 60, 20, juce::Justification::centred);
        }
    }
}

void EQInterface::resized()
{
    // Make the spectrum component fill the entire interface
    if (spectrumComponent)
    {
        spectrumComponent->setBounds(getLocalBounds());
    }
    updateFrequencyResponse();
}

void EQInterface::mouseDown(const juce::MouseEvent& e)
{
    if (audioProcessor)
    {
        // First check if we clicked on an existing band
        for (const auto& band : audioProcessor->getBands())
        {
            float x = frequencyToX(band->getFrequency());
            float y = gainToY(band->getGain());
            
            if (e.position.getDistanceFrom(juce::Point<float>(x, y)) < 8.0f)
            {
                selectedBand = band.get();
                repaint();
                return;  // Exit early if we found a band
            }
        }
        
        // If we didn't click on a band, deselect the current band
        selectedBand = nullptr;
        repaint();
    }
}

void EQInterface::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (audioProcessor)
    {
        // First check if we're clicking near an existing band
        for (const auto& band : audioProcessor->getBands())
        {
            float x = frequencyToX(band->getFrequency());
            float y = gainToY(band->getGain());
            
            if (e.position.getDistanceFrom(juce::Point<float>(x, y)) < 8.0f)
            {
                // If we're near an existing band, don't create a new one
                return;
            }
        }
        
        // Create new band only if we're not near any existing bands
        auto newBand = std::make_unique<EQBand>();
        float freq = xToFrequency(e.position.x);
        float gain = yToGain(e.position.y);
        
        newBand->setFrequency(freq);
        newBand->setGain(gain);
        newBand->setPosition(e.position);
        
        // Store the raw pointer before moving the unique_ptr
        auto* bandPtr = newBand.get();
        audioProcessor->addBand(std::move(newBand));
        
        // Set the newly created band as selected
        selectedBand = bandPtr;
        
        // Update the display
        updateFrequencyResponse();
        repaint();
    }
}

void EQInterface::mouseDrag(const juce::MouseEvent& e)
{
    if (selectedBand != nullptr)
    {
        // Constrain the position to the component bounds
        juce::Point<float> constrainedPos = e.position;
        constrainedPos.x = juce::jlimit(0.0f, static_cast<float>(getWidth()), constrainedPos.x);
        constrainedPos.y = juce::jlimit(0.0f, static_cast<float>(getHeight()), constrainedPos.y);
        
        updateBandPosition(selectedBand, constrainedPos);
    }
}

void EQInterface::mouseUp(const juce::MouseEvent&)
{
    // Keep the band selected until the next mouseDown
    repaint();
}

void EQInterface::addBand(const juce::Point<float>& position)
{
    if (audioProcessor)
    {
        // First check if we're adding near an existing band
        for (const auto& band : audioProcessor->getBands())
        {
            float x = frequencyToX(band->getFrequency());
            float y = gainToY(band->getGain());
            
            if (position.getDistanceFrom(juce::Point<float>(x, y)) < 8.0f)
            {
                // If we're near an existing band, don't create a new one
                return;
            }
        }
        
        auto newBand = std::make_unique<EQBand>();
        float freq = xToFrequency(position.x);
        float gain = yToGain(position.y);
        
        newBand->setFrequency(freq);
        newBand->setGain(gain);
        newBand->setPosition(position);
        
        audioProcessor->addBand(std::move(newBand));
        
        // Update the display
        updateFrequencyResponse();
        repaint();
    }
}

void EQInterface::removeBand(EQBand* band)
{
    if (audioProcessor)
    {
        audioProcessor->removeBand(band);
        updateBands();
    }
}

void EQInterface::updateBandPosition(EQBand* band, const juce::Point<float>& newPosition)
{
    if (audioProcessor)
    {
        float freq = xToFrequency(newPosition.x);
        float gain = yToGain(newPosition.y);
        
        band->setFrequency(freq);
        band->setGain(gain);
        band->setPosition(newPosition);
        
        updateFrequencyResponse();
        repaint();
    }
}

void EQInterface::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    if (audioProcessor)
    {
        for (auto& band : audioProcessor->getBands())
        {
            band->setSampleRate(sampleRate);
        }
        updateFrequencyResponse();
    }
}

void EQInterface::process(juce::AudioBuffer<float>& buffer)
{
    if (fftAnalyzer)
    {
        // Process audio through FFT analyzer
        fftAnalyzer->processAudioBuffer(buffer);
    }
}

void EQInterface::updateFrequencyResponse()
{
    frequencyResponsePath.clear();
    
    // Start at the minimum frequency
    float startFreq = 20.0f;
    float startGain = calculateTotalGain(startFreq);
    frequencyResponsePath.startNewSubPath(frequencyToX(startFreq), gainToY(startGain));
    
    // Use logarithmically spaced points for frequency
    const int numPoints = 200;
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    
    for (int i = 1; i < numPoints; ++i)
    {
        // Calculate frequency point (logarithmic spacing)
        float t = static_cast<float>(i) / (numPoints - 1);
        float freq = minFreq * std::pow(maxFreq/minFreq, t);
        
        // Calculate the total gain at this frequency
        float totalGain = calculateTotalGain(freq);
        
        // Convert to screen coordinates
        float x = frequencyToX(freq);
        float y = gainToY(totalGain);
        
        // Add point to path
        frequencyResponsePath.lineTo(x, y);
    }
}

float EQInterface::calculateTotalGain(float frequency) const
{
    if (!audioProcessor)
        return 0.0f;

    // Sum the responses from all bands
    float totalGain = 0.0f;
    
    for (const auto& band : audioProcessor->getBands())
    {
        totalGain += band->calculateGain(frequency);
    }
    
    // Ensure the total gain stays within our display limits
    return juce::jlimit(minGain, maxGain, totalGain);
}

float EQInterface::frequencyToX(float freq) const
{
    return std::log(freq / minFrequency) / std::log(maxFrequency / minFrequency) * getWidth();
}

float EQInterface::gainToY(float gain) const
{
    // Map gain from [minGain, maxGain] to [0, height]
    // Add a small margin at the top and bottom
    const float margin = 20.0f;
    const float range = maxGain - minGain;
    const float normalizedGain = (gain - minGain) / range;
    return margin + (1.0f - normalizedGain) * (getHeight() - 2.0f * margin);
}

float EQInterface::xToFrequency(float x) const
{
    return minFrequency * std::pow(maxFrequency / minFrequency, x / getWidth());
}

float EQInterface::yToGain(float y) const
{
    return minGain + (1.0f - y / getHeight()) * (maxGain - minGain);
}

void EQInterface::drawGridLines(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    
    // Frequency grid lines (logarithmic)
    for (float freq = 100.0f; freq <= 10000.0f; freq *= 10.0f)
    {
        float x = frequencyToX(freq);
        g.drawLine(x, 0, x, getHeight());
    }
    
    // Gain grid lines
    for (float gain = minGain; gain <= maxGain; gain += 6.0f)
    {
        float y = gainToY(gain);
        g.drawLine(0, y, getWidth(), y);
    }
} 