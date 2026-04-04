#include "AILoopPanel.h"

AILoopPanel::AILoopPanel()
{
    setOpaque(true);
    
    titleLabel.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);
    
    closeButton.onClick = [this] { if (onClose) onClose(); };
    addAndMakeVisible (closeButton);

    promptEditor.setMultiLine (true);
    promptEditor.setReturnKeyStartsNewLine (true);
    promptEditor.setText ("Synth Pad, Warm, Bright, chord progression, Medium Reverb");
    addAndMakeVisible (promptLabel);
    addAndMakeVisible (promptEditor);

    addAndMakeVisible (bpmLabel);
    addAndMakeVisible (bpmValueLabel);

    addAndMakeVisible (barsLabel);
    addAndMakeVisible (barsValueLabel);

    addAndMakeVisible (keyLabel);
    addAndMakeVisible (keyValueLabel);

    stepsSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    stepsSlider.setRange (20, 250, 1);
    stepsSlider.setValue (100);
    addAndMakeVisible (stepsLabel);
    addAndMakeVisible (stepsSlider);

    cfgSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    cfgSlider.setRange (1.0, 15.0, 0.1);
    cfgSlider.setValue (6.0);
    addAndMakeVisible (cfgLabel);
    addAndMakeVisible (cfgSlider);

    seedEditor.setText ("-1");
    addAndMakeVisible (seedLabel);
    addAndMakeVisible (seedEditor);


    generateButton.onClick = [this] {
        if (onGenerate)
        {
            AILoopParams p;
            p.prompt = promptEditor.getText();
            p.bpm = bpmValueLabel.getText().getDoubleValue();
            p.bars = barsValueLabel.getText().getIntValue();
            p.key = keyValueLabel.getText();
            p.steps = (int)stepsSlider.getValue();
            p.cfgScale = (float)cfgSlider.getValue();
            p.seed = seedEditor.getText().getIntValue();
            p.secondsTotal = (p.bars * 4.0 * 60.0) / p.bpm;

            if (styleTransferMode)
            {
                p.sourceAudioPath   = currentSourceAudioPath;
                p.variationStrength = (float) variationSlider.getValue();
            }

            onGenerate (p);
        }
    };
    addAndMakeVisible (generateButton);

    variationSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    variationSlider.setRange (0.0, 1.0, 0.01);
    variationSlider.setValue (0.7);
    addChildComponent (variationLabel);
    addChildComponent (variationSlider);

    statusLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (statusLabel);

    addAndMakeVisible (progressBar);
}

AILoopPanel::~AILoopPanel() = default;

void AILoopPanel::prepareForGeneration (double masterBpm, int detectedBars, const juce::String& masterKey)
{
    styleTransferMode = false;
    currentSourceAudioPath = {};

    titleLabel.setText ("AI Loop Generator", juce::dontSendNotification);
    generateButton.setButtonText ("Generate");
    variationLabel.setVisible (false);
    variationSlider.setVisible (false);

    bpmValueLabel.setText (juce::String (masterBpm, 2), juce::dontSendNotification);
    barsValueLabel.setText (juce::String (detectedBars > 0 ? detectedBars : 4), juce::dontSendNotification);
    keyValueLabel.setText (masterKey, juce::dontSendNotification);

    resized();
}

void AILoopPanel::prepareForStyleTransfer (double masterBpm, int detectedBars, const juce::String& masterKey,
                                           const juce::String& sourceAudioPath)
{
    styleTransferMode = true;
    currentSourceAudioPath = sourceAudioPath;

    titleLabel.setText ("AI Vary", juce::dontSendNotification);
    generateButton.setButtonText ("Vary");
    variationLabel.setVisible (true);
    variationSlider.setVisible (true);

    bpmValueLabel.setText (juce::String (masterBpm, 2), juce::dontSendNotification);
    barsValueLabel.setText (juce::String (detectedBars > 0 ? detectedBars : 4), juce::dontSendNotification);
    keyValueLabel.setText (masterKey, juce::dontSendNotification);

    resized();
}



void AILoopPanel::setServerState (AILoopClient::ServerState state, float setupProgress)
{
    currentServerState = state;
    
    if (state == AILoopClient::ServerState::settingUp)
    {
        statusLabel.setText ("Downloading Model & Setting up Python... Please wait.", juce::dontSendNotification);
        progressValue = setupProgress;
    }
    else if (state == AILoopClient::ServerState::loading)
    {
        statusLabel.setText ("Loading Foundation 1 model to GPU...", juce::dontSendNotification);
        progressValue = -1.0; // indeterminate
    }
    else if (state == AILoopClient::ServerState::ready)
    {
        if (currentGenState != AILoopClient::GenState::running)
            statusLabel.setText ("Ready", juce::dontSendNotification);
        if (currentGenState == AILoopClient::GenState::idle)
            progressValue = 0.0;
    }
    else if (state == AILoopClient::ServerState::error)
    {
        statusLabel.setText ("Server Error. Check console logs.", juce::dontSendNotification);
        progressValue = 0.0;
    }

    updateUIState();
}

void AILoopPanel::setGenerationState (AILoopClient::GenState state, float progress, const juce::String& errorMsg)
{
    currentGenState = state;
    
    if (state == AILoopClient::GenState::running)
    {
        statusLabel.setText (styleTransferMode ? "Generating Variation..." : "Generating Loop...", juce::dontSendNotification);
        progressValue = progress;
    }
    else if (state == AILoopClient::GenState::done)
    {
        statusLabel.setText ("Generation Complete!", juce::dontSendNotification);
        progressValue = 1.0;
    }
    else if (state == AILoopClient::GenState::error)
    {
        statusLabel.setText ("Error: " + errorMsg, juce::dontSendNotification);
        progressValue = 0.0;
    }

    updateUIState();
}

void AILoopPanel::updateUIState()
{
    bool isGenerating = (currentGenState == AILoopClient::GenState::running);
    bool isSettingUp = (currentServerState == AILoopClient::ServerState::settingUp || currentServerState == AILoopClient::ServerState::loading);
    
    generateButton.setEnabled (!isGenerating && !isSettingUp && currentServerState == AILoopClient::ServerState::ready);
    
    promptEditor.setEnabled (!isGenerating);
    // bpmValueLabel is always read-only
    // barsValueLabel is always read-only
    // keyValueLabel is always read-only
    stepsSlider.setEnabled (!isGenerating);
    cfgSlider.setEnabled (!isGenerating);
    seedEditor.setEnabled (!isGenerating);
    variationSlider.setEnabled (!isGenerating);
}

void AILoopPanel::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
    g.setColour (AmlpLookAndFeel::getDimTextColour());
    g.drawRect (getLocalBounds(), 1);
}

void AILoopPanel::resized()
{
    auto b = getLocalBounds().reduced (10);
    
    auto header = b.removeFromTop (24);
    closeButton.setBounds (header.removeFromRight (24));
    titleLabel.setBounds (header);
    
    b.removeFromTop (10);
    
    promptLabel.setBounds (b.removeFromTop (20));
    promptEditor.setBounds (b.removeFromTop (80));
    
    b.removeFromTop (10); // spacing
    
    auto row1 = b.removeFromTop (24);
    bpmLabel.setBounds (row1.removeFromLeft (40));
    bpmValueLabel.setBounds (row1.removeFromLeft (40));
    row1.removeFromLeft (10);
    barsLabel.setBounds (row1.removeFromLeft (40));
    barsValueLabel.setBounds (row1.removeFromLeft (40));
    row1.removeFromLeft (10);
    keyLabel.setBounds (row1.removeFromLeft (35));
    keyValueLabel.setBounds (row1.removeFromLeft (120));
    
    b.removeFromTop (10);
    
    auto row2 = b.removeFromTop (24);
    stepsLabel.setBounds (row2.removeFromLeft (50));
    stepsSlider.setBounds (row2.removeFromLeft (120));
    row2.removeFromLeft (10);
    cfgLabel.setBounds (row2.removeFromLeft (40));
    cfgSlider.setBounds (row2.removeFromLeft (120));
    
    b.removeFromTop (10);
    
    auto row3 = b.removeFromTop (24);
    seedLabel.setBounds (row3.removeFromLeft (50));
    seedEditor.setBounds (row3.removeFromLeft (100));

    if (styleTransferMode)
    {
        b.removeFromTop (10);
        auto varRow = b.removeFromTop (24);
        variationLabel.setBounds (varRow.removeFromLeft (70));
        variationSlider.setBounds (varRow.removeFromLeft (150));
    }

    b.removeFromTop (10);

    statusLabel.setBounds (b.removeFromTop (24));
    progressBar.setBounds (b.removeFromTop (20));
    
    b.removeFromTop (10);
    generateButton.setBounds (b.removeFromTop (30).withSizeKeepingCentre(150, 30));
}
