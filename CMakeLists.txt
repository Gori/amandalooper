cmake_minimum_required(VERSION 3.22...3.31)

file(READ tracktion_engine/VERSION.md TRACKTION_VERSION)
project(AMLP VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# macOS deployment target
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE INTERNAL "")
endif()

# Add JUCE (our own submodule at 8.0.12)
add_subdirectory(JUCE)

# Add Tracktion Engine modules (uses juce_add_modules from JUCE above)
add_subdirectory(tracktion_engine/modules)

# Add libkeyfinder
add_subdirectory(libs/libkeyfinder)

# IDE source groups
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Main application target
juce_add_gui_app(AMLP
    BUNDLE_ID              com.amlp.looper
    PRODUCT_NAME           "AMLP"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "AMLP needs microphone access for audio recording"
    PLUGINHOST_AU          TRUE)

juce_generate_juce_header(AMLP)

target_compile_features(AMLP PRIVATE cxx_std_20)

set_target_properties(AMLP PROPERTIES
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden)

target_sources(AMLP PRIVATE
    source/Main.cpp
    source/MainComponent.h
    source/MainComponent.cpp
    source/core/AmlpEngine.h
    source/core/AmlpEngine.cpp
    source/core/AmlpUIBehaviour.h
    source/core/LoopTrack.h
    source/core/LoopTrack.cpp
    source/core/LoopManager.h
    source/core/LoopManager.cpp
    source/core/OverdubStack.h
    source/core/OverdubStack.cpp
    source/core/TempoDetector.h
    source/core/TempoDetector.cpp
    source/core/KeyDetector.h
    source/core/KeyDetector.cpp
    source/core/LoopLengthController.h
    source/core/LoopLengthController.cpp
    source/core/QuantizeManager.h
    source/core/QuantizeManager.cpp
    source/core/SceneManager.h
    source/core/SceneManager.cpp
    source/core/ProjectManager.h
    source/core/ProjectManager.cpp
    source/core/BounceEngine.h
    source/core/BounceEngine.cpp
    source/midi/MidiMapper.h
    source/midi/MidiMapper.cpp
    source/midi/MidiInputHandler.h
    source/midi/MidiInputHandler.cpp
    source/midi/TapTempoHandler.h
    source/midi/TapTempoHandler.cpp
    source/effects/EffectsChainManager.h
    source/effects/EffectsChainManager.cpp
    source/core/Metronome.h
    source/core/Metronome.cpp
    source/core/InputRouter.h
    source/core/InputRouter.cpp
    source/core/TimeStretchManager.h
    source/core/TimeStretchManager.cpp
    source/core/AudioFileImporter.h
    source/core/AudioFileImporter.cpp
    source/ui/AmlpLookAndFeel.h
    source/ui/AmlpLookAndFeel.cpp
    source/ui/WaveformDisplay.h
    source/ui/WaveformDisplay.cpp
    source/ui/TransportBar.h
    source/ui/TransportBar.cpp
    source/ui/TrackPanel.h
    source/ui/TrackPanel.cpp
    source/ui/SceneBar.h
    source/ui/SceneBar.cpp
    source/ui/LevelMeter.h
    source/ui/LevelMeter.cpp
    source/ui/SettingsPanel.h
    source/ui/SettingsPanel.cpp
    source/ui/PluginBrowserPanel.h
    source/ui/PluginBrowserPanel.cpp
    source/ui/PluginEditorWindow.h
    source/core/AILoopClient.h
    source/core/AILoopClient.cpp
    source/ui/AILoopPanel.h
    source/ui/AILoopPanel.cpp)

file(COPY scripts/ai_loop_server.py scripts/setup_ai_env.py
     DESTINATION ${CMAKE_BINARY_DIR}/AMLP_artefacts/AMLP.app/Contents/Resources/)

target_include_directories(AMLP PRIVATE source libs/libkeyfinder/src)

target_compile_definitions(AMLP PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_PLUGINHOST_AU=1
    JUCE_PLUGINHOST_VST3=1
    TRACKTION_ENABLE_TIMESTRETCH_SOUNDTOUCH=1
    DONT_SET_USING_JUCE_NAMESPACE=1)

target_link_libraries(AMLP PRIVATE
    tracktion::tracktion_core
    tracktion::tracktion_engine
    tracktion::tracktion_graph
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_osc
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    keyfinder)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_options(AMLP PRIVATE "-fno-aligned-allocation")
endif()

# Test target
add_subdirectory(tests)
