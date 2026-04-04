#include "LoopTrack.h"
#include "LoopManager.h"
#include <cmath>

LoopTrack::LoopTrack (te::AudioTrack& t, int index, LoopManager& lm)
    : track (t),
      manager (lm),
      trackIndex (index),
      inputLevelMeter (t.edit.engine.getDeviceManager().deviceManager.getInputLevelGetter())
{
}

LoopTrack::~LoopTrack()
{
    stopTimer();
}

//==============================================================================
// Recording
//==============================================================================

void LoopTrack::startRecording (int slotIndex)
{
    if (state != State::idle && state != State::playing)
        return;

    // Determine target slot — don't switch activeSlotIndex yet,
    // current clip keeps playing during count-in
    switch (recordMode)
    {
        case RecordMode::newLoop:
            pendingRecordSlot = getNumLoops();
            break;
        case RecordMode::replace:
            pendingRecordSlot = (slotIndex >= 0) ? slotIndex : 0;
            break;
        case RecordMode::overdub:
            pendingRecordSlot = (slotIndex >= 0) ? slotIndex : 0;
            break;
    }

    armTrackInput (true);

    {
        auto logFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
            .getChildFile (".amlp").getChildFile ("amlp_debug.log");
        auto tempDir = track.edit.getTempDirectory (false);
        logFile.appendText ("startRecording: tempDir exists=" + juce::String (tempDir.isDirectory() ? "true" : "false")
            + " path=" + tempDir.getFullPathName() + "\n");
    }

    if (manager.hasMasterBPM())
    {
        // BPM is set — count in to next bar boundary, then record
        // Current clip keeps playing during count-in
        state = State::countingIn;

        auto& transport = track.edit.getTransport();
        if (! transport.isPlaying())
            transport.play (false);

        // Count-in: wait until the next loop cycle restart (01:01:00).
        // If the loop is 4 bars and we're at bar 6 (= bar 2 of cycle), target is bar 8 (next cycle start).
        auto pos = transport.getPosition();
        auto barsAndBeats = track.edit.tempoSequence.toBarsAndBeats (pos);
        int loopBars = manager.getDetectedBarCount();
        if (loopBars <= 0) loopBars = 4;

        int currentBar = barsAndBeats.bars;
        int barsIntoLoop = currentBar % loopBars;
        countInTargetBar = currentBar + (loopBars - barsIntoLoop);

        startTimerHz (60);
    }
    else
    {
        // Free mode — first loop sets BPM
        // Stop current clip before recording
        if (activeSlotIndex >= 0)
        {
            if (auto* clip = getClipInSlot (activeSlotIndex))
                if (auto handle = clip->getLaunchHandle())
                    handle->stop ({});
        }
        activeSlotIndex = pendingRecordSlot;

        if (thresholdRecording)
        {
            // Wait for audio input to exceed threshold before recording
            state = State::waitingForThreshold;
            startTimerHz (120);
        }
        else
        {
            // Record immediately
            autoStopTimeSeconds = 0.0;

            auto& transport = track.edit.getTransport();
            if (! transport.isPlaying())
            {
                // Start transport first, then record on next message loop
                // iteration so the audio graph is fully initialized
                transport.play (false);
                state = State::countingIn; // brief transitional state
                juce::Timer::callAfterDelay (50, [this]
                {
                    state = State::recording;
                    auto& t = track.edit.getTransport();
                    t.record (false);
                    recordingStartTime = t.getPosition().inSeconds();
                });
            }
            else
            {
                state = State::recording;
                transport.record (false);
                recordingStartTime = transport.getPosition().inSeconds();
            }
        }
    }
}

void LoopTrack::timerCallback()
{
    if (state == State::waitingForThreshold)
    {
        // Check the live device-manager input meter. Keeping a persistent
        // reference ensures JUCE continues producing level data.
        if (inputLevelMeter != nullptr)
        {
            const auto level = (float) inputLevelMeter->getCurrentLevel();

            if (level >= thresholdLevel)
            {
                // Audio detected — start recording
                stopTimer();
                state = State::recording;
                autoStopTimeSeconds = 0.0;

                auto& transport = track.edit.getTransport();
                if (! transport.isPlaying())
                    transport.play (false);

                transport.record (false);
                recordingStartTime = transport.getPosition().inSeconds();
                return;
            }
        }

        return;
    }

    auto& transport = track.edit.getTransport();

    if (! transport.isPlaying())
        return;

    auto pos = transport.getPosition();
    auto barsAndBeats = track.edit.tempoSequence.toBarsAndBeats (pos);
    int currentBar = barsAndBeats.bars;

    if (state == State::countingIn)
    {
        if (currentBar >= countInTargetBar)
        {
            stopTimer();

            // Stop the current clip — recording is starting on this track
            if (activeSlotIndex >= 0)
            {
                if (auto* clip = getClipInSlot (activeSlotIndex))
                    if (auto handle = clip->getLaunchHandle())
                        handle->stop ({});
            }
            activeSlotIndex = pendingRecordSlot;

            state = State::recording;

            transport.record (false);
            recordingStartTime = pos.inSeconds();

            // Calculate auto-stop time
            int bars = recordingBarCount > 0 ? recordingBarCount : manager.getDetectedBarCount();
            if (bars <= 0) bars = 4;

            double bpm = manager.getMasterBPM();
            autoStopTimeSeconds = (bars * 4.0 * 60.0) / bpm;

            startTimerHz (60);
        }
    }
    else if (state == State::recording && autoStopTimeSeconds > 0.0)
    {
        double elapsed = pos.inSeconds() - recordingStartTime;
        if (elapsed >= autoStopTimeSeconds)
        {
            stopTimer();
            stopRecording();
        }
    }
}

void LoopTrack::stopRecording()
{
    if (state != State::recording)
    {
        // Cancel count-in or threshold wait
        if (state == State::countingIn || state == State::waitingForThreshold)
        {
            stopTimer();
            armTrackInput (false);
            state = State::idle;
        }
        return;
    }

    stopTimer();

    auto& transport = track.edit.getTransport();
    auto stopPos = transport.getPosition().inSeconds();

    transport.stopRecording();
    armTrackInput (false);

    // Allow TE to finalize the recording clip before checking
    track.edit.dispatchPendingUpdatesSynchronously();

    auto logFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
        .getChildFile (".amlp").getChildFile ("amlp_debug.log");

    logFile.appendText ("stopRecording: startPos=" + juce::String (recordingStartTime, 3)
        + " stopPos=" + juce::String (stopPos, 3)
        + " duration=" + juce::String (stopPos - recordingStartTime, 3) + "\n");

    auto clips = track.getClips();
    logFile.appendText ("stopRecording: clips on track=" + juce::String (clips.size()) + "\n");

    // Also check all tracks in the edit for clips
    for (auto* t : te::getAudioTracks (track.edit))
    {
        auto tc = t->getClips();
        if (tc.size() > 0)
            logFile.appendText ("  track '" + t->getName() + "' has " + juce::String (tc.size()) + " clips\n");
    }

    // Check temp dir for recent recordings
    auto tempDir = track.edit.getTempDirectory (false);
    if (tempDir.isDirectory())
    {
        auto files = tempDir.findChildFiles (juce::File::findFiles, false, "*.wav");
        logFile.appendText ("  temp dir: " + juce::String (files.size()) + " wav files\n");
        for (auto& f : files)
            logFile.appendText ("    " + f.getFileName() + " (" + juce::String (f.getSize()) + " bytes)\n");
    }

    te::WaveAudioClip* recordedClip = nullptr;
    for (int i = clips.size() - 1; i >= 0; --i)
    {
        if (auto* wac = dynamic_cast<te::WaveAudioClip*> (clips[i]))
        {
            recordedClip = wac;
            break;
        }
    }

    if (recordedClip != nullptr)
    {
        auto sourceFile = recordedClip->getOriginalFile();
        auto clipPos = recordedClip->getPosition();
        auto clipLengthSeconds = clipPos.getLength().inSeconds();

        // Auto-detect BPM from first recording
        bool isFirstRecording = ! manager.hasMasterBPM();
        if (isFirstRecording && clipLengthSeconds > 0.0)
            manager.detectAndSetTempoFromFirstLoop (clipLengthSeconds);

        auto& slotList = track.getClipSlotList();
        slotList.ensureNumberOfSlots (activeSlotIndex + 1);

        if (auto* slot = getSlot (activeSlotIndex))
        {
            recordedClip->removeFromParent();

            // Normalize clip to start at time 0 so all loops sync to transport origin
            auto normalizedPos = te::ClipPosition { { tracktion::TimePosition(),
                                                       clipPos.getLength() } };

            auto slotClip = te::insertWaveClip (*slot,
                                                 "Loop " + juce::String (activeSlotIndex),
                                                 sourceFile,
                                                 normalizedPos,
                                                 te::DeleteExistingClips::yes);

            if (slotClip != nullptr)
            {
                // Quantize clip length to exact bar boundaries so audio and playhead match
                if (manager.hasMasterBPM())
                {
                    double barLength = (4.0 * 60.0) / manager.getMasterBPM();
                    int clipBars = juce::jmax (1, (int) std::round (clipLengthSeconds / barLength));
                    auto exactLength = tracktion::TimeDuration::fromSeconds (barLength * clipBars);

                    slotClip->setPosition ({ { tracktion::TimePosition(), exactLength } });
                }

                slotClip->setLoopDefaults();

                overdubStacks[activeSlotIndex].clear();
                overdubStacks[activeSlotIndex].pushLayer (sourceFile);

                auto& transport = track.edit.getTransport();

                if (! transport.isPlaying())
                    transport.play (false);

                if (auto handle = slotClip->getLaunchHandle())
                {
                    if (auto referenceHandle = manager.getPlayingLaunchHandle (this))
                    {
                        if (auto* playbackContext = transport.getCurrentPlaybackContext())
                        {
                            if (auto syncPoint = playbackContext->getSyncPoint())
                                handle->playSynced (*referenceHandle, syncPoint->monotonicBeat);
                            else
                                handle->play ({});
                        }
                        else
                        {
                            handle->play ({});
                        }
                    }
                    else
                    {
                        handle->play ({});
                    }
                }

                state = State::playing;
                return;
            }
        }
    }

    state = State::idle;
}

//==============================================================================
// Overdubbing
//==============================================================================

void LoopTrack::startOverdub()
{
    if (state != State::playing || activeSlotIndex < 0)
        return;

    state = State::overdubbing;
    armTrackInput (true);

    auto& transport = track.edit.getTransport();
    if (! transport.isPlaying())
        transport.play (false);

    transport.record (false);
}

void LoopTrack::stopOverdub()
{
    if (state != State::overdubbing)
        return;

    track.edit.getTransport().stopRecording();
    armTrackInput (false);

    auto clips = track.getClips();
    if (clips.size() > 0)
    {
        if (auto* newClip = dynamic_cast<te::WaveAudioClip*> (clips.getLast()))
        {
            auto newFile = newClip->getOriginalFile();
            newClip->removeFromParent();
            overdubStacks[activeSlotIndex].pushLayer (newFile);
        }
    }

    state = State::playing;
}

void LoopTrack::undoLastOverdub()
{
    if (activeSlotIndex < 0)
        return;

    auto& stack = overdubStacks[activeSlotIndex];
    if (stack.canUndo())
        stack.popLayer();
}

void LoopTrack::redoOverdub()
{
    if (activeSlotIndex < 0)
        return;

    auto& stack = overdubStacks[activeSlotIndex];
    if (stack.canRedo())
        stack.restoreLayer();
}

//==============================================================================
// Slot Control
//==============================================================================

void LoopTrack::triggerSlot (int slotIndex)
{
    if (auto* clip = getClipInSlot (slotIndex))
    {
        if (auto handle = clip->getLaunchHandle())
        {
            auto& transport = track.edit.getTransport();
            if (! transport.isPlaying())
                transport.play (false);

            if (auto referenceHandle = manager.getPlayingLaunchHandle (this))
            {
                if (auto* playbackContext = transport.getCurrentPlaybackContext())
                {
                    if (auto syncPoint = playbackContext->getSyncPoint())
                        handle->playSynced (*referenceHandle, syncPoint->monotonicBeat);
                    else
                        handle->play ({});
                }
                else
                {
                    handle->play ({});
                }
            }
            else
            {
                handle->play ({});
            }

            activeSlotIndex = slotIndex;
            state = State::playing;
        }
    }
}

void LoopTrack::stopSlot()
{
    if (activeSlotIndex >= 0)
    {
        if (auto* clip = getClipInSlot (activeSlotIndex))
        {
            if (auto handle = clip->getLaunchHandle())
                handle->stop ({});
        }
    }

    state = State::idle;
}

//==============================================================================
// Track Properties
//==============================================================================

void LoopTrack::setMuted (bool muted)  { track.setMute (muted); }
bool LoopTrack::isMuted() const        { return track.isMuted (false); }
void LoopTrack::setSolo (bool solo)    { track.setSolo (solo); }
bool LoopTrack::isSolo() const         { return track.isSolo (false); }

OverdubStack* LoopTrack::getActiveOverdubStack()
{
    if (activeSlotIndex < 0)
        return nullptr;

    return &overdubStacks[activeSlotIndex];
}

te::WaveAudioClip* LoopTrack::getClipInSlot (int slotIndex)
{
    if (auto* slot = getSlot (slotIndex))
        return dynamic_cast<te::WaveAudioClip*> (slot->getClip());

    return nullptr;
}

std::optional<double> LoopTrack::getPlaybackProgress()
{
    if (! manager.hasMasterBPM())
        return std::nullopt;

    auto* clip = getClipInSlot (activeSlotIndex);
    if (clip == nullptr)
        return std::nullopt;

    auto clipSeconds = clip->getPosition().getLength().inSeconds();
    if (clipSeconds <= 0.0)
        return std::nullopt;

    // Derive position from the ONE master clock — the transport
    auto transportSeconds = track.edit.getTransport().getPosition().inSeconds();
    return std::fmod (transportSeconds, clipSeconds) / clipSeconds;
}

//==============================================================================
// Helpers
//==============================================================================

juce::StringArray LoopTrack::getLoopNames (const juce::String& trackName) const
{
    juce::StringArray names;
    auto slots = track.getClipSlotList().getClipSlots();

    for (int i = 0; i < slots.size(); ++i)
    {
        if (dynamic_cast<te::WaveAudioClip*> (slots[i]->getClip()) != nullptr)
            names.add (trackName + " - " + juce::String (i + 1));
    }

    return names;
}

int LoopTrack::getNumLoops() const
{
    int count = 0;
    auto slots = track.getClipSlotList().getClipSlots();

    for (auto* slot : slots)
        if (slot->getClip() != nullptr)
            ++count;

    return count;
}

int LoopTrack::getCountInBeatsRemaining() const
{
    if (state != State::countingIn)
        return 0;

    auto pos = track.edit.getTransport().getPosition();
    auto barsAndBeats = track.edit.tempoSequence.toBarsAndBeats (pos);

    int barsRemaining = countInTargetBar - barsAndBeats.bars;
    int beatsPerBar = 4;
    int beatsRemainingInCurrentBar = beatsPerBar - (int) barsAndBeats.beats.inBeats();

    int totalBeats = (barsRemaining - 1) * beatsPerBar + beatsRemainingInCurrentBar;
    return juce::jmax (0, totalBeats);
}

void LoopTrack::armTrackInput (bool arm)
{
    auto logFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
        .getChildFile (".amlp").getChildFile ("amlp_debug.log");

    bool found = false;

    for (auto instance : track.edit.getAllInputDevices())
    {
        if (te::isOnTargetTrack (*instance, track, 0))
        {
            instance->setRecordingEnabled (track.itemID, arm);
            found = true;
            logFile.appendText ("armTrackInput: PRIMARY path, arm=" + juce::String (arm ? "true" : "false") + "\n");
        }
    }

    if (! found && arm)
    {
        logFile.appendText ("armTrackInput: FALLBACK path (no existing input)\n");
        for (auto instance : track.edit.getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
            {
                [[maybe_unused]] const auto targetAssignment = instance->setTarget (track.itemID, true, &track.edit.getUndoManager(), 0);
                instance->setRecordingEnabled (track.itemID, true);
                logFile.appendText ("  setTarget done, recEnabled=" + juce::String (instance->isRecordingEnabled (track.itemID) ? "true" : "false") + "\n");
                break;
            }
        }
    }

    if (! found && ! arm)
        logFile.appendText ("armTrackInput: disarm, no input found\n");
}

te::ClipSlot* LoopTrack::getSlot (int slotIndex)
{
    auto slots = track.getClipSlotList().getClipSlots();
    if (slotIndex >= 0 && slotIndex < slots.size())
        return slots[slotIndex];

    return nullptr;
}

juce::File LoopTrack::getRecordingDirectory()
{
    return track.edit.getTempDirectory (true);
}
