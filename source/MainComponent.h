#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/AmlpEngine.h"
#include "core/LoopManager.h"
#include "core/SceneManager.h"
#include "core/Metronome.h"
#include "midi/TapTempoHandler.h"
#include "ui/AmlpLookAndFeel.h"
#include "ui/TransportBar.h"
#include "ui/TrackPanel.h"
#include "ui/SceneBar.h"
#include "ui/LevelMeter.h"

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    // Engine
    AmlpLookAndFeel lookAndFeel;
    AmlpEngine amlpEngine;
    std::unique_ptr<LoopManager> loopManager;
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<Metronome> metronome;
    TapTempoHandler tapTempo;

    // UI
    TransportBar transportBar;
    juce::OwnedArray<TrackPanel> trackPanels;
    SceneBar sceneBar;
    juce::TextButton audioSettingsButton { "Audio Settings" };
    juce::Label deviceStatusLabel;
    LevelMeter masterMeter { LevelMeter::Orientation::vertical };
    juce::AudioDeviceManager::LevelMeter::Ptr inputLevelMeter;

    // Selected track for operations
    int selectedTrack = 0;

    void setupEngine();
    void setupUI();
    void connectCallbacks();
    void updateUI();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
