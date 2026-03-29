#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AmlpLookAndFeel.h"

class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void setSource (const juce::File& audioFile);
    juce::File getCurrentFile() const { return currentFile; }
    void clearSource();
    void setPlaybackPosition (double normalizedPos);
    void setTrackColour (juce::Colour colour);
    void setRecording (bool isRecording);
    void setCountIn (int barsRemaining);

    void setBeatGrid (double bpm, int beatsPerBar, int numBars);

    void paint (juce::Graphics& g) override;
    void resized() override {}

private:
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };

    juce::File currentFile;
    double playbackPosition = 0.0;
    juce::Colour trackColour = AmlpLookAndFeel::getPlayingColour();
    bool recording = false;
    int countInBarsRemaining = 0;

    double gridBPM = 0.0;
    int gridBeatsPerBar = 4;
    int gridNumBars = 0;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
