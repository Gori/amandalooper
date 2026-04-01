#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"
#include "WaveformDisplay.h"
#include "LevelMeter.h"

class TrackPanel : public juce::Component
{
public:
    enum class RecordMode { replace, overdub, newLoop };

    explicit TrackPanel (int trackIndex);

    // Callbacks
    std::function<void()> onArm;
    std::function<void()> onMute;
    std::function<void()> onSolo;
    std::function<void (float)> onVolumeChange;
    std::function<void (RecordMode)> onModeChanged;
    std::function<void (int)> onLoopSelected;

    void setTrackName (const juce::String& name);
    juce::String getTrackName() const;
    void setArmed (bool armed);
    void setMuted (bool muted);
    void setSoloed (bool soloed);
    void setVolume (float normalizedVolume);
    void setRecording (bool recording);
    void setPlaying (bool playing);
    void setOverdubbing (bool overdubbing);
    void setInputLevel (float left, float right);
    void setOutputLevel (float left, float right);
    void setMode (RecordMode mode);
    void setLoopList (const juce::StringArray& names, int activeIndex);

    RecordMode getMode() const { return currentMode; }
    WaveformDisplay& getWaveformDisplay() { return waveformDisplay; }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Colour trackColour;
    RecordMode currentMode = RecordMode::newLoop;

    // Header
    juce::Label nameLabel;
    juce::TextButton replaceButton { "Replace" };
    juce::TextButton overdubButton { "Overdub" };
    juce::TextButton newLoopButton { "New" };
    juce::TextButton armButton { "R" };
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };
    juce::Slider volumeSlider;

    // Waveform
    WaveformDisplay waveformDisplay;

    // Loop selector
    juce::ComboBox loopDropdown;

    // Level meters
    LevelMeter inputMeter { LevelMeter::Orientation::vertical };
    LevelMeter outputMeter { LevelMeter::Orientation::vertical };

    // State
    juce::Colour stateColour = AmlpLookAndFeel::getIdleColour();

    void updateModeButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackPanel)
};
