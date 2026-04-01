#include "TrackPanel.h"

TrackPanel::TrackPanel (int trackIndex)
    : trackColour (AmlpLookAndFeel::getTrackColour (trackIndex))
{
    nameLabel.setText ("Track " + juce::String (trackIndex + 1), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, trackColour);
    nameLabel.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    nameLabel.setEditable (true);
    addAndMakeVisible (nameLabel);

    // Mode buttons
    auto setupModeBtn = [this] (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
        addAndMakeVisible (btn);
    };

    setupModeBtn (replaceButton);
    setupModeBtn (overdubButton);
    setupModeBtn (newLoopButton);

    replaceButton.onClick = [this] { setMode (RecordMode::replace); };
    overdubButton.onClick = [this] { setMode (RecordMode::overdub); };
    newLoopButton.onClick = [this] { setMode (RecordMode::newLoop); };

    updateModeButtons();

    // Arm/Mute/Solo
    armButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    armButton.onClick = [this] { if (onArm) onArm(); };
    addAndMakeVisible (armButton);

    muteButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    muteButton.onClick = [this] { if (onMute) onMute(); };
    addAndMakeVisible (muteButton);

    soloButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    soloButton.onClick = [this] { if (onSolo) onSolo(); };
    addAndMakeVisible (soloButton);

    fxButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    fxButton.onClick = [this] { if (onFxClicked) onFxClicked(); };
    addAndMakeVisible (fxButton);

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

    // Loop dropdown
    loopDropdown.setColour (juce::ComboBox::backgroundColourId, AmlpLookAndFeel::getSurfaceColour());
    loopDropdown.setColour (juce::ComboBox::textColourId, AmlpLookAndFeel::getTextColour());
    loopDropdown.onChange = [this]
    {
        if (onLoopSelected)
            onLoopSelected (loopDropdown.getSelectedId() - 1); // IDs are 1-based
    };
    addAndMakeVisible (loopDropdown);

    // Level meters
    inputMeter.setInputMeter (true);
    addAndMakeVisible (inputMeter);
    addAndMakeVisible (outputMeter);
}

void TrackPanel::setTrackName (const juce::String& name)
{
    nameLabel.setText (name, juce::dontSendNotification);
}

juce::String TrackPanel::getTrackName() const
{
    return nameLabel.getText();
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

void TrackPanel::setMode (RecordMode mode)
{
    currentMode = mode;
    updateModeButtons();

    if (onModeChanged)
        onModeChanged (mode);
}

void TrackPanel::setLoopList (const juce::StringArray& names, int activeIndex)
{
    loopDropdown.clear (juce::dontSendNotification);

    for (int i = 0; i < names.size(); ++i)
        loopDropdown.addItem (names[i], i + 1); // IDs are 1-based

    if (activeIndex >= 0 && activeIndex < names.size())
        loopDropdown.setSelectedId (activeIndex + 1, juce::dontSendNotification);
}

void TrackPanel::updateModeButtons()
{
    auto active = AmlpLookAndFeel::getArmedColour();
    auto inactive = AmlpLookAndFeel::getSurfaceColour();

    replaceButton.setColour (juce::TextButton::buttonColourId,
                              currentMode == RecordMode::replace ? active : inactive);
    overdubButton.setColour (juce::TextButton::buttonColourId,
                              currentMode == RecordMode::overdub ? active : inactive);
    newLoopButton.setColour (juce::TextButton::buttonColourId,
                              currentMode == RecordMode::newLoop ? active : inactive);
}

void TrackPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour (AmlpLookAndFeel::getPanelColour());
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (stateColour);
    g.fillRoundedRectangle (bounds.removeFromLeft (4.0f), 2.0f);
}

void TrackPanel::resized()
{
    auto bounds = getLocalBounds().reduced (6);
    bounds.removeFromLeft (4); // state indicator

    // Input meter
    inputMeter.setBounds (bounds.removeFromLeft (14));
    bounds.removeFromLeft (4);

    // Header column
    auto header = bounds.removeFromLeft (150);
    nameLabel.setBounds (header.removeFromTop (20));

    // Mode buttons row
    auto modeRow = header.removeFromTop (22);
    int modeBtnW = modeRow.getWidth() / 3;
    replaceButton.setBounds (modeRow.removeFromLeft (modeBtnW).reduced (1));
    overdubButton.setBounds (modeRow.removeFromLeft (modeBtnW).reduced (1));
    newLoopButton.setBounds (modeRow.reduced (1));

    // R/M/S row
    auto btnRow = header.removeFromTop (24);
    armButton.setBounds (btnRow.removeFromLeft (28).reduced (1));
    muteButton.setBounds (btnRow.removeFromLeft (28).reduced (1));
    soloButton.setBounds (btnRow.removeFromLeft (28).reduced (1));
    fxButton.setBounds (btnRow.removeFromLeft (28).reduced (1));

    // Volume
    volumeSlider.setBounds (header.removeFromTop (22));

    // Loop dropdown below volume
    header.removeFromTop (2);
    loopDropdown.setBounds (header.removeFromTop (22));

    bounds.removeFromLeft (6);

    // Output meter
    outputMeter.setBounds (bounds.removeFromRight (14));
    bounds.removeFromRight (4);

    // Waveform fills the rest
    waveformDisplay.setBounds (bounds);
}
