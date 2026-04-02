#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/AmlpEngine.h"
#include "core/LoopManager.h"
#include "core/SceneManager.h"
#include "core/Metronome.h"
#include "core/TimeStretchManager.h"
#include "ui/AmlpLookAndFeel.h"
#include "ui/TransportBar.h"
#include "ui/TrackPanel.h"
#include "ui/SceneBar.h"
#include "ui/LevelMeter.h"
#include "ui/SettingsPanel.h"
#include "ui/PluginBrowserPanel.h"
#include "ui/PluginEditorWindow.h"
#include "effects/EffectsChainManager.h"
#include "core/AILoopClient.h"
#include "ui/AILoopPanel.h"

class MainComponent : public juce::Component,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    AmlpLookAndFeel lookAndFeel;
    AmlpEngine amlpEngine;
    std::unique_ptr<LoopManager> loopManager;
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<Metronome> metronome;

    // UI
    TransportBar transportBar;
    juce::OwnedArray<TrackPanel> trackPanels;
    SceneBar sceneBar;
    juce::TextButton audioSettingsButton { "Audio Settings" };
    juce::Label deviceStatusLabel;
    LevelMeter masterMeter { LevelMeter::Orientation::vertical };
    SettingsPanel settingsPanel;
    PluginBrowserPanel pluginBrowser;
    std::unique_ptr<EffectsChainManager> effectsManager;
    juce::AudioDeviceManager::LevelMeter::Ptr inputLevelMeter;
    int pluginTargetTrack = -1; // -1 = master

    std::unique_ptr<AILoopClient> aiLoopClient;
    AILoopPanel aiLoopPanel;
    int aiTargetTrack = 0;

    int selectedTrack = 0;

    void setupEngine();
    void setupUI();
    void connectCallbacks();
    void updateUI();
    void timerCallback() override;
    void showSettingsPanel();
    void onBpmDetected();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
