#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "EQInterface.h"

class SondyEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SondyEQAudioProcessorEditor (SondyEQAudioProcessor&);
    ~SondyEQAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SondyEQAudioProcessor& audioProcessor;
    EQInterface eqInterface;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SondyEQAudioProcessorEditor)
};