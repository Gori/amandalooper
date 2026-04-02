#include "MainComponent.h"
#include "core/AudioFileImporter.h"
#include "core/KeyDetector.h"

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
    effectsManager = std::make_unique<EffectsChainManager> (edit);
    aiLoopClient = std::make_unique<AILoopClient>();

    // No hardcoded BPM — starts in FREE mode
    // BPM will be set by the first recording

    for (int i = 0; i < sceneManager->getNumScenes(); ++i)
        sceneManager->setSceneName (i, "Scene " + juce::String (i + 1));
}

void MainComponent::setupUI()
{
    addAndMakeVisible (transportBar);
    transportBar.setFreeMode (true);

    for (int i = 0; i < loopManager->getTrackCount(); ++i)
    {
        auto* panel = new TrackPanel (i);
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

    settingsPanel.setVisible (false);
    addChildComponent (settingsPanel);

    pluginBrowser.setVisible (false);
    pluginBrowser.onPluginSelected = [this] (const juce::PluginDescription& desc)
    {
        if (pluginTargetTrack >= 0)
        {
            if (auto* lt = loopManager->getTrack (pluginTargetTrack))
                effectsManager->addExternalPlugin (lt->getTrack(), desc);
        }
        else
        {
            effectsManager->addExternalPluginToMaster (desc);
        }
        pluginBrowser.setVisible (false);
    };
    pluginBrowser.onScan = [this]
    {
        effectsManager->scanForPlugins();
        pluginBrowser.setPlugins (effectsManager->getAvailablePlugins());
    };
    addChildComponent (pluginBrowser);

    aiLoopPanel.setVisible (false);
    addChildComponent (aiLoopPanel);

    inputLevelMeter = amlpEngine.getEngine().getDeviceManager().deviceManager.getInputLevelGetter();
}

void MainComponent::connectCallbacks()
{
    transportBar.onPlayPause = [this]
    {
        auto& transport = amlpEngine.getTransport();

        if (transport.isPlaying())
        {
            // Pause — stop transport but keep track states (playheads freeze)
            transport.stop (false, false);
        }
        else
        {
            // Resume/Play — start transport and trigger all tracks with clips
            transport.play (false);

            for (int i = 0; i < loopManager->getTrackCount(); ++i)
                if (auto* t = loopManager->getTrack (i))
                    if (t->getClipInSlot (0) != nullptr && t->getState() == LoopTrack::State::idle)
                        t->triggerSlot (0);
        }
    };

    transportBar.onStop = [this]
    {
        // Full stop — reset everything to beginning
        loopManager->stopAllTracks();
        amlpEngine.getTransport().stop (false, false);
        amlpEngine.getTransport().setPosition (tracktion::TimePosition());
    };

    transportBar.onRecord = [this]
    {
        if (auto* track = loopManager->getTrack (selectedTrack))
        {
            auto s = track->getState();
            if (s == LoopTrack::State::recording
                || s == LoopTrack::State::countingIn
                || s == LoopTrack::State::waitingForThreshold)
            {
                track->stopRecording();
            }
            else
            {
                track->setThresholdRecording (settingsPanel.getThresholdRecording());
                track->startRecording (track->getActiveSlotIndex());
            }
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

    transportBar.onBpmChange = [this] (double bpm)
    {
        double oldBpm = loopManager->getMasterBPM();
        loopManager->setMasterBPM (bpm);

        // If this is the first time BPM is set manually, show bar count selector with default 4 bars
        if (transportBar.freeMode)
        {
            transportBar.setFreeMode (false);
            transportBar.setBarCount (4);
            for (int i = 0; i < loopManager->getTrackCount(); ++i)
                if (auto* lt = loopManager->getTrack (i))
                    lt->setRecordingBarCount (4);
        }

        // Time-stretch existing clips to the new BPM
        if (oldBpm > 0.0)
        {
            for (int i = 0; i < loopManager->getTrackCount(); ++i)
            {
                if (auto* lt = loopManager->getTrack (i))
                {
                    if (auto* clip = lt->getClipInSlot (lt->getActiveSlotIndex()))
                        TimeStretchManager::setClipTempo (*clip, bpm);
                }
            }
        }
    };

    transportBar.onBarCountChange = [this] (int bars)
    {
        // Set recording bar count for all tracks
        for (int i = 0; i < loopManager->getTrackCount(); ++i)
            if (auto* lt = loopManager->getTrack (i))
                lt->setRecordingBarCount (bars);
    };

    transportBar.onSettings = [this] { showSettingsPanel(); };

    transportBar.onKeyChange = [this] (const juce::String& key)
    {
        loopManager->setMasterKey (key);
    };

    transportBar.onMasterFx = [this]
    {
        pluginTargetTrack = -1; // master
        pluginBrowser.setPlugins (effectsManager->getAvailablePlugins());
        pluginBrowser.setVisible (true);
        auto centre = getLocalBounds().getCentre();
        pluginBrowser.setBounds (centre.x - 200, centre.y - 250, 400, 500);
        pluginBrowser.toFront (true);
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

        panel->onModeChanged = [this, i] (TrackPanel::RecordMode mode)
        {
            if (auto* track = loopManager->getTrack (i))
            {
                switch (mode)
                {
                    case TrackPanel::RecordMode::replace:  track->setRecordMode (LoopTrack::RecordMode::replace); break;
                    case TrackPanel::RecordMode::overdub:   track->setRecordMode (LoopTrack::RecordMode::overdub); break;
                    case TrackPanel::RecordMode::newLoop:   track->setRecordMode (LoopTrack::RecordMode::newLoop); break;
                }
            }
        };

        panel->onLoopSelected = [this, i] (int loopIndex)
        {
            if (auto* track = loopManager->getTrack (i))
                track->triggerSlot (loopIndex);
        };

        panel->onFxClicked = [this, i]
        {
            pluginTargetTrack = i;
            pluginBrowser.setPlugins (effectsManager->getAvailablePlugins());
            pluginBrowser.setVisible (true);
            auto centre = getLocalBounds().getCentre();
            pluginBrowser.setBounds (centre.x - 200, centre.y - 250, 400, 500);
            pluginBrowser.toFront (true);
        };

        panel->onAIClicked = [this, i]
        {
            aiTargetTrack = i;

            aiLoopPanel.prepareForGeneration (static_cast<int>(loopManager->getMasterBPM()), loopManager->getDetectedBarCount(), loopManager->getMasterKey());
            aiLoopPanel.setVisible (true);
            auto centre = getLocalBounds().getCentre();
            aiLoopPanel.setBounds (centre.x - 200, centre.y - 200, 400, 420);
            aiLoopPanel.toFront (true);
            aiLoopClient->ensureServerRunning();
        };
    }

    aiLoopPanel.onClose = [this] { aiLoopPanel.setVisible(false); };
    aiLoopPanel.onGenerate = [this] (const AILoopParams& params) {
        aiLoopClient->startGeneration(params);
    };

    aiLoopClient->onComplete = [this] (const juce::File& file, bool success)
    {
        if (success && aiTargetTrack >= 0 && aiTargetTrack < loopManager->getTrackCount())
        {
            if (auto* track = loopManager->getTrack (aiTargetTrack))
            {
                AudioFileImporter importer(amlpEngine.getEdit());
                int nextSlot = track->getActiveSlotIndex() + 1;
                bool foundEmpty = false;
                for (int s = 0; s < 8; ++s) {
                    if (track->getClipInSlot(s) == nullptr) {
                        nextSlot = s;
                        foundEmpty = true;
                        break;
                    }
                }
                if (!foundEmpty) nextSlot = (track->getActiveSlotIndex() + 1) % 8;

                importer.importFileWithTimeStretch (file, track->getTrack(), nextSlot, loopManager->getMasterBPM());
                track->triggerSlot(nextSlot);
                aiLoopPanel.setVisible(false);
            }
        }
    };

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
    bool bpmIsSet = loopManager->hasMasterBPM();
    
    bool hasAudio = false;
    for (int t = 0; t < loopManager->getTrackCount(); ++t) {
        if (auto* lt = loopManager->getTrack(t)) {
            if (lt->getClipInSlot(0) != nullptr || lt->getState() == LoopTrack::State::recording) {
                hasAudio = true;
                break;
            }
        }
    }

    for (int i = 0; i < trackPanels.size() && i < loopManager->getTrackCount(); ++i)
    {
        auto* loopTrack = loopManager->getTrack (i);
        auto* panel = trackPanels[i];
        if (loopTrack == nullptr || panel == nullptr)
            continue;

        auto state = loopTrack->getState();
        panel->setRecording (state == LoopTrack::State::recording);
        panel->setAIEnabled(hasAudio);

        // Count-in or threshold waiting overlay on waveform
        if (state == LoopTrack::State::waitingForThreshold)
            panel->getWaveformDisplay().setCountIn (-1); // -1 = waiting for audio
        else
            panel->getWaveformDisplay().setCountIn (loopTrack->getCountInBeatsRemaining());
        panel->setPlaying (state == LoopTrack::State::playing);
        panel->setOverdubbing (state == LoopTrack::State::overdubbing);
        panel->setArmed (i == selectedTrack);

        // Update loop dropdown
        auto loopNames = loopTrack->getLoopNames (panel->getTrackName());
        panel->setLoopList (loopNames, loopTrack->getActiveSlotIndex());

        auto& teTrack = loopTrack->getTrack();

        // Waveform display
        if (auto* clip = loopTrack->getClipInSlot (loopTrack->getActiveSlotIndex()))
        {
            auto file = clip->getOriginalFile();
            auto& waveform = panel->getWaveformDisplay();

            if (waveform.getCurrentFile() != file)
            {
                waveform.setSource (file);
                
                // Detect key and auto-set master key if missing
                auto detectedKey = KeyDetector::detectKey (file);
                waveform.setDetectedKey (detectedKey);

                if (! loopManager->hasMasterKey() && ! detectedKey.isEmpty())
                {
                    loopManager->setMasterKey (detectedKey);
                    transportBar.setKey (detectedKey);
                }
            }

            // Beat grid — use this clip's actual bar count
            if (bpmIsSet)
            {
                double barLength = (4.0 * 60.0) / loopManager->getMasterBPM();
                auto clipSeconds = clip->getPosition().getLength().inSeconds();
                int clipBars = juce::jmax (1, (int) std::round (clipSeconds / barLength));
                waveform.setBeatGrid (loopManager->getMasterBPM(), 4, clipBars);
            }

            // Playback cursor — each track uses its own bar count but BPM-derived length for sync
            if ((state == LoopTrack::State::playing || state == LoopTrack::State::overdubbing)
                && amlpEngine.getTransport().isPlaying() && bpmIsSet)
            {
                double barLength = (4.0 * 60.0) / loopManager->getMasterBPM();

                // Calculate this clip's bar count from its actual length
                auto clipSeconds = clip->getPosition().getLength().inSeconds();
                int clipBars = juce::jmax (1, (int) std::round (clipSeconds / barLength));
                double loopLength = barLength * clipBars;

                if (loopLength > 0.0)
                {
                    auto pos = amlpEngine.getTransport().getPosition().inSeconds();
                    waveform.setPlaybackPosition (std::fmod (pos, loopLength) / loopLength);
                }
            }
        }

        // Output level
        if (auto* levelPlugin = teTrack.getLevelMeterPlugin())
        {
            auto cache = levelPlugin->measurer.getLevelCache();
            panel->setOutputLevel (te::dbToGain (cache.first), te::dbToGain (cache.second));
        }

        // Input level — only on armed track
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

    // Transport state
    {
        bool anyPlaying = false;
        bool anyRecording = false;
        bool anyCountingIn = false;

        for (int i = 0; i < loopManager->getTrackCount(); ++i)
        {
            if (auto* lt = loopManager->getTrack (i))
            {
                auto s = lt->getState();
                if (s == LoopTrack::State::playing)     anyPlaying = true;
                if (s == LoopTrack::State::recording)   anyRecording = true;
                if (s == LoopTrack::State::countingIn)   anyCountingIn = true;
            }
        }

        transportBar.setPlaying (anyPlaying || amlpEngine.getTransport().isPlaying());
        transportBar.setRecording (anyRecording);
        transportBar.setCountingIn (anyCountingIn);
    }

    // Time display — wraps at longest loop length
    if (bpmIsSet && amlpEngine.getTransport().isPlaying())
    {
        // Find the longest clip length across all tracks
        double longestClipSeconds = 0.0;
        for (int i = 0; i < loopManager->getTrackCount(); ++i)
        {
            if (auto* lt = loopManager->getTrack (i))
            {
                if (auto* clip = lt->getClipInSlot (lt->getActiveSlotIndex()))
                {
                    auto len = clip->getPosition().getLength().inSeconds();
                    if (len > longestClipSeconds)
                        longestClipSeconds = len;
                }
            }
        }

        auto posSeconds = amlpEngine.getTransport().getPosition().inSeconds();

        // Wrap position within the longest loop
        if (longestClipSeconds > 0.0)
            posSeconds = std::fmod (posSeconds, longestClipSeconds);

        auto wrappedPos = tracktion::TimePosition::fromSeconds (posSeconds);
        auto barsAndBeats = amlpEngine.getEdit().tempoSequence.toBarsAndBeats (wrappedPos);
        int bar = barsAndBeats.bars + 1;
        double beatFrac = barsAndBeats.beats.inBeats();
        int beat = (int) beatFrac + 1;
        int ticks = (int) ((beatFrac - (int) beatFrac) * 100.0);
        transportBar.setBarPosition (bar, beat, ticks);
    }

    // Check if BPM was just detected (first recording completed)
    if (bpmIsSet && transportBar.freeMode)
        onBpmDetected();

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

    if (aiLoopClient)
    {
        aiLoopPanel.setServerState (aiLoopClient->getServerState(), aiLoopClient->getSetupProgress());
        aiLoopPanel.setGenerationState (aiLoopClient->getGenerationState(), aiLoopClient->getGenerationProgress(), aiLoopClient->getErrorMessage());
    }
}

void MainComponent::timerCallback()
{
    updateUI();
}

void MainComponent::onBpmDetected()
{
    transportBar.setBpm (loopManager->getMasterBPM());
    transportBar.setFreeMode (false);
    transportBar.setBarCount (loopManager->getDetectedBarCount());

    // Auto-enable metronome if settings say so
    if (settingsPanel.getAutoMetronome())
    {
        metronome->setEnabled (true);
        metronome->setRecordingOnly (false);
        transportBar.setMetronomeEnabled (true);
    }

    // Set quantization to bar boundaries now that we have a tempo
    loopManager->getQuantizeManager().setMode (QuantizeManager::QuantizeMode::bar);
}

void MainComponent::showSettingsPanel()
{
    settingsPanel.setVisible (! settingsPanel.isVisible());

    if (settingsPanel.isVisible())
    {
        auto centre = getLocalBounds().getCentre();
        settingsPanel.setBounds (centre.x - 150, centre.y - 100, 300, 200);
        settingsPanel.toFront (true);
    }
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
