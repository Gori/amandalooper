#include "MainComponent.h"

namespace te = tracktion::engine;

MainComponent::MainComponent()
{
    setLookAndFeel (&lookAndFeel);
    setupEngine();
    setupUI();
    connectCallbacks();
    startTimerHz (30);
    setSize (1200, 800);
}

MainComponent::~MainComponent()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MainComponent::setupEngine()
{
    auto& edit = amlpEngine.getEdit();

    loopManager = std::make_unique<LoopManager> (edit);
    loopManager->ensureScenes (8);

    sceneManager = std::make_unique<SceneManager> (edit);
    metronome = std::make_unique<Metronome> (edit);

    loopManager->setMasterBPM (120.0);

    for (int i = 0; i < sceneManager->getNumScenes(); ++i)
        sceneManager->setSceneName (i, "Scene " + juce::String (i + 1));
}

void MainComponent::setupUI()
{
    addAndMakeVisible (transportBar);

    for (int i = 0; i < loopManager->getTrackCount(); ++i)
    {
        auto* panel = new TrackPanel (i);
        panel->setNumSlots (sceneManager->getNumScenes());
        addAndMakeVisible (panel);
        trackPanels.add (panel);
    }

    sceneBar.setNumScenes (sceneManager->getNumScenes());
    for (int i = 0; i < sceneManager->getNumScenes(); ++i)
        sceneBar.setSceneName (i, sceneManager->getSceneName (i));
    addAndMakeVisible (sceneBar);

    audioSettingsButton.onClick = [this] { amlpEngine.showAudioDeviceSettings (*this); };
    addAndMakeVisible (audioSettingsButton);

    deviceStatusLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getDimTextColour());
    deviceStatusLabel.setFont (juce::FontOptions (11.0f));
    deviceStatusLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (deviceStatusLabel);

    addAndMakeVisible (masterMeter);

    transportBar.setBpm (loopManager->getMasterBPM());

    // Hold a reference to the input level meter so JUCE measures input levels
    inputLevelMeter = amlpEngine.getEngine().getDeviceManager().deviceManager.getInputLevelGetter();
}

void MainComponent::connectCallbacks()
{
    transportBar.onRecord = [this]
    {
        if (auto* track = loopManager->getTrack (selectedTrack))
        {
            if (track->getState() == LoopTrack::State::recording)
                track->stopRecording();
            else
                track->startRecording (0);
        }
    };

    transportBar.onPlay = [this]
    {
        if (auto* track = loopManager->getTrack (selectedTrack))
        {
            if (track->getState() == LoopTrack::State::idle)
                track->triggerSlot (0);
        }
    };

    transportBar.onStop = [this]
    {
        loopManager->stopAllTracks();
    };

    transportBar.onOverdub = [this]
    {
        if (auto* track = loopManager->getTrack (selectedTrack))
        {
            if (track->getState() == LoopTrack::State::overdubbing)
                track->stopOverdub();
            else if (track->getState() == LoopTrack::State::playing)
                track->startOverdub();
        }
    };

    transportBar.onMetronomeToggle = [this]
    {
        bool newState = ! metronome->isEnabled();
        metronome->setEnabled (newState);
        metronome->setRecordingOnly (false);
        transportBar.setMetronomeEnabled (newState);

        auto& transport = amlpEngine.getEdit().getTransport();
        if (newState && ! transport.isPlaying())
            transport.play (false);
    };

    transportBar.onTapTempo = [this]
    {
        tapTempo.tap();
        double bpm = tapTempo.getBPM();
        if (bpm > 0.0)
        {
            loopManager->setMasterBPM (bpm);
            transportBar.setBpm (bpm);
        }
    };

    transportBar.onBpmChange = [this] (double bpm)
    {
        loopManager->setMasterBPM (bpm);
    };

    for (int i = 0; i < trackPanels.size(); ++i)
    {
        auto* panel = trackPanels[i];

        panel->onArm = [this, i] { selectedTrack = i; };

        panel->onMute = [this, i]
        {
            if (auto* track = loopManager->getTrack (i))
            {
                track->setMuted (! track->isMuted());
                trackPanels[i]->setMuted (track->isMuted());
            }
        };

        panel->onSolo = [this, i]
        {
            if (auto* track = loopManager->getTrack (i))
            {
                track->setSolo (! track->isSolo());
                trackPanels[i]->setSoloed (track->isSolo());
            }
        };

        panel->onSlotSelected = [this, i] (int slotIndex)
        {
            if (auto* track = loopManager->getTrack (i))
                track->triggerSlot (slotIndex);
        };
    }

    sceneBar.onLaunchScene = [this] (int sceneIndex)
    {
        sceneManager->launchScene (sceneIndex);
        sceneBar.setActiveScene (sceneIndex);
    };

    sceneBar.onStopAll = [this]
    {
        sceneManager->stopAllScenes();
        loopManager->stopAllTracks();
        sceneBar.setActiveScene (-1);
    };
}

void MainComponent::updateUI()
{
    for (int i = 0; i < trackPanels.size() && i < loopManager->getTrackCount(); ++i)
    {
        auto* loopTrack = loopManager->getTrack (i);
        auto* panel = trackPanels[i];
        if (loopTrack == nullptr || panel == nullptr)
            continue;

        auto state = loopTrack->getState();
        panel->setRecording (state == LoopTrack::State::recording);
        panel->setPlaying (state == LoopTrack::State::playing);
        panel->setOverdubbing (state == LoopTrack::State::overdubbing);
        panel->setActiveSlot (loopTrack->getActiveSlotIndex());
        panel->setArmed (i == selectedTrack);

        auto& teTrack = loopTrack->getTrack();

        // Update waveform display if a clip exists in the active slot
        if (auto* clip = loopTrack->getClipInSlot (loopTrack->getActiveSlotIndex()))
        {
            auto file = clip->getOriginalFile();
            auto& waveform = panel->getWaveformDisplay();

            if (waveform.getCurrentFile() != file)
                waveform.setSource (file);

            // Update playback position
            if (auto handle = clip->getLaunchHandle())
            {
                if (auto playedRange = handle->getPlayedRange())
                {
                    auto& ts = amlpEngine.getEdit().tempoSequence;
                    auto clipLength = clip->getPosition().getLength().inSeconds();

                    if (clipLength > 0.0)
                    {
                        auto posTime = ts.toTime (playedRange->getEnd()).inSeconds();
                        waveform.setPlaybackPosition (std::fmod (posTime, clipLength) / clipLength);
                    }
                }
            }
        }

        // Output level from TE's LevelMeterPlugin
        if (auto* levelPlugin = teTrack.getLevelMeterPlugin())
        {
            auto cache = levelPlugin->measurer.getLevelCache();
            panel->setOutputLevel (te::dbToGain (cache.first), te::dbToGain (cache.second));
        }

        // Input level — only show on armed tracks
        if (i == selectedTrack && inputLevelMeter != nullptr)
        {
            auto level = (float) inputLevelMeter->getCurrentLevel();
            panel->setInputLevel (level, level);
        }
        else
        {
            panel->setInputLevel (0.0f, 0.0f);
        }
    }

    if (auto* loopTrack = loopManager->getTrack (selectedTrack))
    {
        transportBar.setRecording (loopTrack->getState() == LoopTrack::State::recording);
        transportBar.setPlaying (loopTrack->getState() == LoopTrack::State::playing);
        transportBar.setOverdubbing (loopTrack->getState() == LoopTrack::State::overdubbing);
    }

    // Master output meter
    float masterL = 0.0f, masterR = 0.0f;
    for (int i = 0; i < loopManager->getTrackCount(); ++i)
    {
        if (auto* lt = loopManager->getTrack (i))
        {
            if (auto* lmp = lt->getTrack().getLevelMeterPlugin())
            {
                auto mc = lmp->measurer.getLevelCache();
                masterL = std::max (masterL, te::dbToGain (mc.first));
                masterR = std::max (masterR, te::dbToGain (mc.second));
            }
        }
    }
    masterMeter.setLevel (masterL, masterR);

    // Device status
    if (auto* device = amlpEngine.getEngine().getDeviceManager().deviceManager.getCurrentAudioDevice())
    {
        auto sr = device->getCurrentSampleRate();
        auto bs = device->getCurrentBufferSizeSamples();
        deviceStatusLabel.setText (device->getName()
            + " | " + juce::String (sr / 1000.0, 1) + "kHz"
            + " | " + juce::String (bs) + " samples"
            + " | " + juce::String ((bs / sr) * 1000.0, 1) + "ms",
            juce::dontSendNotification);
    }
    else
    {
        deviceStatusLabel.setText ("No audio device - click Audio Settings", juce::dontSendNotification);
    }
}

void MainComponent::timerCallback()
{
    updateUI();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getBackgroundColour());
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    deviceStatusLabel.setBounds (bounds.removeFromTop (18).reduced (8, 0));

    auto topBar = bounds.removeFromTop (50);
    transportBar.setBounds (topBar.removeFromLeft (topBar.getWidth() - 120));
    audioSettingsButton.setBounds (topBar.reduced (4));

    sceneBar.setBounds (bounds.removeFromBottom (44));

    masterMeter.setBounds (bounds.removeFromRight (24).reduced (4));

    bounds.reduce (4, 4);
    int trackHeight = trackPanels.isEmpty() ? 0 : bounds.getHeight() / trackPanels.size();

    for (auto* panel : trackPanels)
        panel->setBounds (bounds.removeFromTop (trackHeight).reduced (0, 2));
}
