juce_add_console_app(AMLPTests
    PRODUCT_NAME "AMLPTests")

juce_generate_juce_header(AMLPTests)

target_compile_features(AMLPTests PRIVATE cxx_std_20)

target_sources(AMLPTests PRIVATE
    TestMain.cpp
    OverdubStackTest.cpp
    TempoDetectorTest.cpp
    LoopManagerTest.cpp
    SceneManagerTest.cpp
    ProjectManagerTest.cpp
    MidiMapperTest.cpp
    TapTempoTest.cpp
    EffectsChainTest.cpp
    # Source files under test
    ${CMAKE_SOURCE_DIR}/source/core/OverdubStack.cpp
    ${CMAKE_SOURCE_DIR}/source/core/TempoDetector.cpp
    ${CMAKE_SOURCE_DIR}/source/core/LoopTrack.cpp
    ${CMAKE_SOURCE_DIR}/source/core/LoopManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/LoopLengthController.cpp
    ${CMAKE_SOURCE_DIR}/source/core/QuantizeManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/SceneManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/ProjectManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/BounceEngine.cpp
    ${CMAKE_SOURCE_DIR}/source/midi/MidiMapper.cpp
    ${CMAKE_SOURCE_DIR}/source/midi/MidiInputHandler.cpp
    ${CMAKE_SOURCE_DIR}/source/midi/TapTempoHandler.cpp
    ${CMAKE_SOURCE_DIR}/source/effects/EffectsChainManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/Metronome.cpp
    ${CMAKE_SOURCE_DIR}/source/core/InputRouter.cpp
    ${CMAKE_SOURCE_DIR}/source/core/TimeStretchManager.cpp
    ${CMAKE_SOURCE_DIR}/source/core/AudioFileImporter.cpp)

target_include_directories(AMLPTests PRIVATE
    ${CMAKE_SOURCE_DIR}/source)

target_compile_definitions(AMLPTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_UNIT_TESTS=1
    DONT_SET_USING_JUCE_NAMESPACE=1)

target_link_libraries(AMLPTests PRIVATE
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
    juce::juce_recommended_warning_flags)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_options(AMLPTests PRIVATE "-fno-aligned-allocation")
endif()
