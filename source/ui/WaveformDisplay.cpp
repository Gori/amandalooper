#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
    formatManager.registerBasicFormats();
    startTimerHz (30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::setSource (const juce::File& audioFile)
{
    if (audioFile.existsAsFile())
        thumbnail.setSource (new juce::FileInputSource (audioFile));
}

void WaveformDisplay::clearSource()
{
    thumbnail.clear();
    playbackPosition = 0.0;
    repaint();
}

void WaveformDisplay::setPlaybackPosition (double normalizedPos)
{
    playbackPosition = normalizedPos;
}

void WaveformDisplay::setTrackColour (juce::Colour colour)
{
    trackColour = colour;
    repaint();
}

void WaveformDisplay::setRecording (bool isRecording)
{
    recording = isRecording;
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (AmlpLookAndFeel::getBackgroundColour().darker (0.3f));
    g.fillRoundedRectangle (bounds, 4.0f);

    if (thumbnail.getTotalLength() > 0.0)
    {
        // Draw waveform
        g.setColour (trackColour.withAlpha (0.7f));
        thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                0.0, thumbnail.getTotalLength(), 1.0f);

        // Draw playback cursor
        if (playbackPosition > 0.0)
        {
            auto cursorX = bounds.getX() + static_cast<float> (playbackPosition) * bounds.getWidth();
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawLine (cursorX, bounds.getY(), cursorX, bounds.getBottom(), 2.0f);
        }
    }
    else
    {
        // Empty state
        g.setColour (AmlpLookAndFeel::getDimTextColour());
        g.setFont (juce::FontOptions (14.0f));
        g.drawText (recording ? "Recording..." : "Empty",
                    getLocalBounds(), juce::Justification::centred);
    }

    // Recording indicator border
    if (recording)
    {
        g.setColour (AmlpLookAndFeel::getRecordingColour());
        g.drawRoundedRectangle (bounds.reduced (1.0f), 4.0f, 2.0f);
    }
}

void WaveformDisplay::timerCallback()
{
    repaint();
}
