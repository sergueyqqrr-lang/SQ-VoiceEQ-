cmake_minimum_required(VERSION 3.22)

project(AnalogAPI_EQ VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Obtiene JUCE automáticamente desde GitHub ---
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AnalogAPI_EQ
    COMPANY_NAME "TuEstudio"
    PLUGIN_MANUFACTURER_CODE Tues
    PLUGIN_CODE Aeq1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Analog API EQ"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "EQ"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

juce_generate_juce_header(AnalogAPI_EQ)

target_sources(AnalogAPI_EQ PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DSP/EQBand.cpp
    Source/DSP/EQBand.h
    Source/DSP/AnalogSaturator.h
    Source/GUI/LookAndFeel.cpp
    Source/GUI/LookAndFeel.h
    Source/GUI/EQCurveComponent.cpp
    Source/GUI/EQCurveComponent.h
    Source/GUI/SpectrumAnalyzer.cpp
    Source/GUI/SpectrumAnalyzer.h
    Source/GUI/BandPanel.cpp
    Source/GUI/BandPanel.h
)

target_compile_definitions(AnalogAPI_EQ PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AnalogAPI_EQ PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
