#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
    Dark theme look and feel for AMLP.
    Hardware looper inspired — dark grays, colored track indicators,
    state-based button colors.
*/
class AmlpLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AmlpLookAndFeel();

    // Track colors (one per track)
    static juce::Colour getTrackColour (int trackIndex);

    // State colors
    static juce::Colour getRecordingColour()    { return juce::Colour (0xffff3333); }
    static juce::Colour getPlayingColour()      { return juce::Colour (0xff33cc33); }
    static juce::Colour getOverdubbingColour()  { return juce::Colour (0xffffcc00); }
    static juce::Colour getIdleColour()         { return juce::Colour (0xff555555); }
    static juce::Colour getArmedColour()        { return juce::Colour (0xff3388ff); }

    // Background colors
    static juce::Colour getBackgroundColour()   { return juce::Colour (0xff1a1a2e); }
    static juce::Colour getPanelColour()        { return juce::Colour (0xff16213e); }
    static juce::Colour getSurfaceColour()      { return juce::Colour (0xff0f3460); }
    static juce::Colour getTextColour()         { return juce::Colour (0xffe0e0e0); }
    static juce::Colour getDimTextColour()      { return juce::Colour (0xff888888); }

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};
