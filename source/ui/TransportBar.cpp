#include "TransportBar.h"

TransportBar::TransportBar()
{
    // Record button - red
    recordButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getRecordingColour().darker (0.5f));
    recordButton.onClick = [this] { if (onRecord) onRecord(); };
    addAndMakeVisible (recordButton);

    // Play button - green
    playButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getPlayingColour().darker (0.5f));
    playButton.onClick = [this] { if (onPlay) onPlay(); };
    addAndMakeVisible (playButton);

    // Stop button
    stopButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    stopButton.onClick = [this] { if (onStop) onStop(); };
    addAndMakeVisible (stopButton);

    // Overdub button - yellow
    overdubButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getOverdubbingColour().darker (0.5f));
    overdubButton.onClick = [this] { if (onOverdub) onOverdub(); };
    addAndMakeVisible (overdubButton);

    // Metronome toggle
    metronomeButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    metronomeButton.onClick = [this] { if (onMetronomeToggle) onMetronomeToggle(); };
    addAndMakeVisible (metronomeButton);

    // Tap tempo
    tapTempoButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    tapTempoButton.onClick = [this] { if (onTapTempo) onTapTempo(); };
    addAndMakeVisible (tapTempoButton);

    // BPM display
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getDimTextColour());
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (bpmLabel);

    // BPM slider
    bpmSlider.setRange (40.0, 240.0, 1.0);
    bpmSlider.setValue (120.0, juce::dontSendNotification);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 30);
    bpmSlider.setColour (juce::Slider::textBoxTextColourId, AmlpLookAndFeel::getTextColour());
    bpmSlider.setColour (juce::Slider::textBoxBackgroundColourId, AmlpLookAndFeel::getSurfaceColour());
    bpmSlider.onValueChange = [this]
    {
        if (onBpmChange)
            onBpmChange (bpmSlider.getValue());
    };
    addAndMakeVisible (bpmSlider);
}

void TransportBar::setBpm (double bpm)
{
    bpmSlider.setValue (bpm, juce::dontSendNotification);
}

void TransportBar::setRecording (bool isRecording)
{
    recordButton.setColour (juce::TextButton::buttonColourId,
        isRecording ? AmlpLookAndFeel::getRecordingColour()
                    : AmlpLookAndFeel::getRecordingColour().darker (0.5f));
    recordButton.repaint();
}

void TransportBar::setPlaying (bool isPlaying)
{
    playButton.setColour (juce::TextButton::buttonColourId,
        isPlaying ? AmlpLookAndFeel::getPlayingColour()
                  : AmlpLookAndFeel::getPlayingColour().darker (0.5f));
    playButton.repaint();
}

void TransportBar::setOverdubbing (bool isOverdubbing)
{
    overdubButton.setColour (juce::TextButton::buttonColourId,
        isOverdubbing ? AmlpLookAndFeel::getOverdubbingColour()
                      : AmlpLookAndFeel::getOverdubbingColour().darker (0.5f));
    overdubButton.repaint();
}

void TransportBar::setMetronomeEnabled (bool enabled)
{
    metronomeButton.setColour (juce::TextButton::buttonColourId,
        enabled ? AmlpLookAndFeel::getArmedColour()
                : AmlpLookAndFeel::getSurfaceColour());
    metronomeButton.repaint();
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds().reduced (6);
    int buttonW = 80;
    int spacing = 6;

    recordButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    playButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    stopButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    overdubButton.setBounds (bounds.removeFromLeft (buttonW));

    bounds.removeFromLeft (spacing * 3);

    metronomeButton.setBounds (bounds.removeFromLeft (60));
    bounds.removeFromLeft (spacing);
    tapTempoButton.setBounds (bounds.removeFromLeft (50));

    bounds.removeFromLeft (spacing * 3);

    bpmLabel.setBounds (bounds.removeFromLeft (35));
    bpmSlider.setBounds (bounds.removeFromLeft (180));
}
