cmake_minimum_required(VERSION 3.22)

project(FalconEcho2026 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_DIR "E:/SDKs/JUCE" CACHE PATH "Path to JUCE")
add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(FalconEcho2026
    COMPANY_NAME "Falcon Eye Corporation"
    BUNDLE_ID com.falconeyecorporation.falconecho2026
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Flcn
    PLUGIN_CODE Tqgq
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/icon.png"
    FORMATS VST3 Standalone
    PRODUCT_NAME "Falcon Echo 2026")

target_sources(FalconEcho2026 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(FalconEcho2026 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_compile_definitions(FalconEcho2026 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    "FALCON_PRODUCT_ID=\"FalconEcho2026\""
    "FALCON_LICENSE_URL=\"\""
    FALCON_LICENSE_DISABLED=1
    FALCON_KNOB_STYLE=0
    FALCON_BG_THEME=0)

target_link_libraries(FalconEcho2026
    PRIVATE
        juce::juce_audio_utils
        juce::juce_cryptography
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
