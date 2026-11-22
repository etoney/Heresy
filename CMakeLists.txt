cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
project(Heresy VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use a local checkout if JUCE_DIR is given, otherwise download it.
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

option(HERESY_PLUGIN "Build the plugin" ON)
option(HERESY_TESTS "Build the headless render test" OFF)

if(HERESY_PLUGIN)
    juce_add_plugin(Heresy
        COMPANY_NAME "Castle Danger Music Company"
        COMPANY_WEBSITE "https://castledangermusic.com"
        COMPANY_COPYRIGHT "Castle Danger Music Company"
        BUNDLE_ID com.castledangermusic.heresy
        PLUGIN_MANUFACTURER_CODE Cdmc
        PLUGIN_CODE Hrsy
        PRODUCT_NAME "Heresy"
        DESCRIPTION "Horror sound design synth"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        FORMATS AU VST3 Standalone
        AU_MAIN_TYPE kAudioUnitType_MusicDevice
        VST3_CATEGORIES Instrument Synth)

    juce_add_binary_data(HeresyData SOURCES
        assets/fonts/Anton-Regular.ttf
        assets/fonts/Oswald-Variable.ttf
        assets/fonts/CourierPrime-Bold.ttf)

    target_sources(Heresy PRIVATE
        Source/Engine.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

    target_compile_definitions(Heresy PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

    target_link_libraries(Heresy
        PRIVATE
            HeresyData
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags)

    # UI snapshot tool (links the plugin's shared code)
    juce_add_console_app(HeresySnapshot PRODUCT_NAME "HeresySnapshot")
    target_sources(HeresySnapshot PRIVATE tests/snapshot.cpp)
    target_include_directories(HeresySnapshot PRIVATE Source)
    target_compile_definitions(HeresySnapshot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(HeresySnapshot PRIVATE Heresy)
endif()

if(HERESY_TESTS)
    juce_add_console_app(HeresyRender PRODUCT_NAME "HeresyRender")
    target_sources(HeresyRender PRIVATE tests/render.cpp Source/Engine.cpp)
    target_include_directories(HeresyRender PRIVATE Source)
    target_compile_definitions(HeresyRender PRIVATE JUCE_USE_CURL=0 JUCE_WEB_BROWSER=0)
    target_link_libraries(HeresyRender PRIVATE juce::juce_dsp juce::juce_audio_formats
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endif()
