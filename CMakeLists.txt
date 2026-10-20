cmake_minimum_required(VERSION 3.22)
project(TrapVoxAI VERSION 0.2.0)

# Set JUCE_DIR to your local JUCE folder when configuring, or add JUCE as a submodule.
if(NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "Set JUCE_DIR to the path of your JUCE checkout, e.g. -DJUCE_DIR=C:/JUCE")
endif()
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(TrapVoxAI
    COMPANY_NAME "TrapVox"
    BUNDLE_ID "com.trapvox.ai"
    PLUGIN_MANUFACTURER_CODE TrVx
    PLUGIN_CODE Tv02
    FORMATS VST3
    PRODUCT_NAME "TrapVox AI"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(TrapVoxAI)

target_sources(TrapVoxAI PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_features(TrapVoxAI PRIVATE cxx_std_17)
target_compile_definitions(TrapVoxAI PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(TrapVoxAI PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
