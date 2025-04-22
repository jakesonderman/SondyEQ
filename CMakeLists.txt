cmake_minimum_required(VERSION 3.15)

project(SondyEQ VERSION 1.0.0)

# Set JUCE path
set(JUCE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/modules" CACHE STRING "JUCE modules path")
set(JUCE_EXAMPLES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/examples" CACHE STRING "JUCE examples path")
set(JUCE_EXTRAS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/extras" CACHE STRING "JUCE extras path")

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Create the plugin target
juce_add_plugin(SondyEQ
    VERSION 1.0.0
    COMPANY_NAME "Sondy"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Sndy
    PLUGIN_CODE Seq1
    FORMATS VST3
    PRODUCT_NAME "SondyEQ")

# Add source files
target_sources(SondyEQ
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/EQBand.cpp
        Source/EQInterface.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.h
        Source/EQBand.h
        Source/EQInterface.h)

# Add include directories
target_include_directories(SondyEQ
    PRIVATE
        Source
        ${JUCE_MODULE_PATH})

# Link against JUCE modules
target_link_libraries(SondyEQ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_core
        juce::juce_dsp)