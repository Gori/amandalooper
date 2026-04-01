#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"

class TransportBar : public juce::Component
{
public:
    TransportBar();

    // Callbacks
    std::function<void()> onPlayPause;
    std::function<void()> onStop;
    std::function<void()> onRecord;
    std::function<void()> onMetronomeToggle;
    std::function<void()> onSettings;
    std::function<void()> onMasterFx;
    std::function<void (double)> onBpmChange;
    std::function<void (int)> onBarCountChange;

    void setBpm (double bpm);
    void setFreeMode (bool isFree);
    void setPlaying (bool isPlaying);
    void setRecording (bool isRecording);
    void setMetronomeEnabled (bool enabled);
    void setCountingIn (bool countingIn);
    void setBarPosition (int bar, int beat, int ticks);
    void setBarCount (int bars);

    void paint (juce::Graphics& g) override;
    void resized() override;

    bool freeMode = true;

private:
    juce::TextButton playPauseButton { juce::String::fromUTF8 ("\xe2\x96\xb6") };
    juce::TextButton stopButton { juce::String::fromUTF8 ("\xe2\x8f\xb9") };
    juce::TextButton recordButton { juce::String::fromUTF8 ("\xe2\x8f\xba") };
    juce::TextButton metronomeButton { juce::String::fromUTF8 ("\xf0\x9f\xa5\x81") };
    juce::TextButton settingsButton { juce::String::fromUTF8 ("\xe2\x9a\x99") };
    juce::TextButton masterFxButton { "MFX" };

    juce::Label timeDisplay;
    juce::Label bpmField;
    juce::ComboBox barCountSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};
