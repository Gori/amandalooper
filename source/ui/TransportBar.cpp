#include "TransportBar.h"

TransportBar::TransportBar()
{
    playPauseButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    playPauseButton.onClick = [this] { if (onPlayPause) onPlayPause(); };
    addAndMakeVisible (playPauseButton);

    stopButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    stopButton.onClick = [this] { if (onStop) onStop(); };
    addAndMakeVisible (stopButton);

    recordButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    recordButton.onClick = [this] { if (onRecord) onRecord(); };
    addAndMakeVisible (recordButton);

    metronomeButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    metronomeButton.onClick = [this] { if (onMetronomeToggle) onMetronomeToggle(); };
    addAndMakeVisible (metronomeButton);

    settingsButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    settingsButton.onClick = [this] { if (onSettings) onSettings(); };
    addAndMakeVisible (settingsButton);

    // Large time display
    timeDisplay.setText ("--:--:--", juce::dontSendNotification);
    timeDisplay.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    timeDisplay.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    timeDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (timeDisplay);

    // BPM text field
    bpmField.setText ("FREE", juce::dontSendNotification);
    bpmField.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    bpmField.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    bpmField.setColour (juce::Label::backgroundColourId, AmlpLookAndFeel::getSurfaceColour());
    bpmField.setColour (juce::Label::outlineColourId, AmlpLookAndFeel::getSurfaceColour().brighter (0.3f));
    bpmField.setJustificationType (juce::Justification::centred);
    bpmField.setEditable (true);
    bpmField.onTextChange = [this]
    {
        auto text = bpmField.getText().trim();
        double bpm = text.getDoubleValue();
        if (bpm >= 40.0 && bpm <= 240.0 && onBpmChange)
            onBpmChange (bpm);
    };
    addAndMakeVisible (bpmField);

    // Bar count selector
    barCountSelector.addItem ("1 bar", 1);
    barCountSelector.addItem ("2 bars", 2);
    barCountSelector.addItem ("4 bars", 4);
    barCountSelector.addItem ("8 bars", 8);
    barCountSelector.addItem ("16 bars", 16);
    barCountSelector.setVisible (false);
    barCountSelector.onChange = [this]
    {
        if (onBarCountChange)
            onBarCountChange (barCountSelector.getSelectedId());
    };
    addAndMakeVisible (barCountSelector);
}

void TransportBar::setBpm (double bpm)
{
    bpmField.setText (juce::String ((int) bpm), juce::dontSendNotification);

    if (freeMode)
        setFreeMode (false);
}

void TransportBar::setFreeMode (bool isFree)
{
    freeMode = isFree;
    barCountSelector.setVisible (! isFree);
    bpmField.setEditable (! isFree);

    if (isFree)
    {
        bpmField.setText ("FREE", juce::dontSendNotification);
        timeDisplay.setText ("--:--:--", juce::dontSendNotification);
    }
}

void TransportBar::setPlaying (bool isPlaying)
{
    playPauseButton.setButtonText (isPlaying ? juce::String::fromUTF8 ("\xe2\x8f\xb8")
                                             : juce::String::fromUTF8 ("\xe2\x96\xb6"));
    playPauseButton.setColour (juce::TextButton::buttonColourId,
        isPlaying ? AmlpLookAndFeel::getPlayingColour().darker (0.3f)
                  : AmlpLookAndFeel::getSurfaceColour());
}

void TransportBar::setRecording (bool isRecording)
{
    recordButton.setColour (juce::TextButton::buttonColourId,
        isRecording ? AmlpLookAndFeel::getRecordingColour()
                    : AmlpLookAndFeel::getRecordingColour().darker (0.5f));
}

void TransportBar::setMetronomeEnabled (bool enabled)
{
    metronomeButton.setColour (juce::TextButton::buttonColourId,
        enabled ? AmlpLookAndFeel::getArmedColour()
                : AmlpLookAndFeel::getSurfaceColour());
}

void TransportBar::setBarCount (int bars)
{
    // Select the matching item, or closest
    if (barCountSelector.indexOfItemId (bars) >= 0)
        barCountSelector.setSelectedId (bars, juce::dontSendNotification);
}

void TransportBar::setCountingIn (bool countingIn)
{
    recordButton.setButtonText (countingIn ? juce::String::fromUTF8 ("\xe2\x8f\xb3")
                                           : juce::String::fromUTF8 ("\xe2\x8f\xba"));
}

void TransportBar::setBarPosition (int bar, int beat, int ticks)
{
    auto text = juce::String (bar).paddedLeft ('0', 2) + ":"
              + juce::String (beat).paddedLeft ('0', 2) + ":"
              + juce::String (ticks).paddedLeft ('0', 2);
    timeDisplay.setText (text, juce::dontSendNotification);
}

void TransportBar::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
}

void TransportBar::resized()
{
    auto bounds = getLocalBounds().reduced (6);
    int buttonW = 55;
    int spacing = 4;

    playPauseButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    stopButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    recordButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    metronomeButton.setBounds (bounds.removeFromLeft (buttonW));
    bounds.removeFromLeft (spacing);
    settingsButton.setBounds (bounds.removeFromLeft (40));

    bounds.removeFromLeft (spacing * 2);

    // Time display — large, centered
    timeDisplay.setBounds (bounds.removeFromLeft (160));

    bounds.removeFromLeft (spacing * 2);

    // BPM field
    bpmField.setBounds (bounds.removeFromLeft (60));
    bounds.removeFromLeft (spacing);

    // Bar count selector
    barCountSelector.setBounds (bounds.removeFromLeft (80));
}
