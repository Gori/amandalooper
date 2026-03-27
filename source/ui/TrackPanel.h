#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"
#include "WaveformDisplay.h"
#include "LevelMeter.h"

/**
    A single horizontal track panel — hardware looper style.
    Contains: track name, arm/mute/solo buttons, waveform display,
    slot selector, and volume fader with level meter.
*/
class TrackPanel : public juce::Component
{
public:
    explicit TrackPanel (int trackIndex);

    // Callbacks
    std::function<void()> onArm;
    std::function<void()> onMute;
    std::function<void()> onSolo;
    std::function<void (int)> onSlotSelected;
    std::function<void (float)> onVolumeChange;

    void setTrackName (const juce::String& name);
    void setArmed (bool armed);
    void setMuted (bool muted);
    void setSoloed (bool soloed);
    void setNumSlots (int numSlots);
    void setActiveSlot (int slotIndex);
    void setVolume (float normalizedVolume);
    void setRecording (bool recording);
    void setPlaying (bool playing);
    void setOverdubbing (bool overdubbing);
    void setInputLevel (float left, float right);
    void setOutputLevel (float left, float right);

    WaveformDisplay& getWaveformDisplay() { return waveformDisplay; }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::Colour trackColour;

    // Header controls
    juce::Label nameLabel;
    juce::TextButton armButton { "R" };
    juce::TextButton muteButton { "M" };
    juce::TextButton soloButton { "S" };

    // Waveform
    WaveformDisplay waveformDisplay;

    // Slot selector buttons
    juce::OwnedArray<juce::TextButton> slotButtons;
    int activeSlotIndex = -1;

    // Mixer
    juce::Slider volumeSlider;

    // Level meters
    LevelMeter inputMeter { LevelMeter::Orientation::vertical };
    LevelMeter outputMeter { LevelMeter::Orientation::vertical };

    // State indicator
    juce::Colour stateColour = AmlpLookAndFeel::getIdleColour();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackPanel)
};
