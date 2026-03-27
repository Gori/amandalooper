#include "TrackPanel.h"

TrackPanel::TrackPanel (int trackIndex)
    : trackColour (AmlpLookAndFeel::getTrackColour (trackIndex))
{
    // Track name
    nameLabel.setText ("Track " + juce::String (trackIndex + 1), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, trackColour);
    nameLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    nameLabel.setEditable (true);
    addAndMakeVisible (nameLabel);

    // Arm button
    armButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    armButton.onClick = [this] { if (onArm) onArm(); };
    addAndMakeVisible (armButton);

    // Mute button
    muteButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    muteButton.onClick = [this] { if (onMute) onMute(); };
    addAndMakeVisible (muteButton);

    // Solo button
    soloButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    soloButton.onClick = [this] { if (onSolo) onSolo(); };
    addAndMakeVisible (soloButton);

    // Waveform
    waveformDisplay.setTrackColour (trackColour);
    addAndMakeVisible (waveformDisplay);

    // Volume slider
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setRange (0.0, 1.0);
    volumeSlider.setValue (0.8);
    volumeSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.onValueChange = [this]
    {
        if (onVolumeChange)
            onVolumeChange (static_cast<float> (volumeSlider.getValue()));
    };
    addAndMakeVisible (volumeSlider);

    // Level meters
    inputMeter.setInputMeter (true);
    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);
}

void TrackPanel::setTrackName (const juce::String& name)
{
    nameLabel.setText (name, juce::dontSendNotification);
}

void TrackPanel::setArmed (bool armed)
{
    armButton.setColour (juce::TextButton::buttonColourId,
        armed ? AmlpLookAndFeel::getRecordingColour() : AmlpLookAndFeel::getSurfaceColour());
}

void TrackPanel::setMuted (bool muted)
{
    muteButton.setColour (juce::TextButton::buttonColourId,
        muted ? AmlpLookAndFeel::getOverdubbingColour() : AmlpLookAndFeel::getSurfaceColour());
}

void TrackPanel::setSoloed (bool soloed)
{
    soloButton.setColour (juce::TextButton::buttonColourId,
        soloed ? AmlpLookAndFeel::getArmedColour() : AmlpLookAndFeel::getSurfaceColour());
}

void TrackPanel::setNumSlots (int numSlots)
{
    slotButtons.clear();
    for (int i = 0; i < numSlots; ++i)
    {
        auto* btn = new juce::TextButton (juce::String (i + 1));
        btn->setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
        btn->onClick = [this, i] { if (onSlotSelected) onSlotSelected (i); };
        addAndMakeVisible (btn);
        slotButtons.add (btn);
    }
    resized();
}

void TrackPanel::setActiveSlot (int slotIndex)
{
    activeSlotIndex = slotIndex;
    for (int i = 0; i < slotButtons.size(); ++i)
    {
        slotButtons[i]->setColour (juce::TextButton::buttonColourId,
            i == slotIndex ? trackColour.darker (0.3f) : AmlpLookAndFeel::getSurfaceColour());
    }
}

void TrackPanel::setVolume (float normalizedVolume)
{
    volumeSlider.setValue (normalizedVolume, juce::dontSendNotification);
}

void TrackPanel::setRecording (bool recording)
{
    stateColour = recording ? AmlpLookAndFeel::getRecordingColour() : AmlpLookAndFeel::getIdleColour();
    waveformDisplay.setRecording (recording);
    repaint();
}

void TrackPanel::setPlaying (bool playing)
{
    if (playing)
        stateColour = AmlpLookAndFeel::getPlayingColour();
    repaint();
}

void TrackPanel::setOverdubbing (bool overdubbing)
{
    if (overdubbing)
        stateColour = AmlpLookAndFeel::getOverdubbingColour();
    repaint();
}

void TrackPanel::setInputLevel (float left, float right)
{
    inputMeter.setLevel (left, right);
}

void TrackPanel::setOutputLevel (float left, float right)
{
    outputMeter.setLevel (left, right);
}

void TrackPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (AmlpLookAndFeel::getPanelColour());
    g.fillRoundedRectangle (bounds, 4.0f);

    // State indicator bar on the left
    g.setColour (stateColour);
    g.fillRoundedRectangle (bounds.removeFromLeft (4.0f), 2.0f);
}

void TrackPanel::resized()
{
    auto bounds = getLocalBounds().reduced (6);
    bounds.removeFromLeft (4); // space for state indicator

    // Input meter on the left
    inputMeter.setBounds (bounds.removeFromLeft (14));
    bounds.removeFromLeft (4);

    // Header row: name + arm/mute/solo
    auto header = bounds.removeFromLeft (140);
    nameLabel.setBounds (header.removeFromTop (24));

    auto btnRow = header.removeFromTop (28);
    armButton.setBounds (btnRow.removeFromLeft (30).reduced (1));
    muteButton.setBounds (btnRow.removeFromLeft (30).reduced (1));
    soloButton.setBounds (btnRow.removeFromLeft (30).reduced (1));

    // Volume at the bottom of header
    volumeSlider.setBounds (header.removeFromTop (24));

    bounds.removeFromLeft (6);

    // Output meter next to waveform
    outputMeter.setBounds (bounds.removeFromRight (14));
    bounds.removeFromRight (4);

    // Slot selector on the right
    auto slotArea = bounds.removeFromRight (std::min (bounds.getWidth() / 4, 160));
    int slotBtnH = slotButtons.isEmpty() ? 0 : std::min (30, slotArea.getHeight() / slotButtons.size());
    for (auto* btn : slotButtons)
    {
        btn->setBounds (slotArea.removeFromTop (slotBtnH).reduced (1));
    }

    // Waveform fills remaining space
    waveformDisplay.setBounds (bounds);
}
