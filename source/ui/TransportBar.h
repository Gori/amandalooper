#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"

/**
    Top transport bar with BPM, record/play/stop/overdub buttons,
    metronome toggle, and master volume.
*/
class TransportBar : public juce::Component
{
public:
    TransportBar();

    // Callbacks — set these from MainComponent
    std::function<void()> onRecord;
    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void()> onOverdub;
    std::function<void()> onMetronomeToggle;
    std::function<void()> onTapTempo;
    std::function<void (double)> onBpmChange;

    void setBpm (double bpm);
    void setRecording (bool isRecording);
    void setPlaying (bool isPlaying);
    void setOverdubbing (bool isOverdubbing);
    void setMetronomeEnabled (bool enabled);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::TextButton recordButton { "REC" };
    juce::TextButton playButton { "PLAY" };
    juce::TextButton stopButton { "STOP" };
    juce::TextButton overdubButton { "OVERDUB" };
    juce::TextButton metronomeButton { "METRO" };
    juce::TextButton tapTempoButton { "TAP" };
    juce::Label bpmLabel;
    juce::Slider bpmSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportBar)
};
