#include "LoopTrack.h"
#include "LoopManager.h"

LoopTrack::LoopTrack (te::AudioTrack& t, int index, LoopManager& lm)
    : track (t), manager (lm), trackIndex (index)
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

    if (state == State::playing && activeSlotIndex != slotIndex)
        stopSlot();

    activeSlotIndex = slotIndex;
    armTrackInput (true);

    if (manager.hasMasterBPM())
    {
        // BPM is set — count in to next bar boundary, then record
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
        // Free mode — record immediately, first loop sets BPM
        state = State::recording;
        autoStopTimeSeconds = 0.0;

        auto& transport = track.edit.getTransport();
        if (! transport.isPlaying())
            transport.play (false);

        transport.record (false);
        recordingStartTime = transport.getPosition().inSeconds();
    }
}

void LoopTrack::timerCallback()
{
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
        // Cancel count-in
        if (state == State::countingIn)
        {
            stopTimer();
            armTrackInput (false);
            state = State::idle;
        }
        return;
    }

    stopTimer();

    track.edit.getTransport().stopRecording();
    armTrackInput (false);

    auto clips = track.getClips();

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
        if (! manager.hasMasterBPM() && clipLengthSeconds > 0.0)
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
                slotClip->setLoopDefaults();

                overdubStacks[activeSlotIndex].clear();
                overdubStacks[activeSlotIndex].pushLayer (sourceFile);

                auto& transport = track.edit.getTransport();
                if (! transport.isPlaying())
                    transport.play (false);

                if (auto handle = slotClip->getLaunchHandle())
                    handle->play ({});

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

            handle->play ({});
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
    activeSlotIndex = -1;
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

//==============================================================================
// Helpers
//==============================================================================

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
    bool found = false;

    for (auto instance : track.edit.getAllInputDevices())
    {
        if (te::isOnTargetTrack (*instance, track, 0))
        {
            instance->setRecordingEnabled (track.itemID, arm);
            found = true;
        }
    }

    if (! found && arm)
    {
        for (auto instance : track.edit.getAllInputDevices())
        {
            if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
            {
                instance->setTarget (track.itemID, true, &track.edit.getUndoManager(), 0);
                instance->setRecordingEnabled (track.itemID, true);
                break;
            }
        }
    }
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
