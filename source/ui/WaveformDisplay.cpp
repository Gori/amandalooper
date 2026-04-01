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
    {
        currentFile = audioFile;
        thumbnail.setSource (new juce::FileInputSource (audioFile));
    }
}

void WaveformDisplay::clearSource()
{
    currentFile = juce::File();
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

void WaveformDisplay::setCountIn (int barsRemaining)
{
    countInBarsRemaining = barsRemaining;
}

void WaveformDisplay::setBeatGrid (double bpm, int beatsPerBar, int numBars)
{
    gridBPM = bpm;
    gridBeatsPerBar = beatsPerBar;
    gridNumBars = numBars;
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (AmlpLookAndFeel::getBackgroundColour().darker (0.3f));
    g.fillRoundedRectangle (bounds, 4.0f);

    if (thumbnail.getTotalLength() > 0.0)
    {
        // Draw beat grid behind waveform
        if (gridBPM > 0.0 && gridNumBars > 0)
        {
            int totalBeats = gridNumBars * gridBeatsPerBar;
            float beatWidth = bounds.getWidth() / (float) totalBeats;

            for (int i = 0; i <= totalBeats; ++i)
            {
                float x = bounds.getX() + (float) i * beatWidth;
                bool isBarLine = (i % gridBeatsPerBar == 0);

                g.setColour (isBarLine ? juce::Colours::white.withAlpha (0.25f)
                                       : juce::Colours::white.withAlpha (0.08f));
                g.drawLine (x, bounds.getY(), x, bounds.getBottom(),
                            isBarLine ? 1.5f : 0.5f);
            }
        }

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
        g.setColour (AmlpLookAndFeel::getDimTextColour());
        g.setFont (juce::FontOptions (14.0f));
        g.drawText (recording ? "Recording..." : "Empty",
                    getLocalBounds(), juce::Justification::centred);
    }

    if (recording)
    {
        g.setColour (AmlpLookAndFeel::getRecordingColour());
        g.drawRoundedRectangle (bounds.reduced (1.0f), 4.0f, 2.0f);
    }

    // Count-in or threshold waiting overlay
    if (countInBarsRemaining != 0)
    {
        g.setColour (AmlpLookAndFeel::getRecordingColour().withAlpha (0.3f));
        g.fillRoundedRectangle (bounds, 4.0f);

        g.setColour (juce::Colours::white);

        if (countInBarsRemaining < 0)
        {
            // Waiting for audio threshold
            g.setFont (juce::FontOptions (18.0f));
            g.drawText ("Waiting for audio...",
                        getLocalBounds(), juce::Justification::centred);
        }
        else
        {
            // Beat countdown
            g.setFont (juce::FontOptions (48.0f, juce::Font::bold));
            g.drawText (juce::String (countInBarsRemaining),
                        getLocalBounds(), juce::Justification::centred);
        }

        g.setColour (AmlpLookAndFeel::getRecordingColour());
        g.drawRoundedRectangle (bounds.reduced (1.0f), 4.0f, 3.0f);
    }
}

void WaveformDisplay::timerCallback()
{
    repaint();
}
