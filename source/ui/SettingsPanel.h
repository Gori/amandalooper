#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"

class SettingsPanel : public juce::Component
{
public:
    SettingsPanel();

    std::function<void()> onClose;

    bool getAutoMetronome() const;
    bool getThresholdRecording() const;
    int getDefaultBarCount() const;   // 0 = match first loop
    int getCountInBars() const;       // 1 or 2

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::ToggleButton autoMetronomeToggle { "Auto-enable metronome after first recording" };
    juce::ToggleButton thresholdRecordingToggle { "Start first recording on audio threshold" };
    juce::ComboBox defaultBarCountSelector;
    juce::ComboBox countInBarsSelector;
    juce::Label defaultBarCountLabel;
    juce::Label countInBarsLabel;
    juce::Label titleLabel;
    juce::TextButton closeButton { "Close" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPanel)
};
