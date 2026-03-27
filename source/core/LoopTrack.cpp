#include "LoopTrack.h"

LoopTrack::LoopTrack (te::AudioTrack& t, int index)
    : track (t), trackIndex (index)
{
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
    state = State::recording;

    armTrackInput (true);

    auto& transport = track.edit.getTransport();
    transport.record (false);
}

void LoopTrack::stopRecording()
{
    if (state != State::recording)
        return;

    auto& transport = track.edit.getTransport();
    transport.stop (false, false);

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

        auto& slotList = track.getClipSlotList();
        slotList.ensureNumberOfSlots (activeSlotIndex + 1);

        if (auto* slot = getSlot (activeSlotIndex))
        {
            recordedClip->removeFromParent();

            auto slotClip = te::insertWaveClip (*slot,
                                                 "Loop " + juce::String (activeSlotIndex),
                                                 sourceFile,
                                                 clipPos,
                                                 te::DeleteExistingClips::yes);

            if (slotClip != nullptr)
            {
                slotClip->setLoopDefaults();

                overdubStacks[activeSlotIndex].clear();
                overdubStacks[activeSlotIndex].pushLayer (sourceFile);

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
    transport.record (false);
}

void LoopTrack::stopOverdub()
{
    if (state != State::overdubbing)
        return;

    auto& transport = track.edit.getTransport();
    transport.stop (false, false);

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

void LoopTrack::armTrackInput (bool arm)
{
    for (auto instance : track.edit.getAllInputDevices())
        if (te::isOnTargetTrack (*instance, track, 0))
            instance->setRecordingEnabled (track.itemID, arm);
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
