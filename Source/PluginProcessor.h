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
#include "EQInterface.h"
#include "EQBand.h"

class SondyEQAudioProcessor : public juce::AudioProcessor
{
public:
    SondyEQAudioProcessor();
    ~SondyEQAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    EQInterface& getEQInterface() { return eqInterface; }

    // Public access to bands for the editor
    std::vector<std::unique_ptr<EQBand>>& getBands() { return bands; }
    void addBand(std::unique_ptr<EQBand> band);
    void removeBand(EQBand* band);

private:
    EQInterface eqInterface;
    std::vector<std::unique_ptr<EQBand>> bands;
    juce::dsp::ProcessSpec spec;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SondyEQAudioProcessor)
};