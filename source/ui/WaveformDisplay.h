#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AmlpLookAndFeel.h"

/**
    Displays an audio waveform with playback cursor and loop region.
    Uses juce::AudioThumbnail for efficient rendering.
*/
class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    /** Set the audio file to display. */
    void setSource (const juce::File& audioFile);

    /** Clear the display. */
    void clearSource();

    /** Set normalized playback position (0.0 - 1.0). */
    void setPlaybackPosition (double normalizedPos);

    /** Set the track colour for the waveform. */
    void setTrackColour (juce::Colour colour);

    /** Set whether this track is recording. */
    void setRecording (bool isRecording);

    void paint (juce::Graphics& g) override;
    void resized() override {}

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };

    double playbackPosition = 0.0;
    juce::Colour trackColour = AmlpLookAndFeel::getPlayingColour();
    bool recording = false;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
