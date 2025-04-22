#include "EQInterface.h"

EQInterface::EQInterface()
{
    addMouseListener(this, true);
}

EQInterface::~EQInterface()
{
}

void EQInterface::paint(juce::Graphics& g)
{
    // Draw background
    g.fillAll(juce::Colours::black);
    
    // Draw grid lines
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
    
    // Create dynamic gradient based on bands
    if (!bands.empty())
    {
        juce::ColourGradient gradient;
        
        // Add gradient stops for each band
        for (size_t i = 0; i < bands.size(); ++i)
        {
            const auto& band = bands[i];
            float x = frequencyToX(band->getFrequency());
            float y = gainToY(band->getGain());
            
            // Calculate color based on band type and gain
            juce::Colour bandColor;
            switch (band->getType())
            {
                case FilterType::LowShelf:
                    bandColor = juce::Colours::blue;
                    break;
                case FilterType::HighShelf:
                    bandColor = juce::Colours::red;
                    break;
                case FilterType::Peak:
                    bandColor = juce::Colours::green;
                    break;
                case FilterType::Notch:
                    bandColor = juce::Colours::yellow;
                    break;
                case FilterType::LowPass:
                    bandColor = juce::Colours::cyan;
                    break;
                case FilterType::HighPass:
                    bandColor = juce::Colours::magenta;
                    break;
            }
            
            // Adjust color brightness based on gain
            float brightness = juce::jmap(band->getGain(), minGain, maxGain, 0.3f, 1.0f);
            bandColor = bandColor.withBrightness(brightness);
            
            // Add gradient stop
            gradient.addColour(static_cast<float>(i) / (bands.size() - 1), bandColor);
        }
        
        // If we only have one band, add a second stop to create a gradient
        if (bands.size() == 1)
        {
            gradient.addColour(1.0f, bands[0]->getType() == FilterType::Peak ? 
                             juce::Colours::green : juce::Colours::blue);
        }
        
        // Set up gradient
        gradient.point1 = { 0.0f, 0.0f };
        gradient.point2 = { static_cast<float>(getWidth()), 0.0f };
        gradient.isRadial = false;
        
        // Draw frequency response curve with dynamic gradient
        g.setGradientFill(gradient);
        g.strokePath(frequencyResponsePath, juce::PathStrokeType(3.0f));
    }
    else
    {
        // Default gradient when no bands are present
        juce::ColourGradient defaultGradient(
            juce::Colours::cyan.withAlpha(0.8f),
            0.0f, 0.0f,
            juce::Colours::magenta.withAlpha(0.8f),
            static_cast<float>(getWidth()), 0.0f,
            false);
        g.setGradientFill(defaultGradient);
        g.strokePath(frequencyResponsePath, juce::PathStrokeType(3.0f));
    }
    
    // Draw bands with improved visibility
    for (const auto& band : bands)
    {
        juce::Point<float> pos = band->getPosition();
        float x = frequencyToX(band->getFrequency());
        float y = gainToY(band->getGain());
        
        // Draw band handle with color matching the curve
        juce::Colour bandColor;
        switch (band->getType())
        {
            case FilterType::LowShelf:
                bandColor = juce::Colours::blue;
                break;
            case FilterType::HighShelf:
                bandColor = juce::Colours::red;
                break;
            case FilterType::Peak:
                bandColor = juce::Colours::green;
                break;
            case FilterType::Notch:
                bandColor = juce::Colours::yellow;
                break;
            case FilterType::LowPass:
                bandColor = juce::Colours::cyan;
                break;
            case FilterType::HighPass:
                bandColor = juce::Colours::magenta;
                break;
        }
        
        // Adjust color brightness based on gain
        float brightness = juce::jmap(band->getGain(), minGain, maxGain, 0.3f, 1.0f);
        bandColor = bandColor.withBrightness(brightness);
        
        g.setColour(band.get() == selectedBand ? bandColor.brighter(0.5f) : bandColor);
        g.fillEllipse(x - 6, y - 6, 12, 12);
        
        // Draw band outline
        g.setColour(juce::Colours::black);
        g.drawEllipse(x - 6, y - 6, 12, 12, 1.0f);
        
        // Draw frequency and gain labels
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        
        // Frequency label
        juce::String freqText = juce::String(band->getFrequency(), 0) + " Hz";
        g.drawText(freqText, x - 30, y - 25, 60, 20, juce::Justification::centred);
        
        // Gain label
        juce::String gainText = juce::String(band->getGain(), 1) + " dB";
        g.drawText(gainText, x - 30, y + 5, 60, 20, juce::Justification::centred);
    }
}

void EQInterface::resized()
{
    updateFrequencyResponse();
}

void EQInterface::mouseDown(const juce::MouseEvent& e)
{
    selectedBand = nullptr;
    
    for (const auto& band : bands)
    {
        juce::Point<float> pos = band->getPosition();
        float x = frequencyToX(band->getFrequency());
        float y = gainToY(band->getGain());
        
        if (e.position.getDistanceFrom(juce::Point<float>(x, y)) < 8.0f)
        {
            selectedBand = band.get();
            repaint();
            break;
        }
    }
}

void EQInterface::mouseDoubleClick(const juce::MouseEvent& e)
{
    addBand(e.position);
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
    if (selectedBand != nullptr)
    {
        selectedBand = nullptr;
        repaint();
    }
}

void EQInterface::addBand(const juce::Point<float>& position)
{
    auto newBand = std::make_unique<EQBand>();
    newBand->setSampleRate(sampleRate);
    updateBandPosition(newBand.get(), position);
    bands.push_back(std::move(newBand));
    updateFrequencyResponse();
    repaint();
}

void EQInterface::removeBand(EQBand* band)
{
    bands.erase(std::remove_if(bands.begin(), bands.end(),
                              [band](const auto& b) { return b.get() == band; }),
                bands.end());
    updateFrequencyResponse();
    repaint();
}

void EQInterface::updateBandPosition(EQBand* band, const juce::Point<float>& newPosition)
{
    float freq = xToFrequency(newPosition.x);
    float gain = yToGain(newPosition.y);
    
    band->setFrequency(freq);
    band->setGain(gain);
    band->setPosition(newPosition);
    
    updateFrequencyResponse();
    repaint();
}

void EQInterface::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
    for (auto& band : bands)
    {
        band->setSampleRate(sampleRate);
    }
    updateFrequencyResponse();
}

void EQInterface::process(juce::AudioBuffer<float>& buffer)
{
    for (auto& band : bands)
    {
        band->process(buffer);
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
    // Sum the responses from all bands
    float totalGain = 0.0f;
    
    for (const auto& band : bands)
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