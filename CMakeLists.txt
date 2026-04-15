cmake_minimum_required(VERSION 3.22)
project(StarFlux VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(StarFlux
    COMPANY_NAME "AkeraSky"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE AkSk
    PLUGIN_CODE StFx
    FORMATS VST3 Standalone
    PRODUCT_NAME "StarFlux"
)

target_sources(StarFlux PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h

    Source/dsp/AudioAnalysis.cpp
    Source/dsp/AudioAnalysis.h
    Source/dsp/AnalysisFrame.h
    Source/dsp/Smoothing.h

    Source/engine/ParameterIDs.h
    Source/engine/StarState.h
    Source/engine/StarFieldEngine.cpp
    Source/engine/StarFieldEngine.h
    Source/engine/MotionEngine.cpp
    Source/engine/MotionEngine.h
    Source/engine/GlitchEngine.cpp
    Source/engine/GlitchEngine.h
    Source/engine/VisualTimeline.cpp
    Source/engine/VisualTimeline.h

    Source/render/ShaderSources.h
    Source/render/GLHelpers.cpp
    Source/render/GLHelpers.h
    Source/render/StarRenderer.cpp
    Source/render/StarRenderer.h

    Source/ui/Theme.h
    Source/ui/MainControls.cpp
    Source/ui/MainControls.h
    Source/ui/AdvancedPanel.cpp
    Source/ui/AdvancedPanel.h
    Source/ui/ToggleButtonRow.cpp
    Source/ui/ToggleButtonRow.h

    Source/CommonIncludes.h
)

target_include_directories(StarFlux PRIVATE
    Source
)

target_compile_definitions(StarFlux
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
	JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(StarFlux
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)