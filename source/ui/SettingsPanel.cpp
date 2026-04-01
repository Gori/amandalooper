#include "SettingsPanel.h"

SettingsPanel::SettingsPanel()
{
    titleLabel.setText ("Settings", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    addAndMakeVisible (titleLabel);

    autoMetronomeToggle.setToggleState (true, juce::dontSendNotification);
    autoMetronomeToggle.setColour (juce::ToggleButton::textColourId, AmlpLookAndFeel::getTextColour());
    autoMetronomeToggle.setColour (juce::ToggleButton::tickColourId, AmlpLookAndFeel::getPlayingColour());
    addAndMakeVisible (autoMetronomeToggle);

    thresholdRecordingToggle.setToggleState (true, juce::dontSendNotification);
    thresholdRecordingToggle.setColour (juce::ToggleButton::textColourId, AmlpLookAndFeel::getTextColour());
    thresholdRecordingToggle.setColour (juce::ToggleButton::tickColourId, AmlpLookAndFeel::getPlayingColour());
    addAndMakeVisible (thresholdRecordingToggle);

    defaultBarCountLabel.setText ("Default loop length:", juce::dontSendNotification);
    defaultBarCountLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    addAndMakeVisible (defaultBarCountLabel);

    defaultBarCountSelector.addItem ("Match first loop", 1);
    defaultBarCountSelector.addItem ("1 bar", 2);
    defaultBarCountSelector.addItem ("2 bars", 3);
    defaultBarCountSelector.addItem ("4 bars", 4);
    defaultBarCountSelector.addItem ("8 bars", 5);
    defaultBarCountSelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (defaultBarCountSelector);

    countInBarsLabel.setText ("Count-in:", juce::dontSendNotification);
    countInBarsLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    addAndMakeVisible (countInBarsLabel);

    countInBarsSelector.addItem ("1 bar", 1);
    countInBarsSelector.addItem ("2 bars", 2);
    countInBarsSelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (countInBarsSelector);

    setSize (300, 240);
}

bool SettingsPanel::getAutoMetronome() const
{
    return autoMetronomeToggle.getToggleState();
}

bool SettingsPanel::getThresholdRecording() const
{
    return thresholdRecordingToggle.getToggleState();
}

int SettingsPanel::getDefaultBarCount() const
{
    int id = defaultBarCountSelector.getSelectedId();
    if (id == 2) return 1;
    if (id == 3) return 2;
    if (id == 4) return 4;
    if (id == 5) return 8;
    return 0;
}

int SettingsPanel::getCountInBars() const
{
    return countInBarsSelector.getSelectedId();
}

void SettingsPanel::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
    g.setColour (AmlpLookAndFeel::getSurfaceColour());
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1), 6.0f, 2.0f);
}

void SettingsPanel::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    titleLabel.setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (8);
    autoMetronomeToggle.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (4);
    thresholdRecordingToggle.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (8);

    auto row = bounds.removeFromTop (28);
    defaultBarCountLabel.setBounds (row.removeFromLeft (140));
    defaultBarCountSelector.setBounds (row);

    bounds.removeFromTop (8);

    row = bounds.removeFromTop (28);
    countInBarsLabel.setBounds (row.removeFromLeft (140));
    countInBarsSelector.setBounds (row);
}
