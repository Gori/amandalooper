#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"

/**
    A simple peak + RMS level meter with configurable orientation.
*/
class LevelMeter : public juce::Component,
                   private juce::Timer
{
public:
    enum class Orientation { vertical, horizontal };

    explicit LevelMeter (Orientation orient = Orientation::vertical);

    /** Set the current level (0.0 - 1.0 linear). Call from message thread. */
    void setLevel (float leftLevel, float rightLevel);

    /** Set whether this is an input meter (blue) or output meter (green). */
    void setInputMeter (bool isInput);

    void paint (juce::Graphics& g) override;

private:
    Orientation orientation;
    float leftPeak = 0.0f, rightPeak = 0.0f;
    float leftDisplay = 0.0f, rightDisplay = 0.0f;
    bool inputMeter = false;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
