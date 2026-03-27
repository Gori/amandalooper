#pragma once

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "OverdubStack.h"

namespace te = tracktion::engine;

/**
    A looper track wrapping a te::AudioTrack.

    Manages the record/play/overdub state machine for one track.
    Each track has N clip slots (one per scene). Only one slot can be
    active (playing/recording) at a time.

    Overdubs use multi-clip layering: existing clips keep playing while
    a new clip is recorded on top. Undo removes the last layer.

    Thread safety: all methods must be called on the message thread.
*/
class LoopTrack
{
public:
    enum class State
    {
        idle,
        recording,
        playing,
        overdubbing
    };

    LoopTrack (te::AudioTrack& track, int index);

    // ---- State Machine ----
    State getState() const { return state; }
    int getIndex() const { return trackIndex; }

    /** Start recording into the given slot. */
    void startRecording (int slotIndex);

    /** Stop recording. Creates the clip, starts looped playback. */
    void stopRecording();

    /** Start overdubbing on the active slot (records new layer while existing plays). */
    void startOverdub();

    /** Stop overdubbing. Finalizes the new layer. */
    void stopOverdub();

    /** Undo the last overdub layer. Gapless — existing audio keeps playing. */
    void undoLastOverdub();

    /** Redo the last undone overdub. */
    void redoOverdub();

    // ---- Slot Control ----
    /** Trigger (play) a specific slot. */
    void triggerSlot (int slotIndex);

    /** Stop the currently active slot. */
    void stopSlot();

    /** Get the currently active slot index, or -1 if none. */
    int getActiveSlotIndex() const { return activeSlotIndex; }

    // ---- Track Properties ----
    te::AudioTrack& getTrack() { return track; }
    const te::AudioTrack& getTrack() const { return track; }

    /** Mute/unmute this track. */
    void setMuted (bool muted);
    bool isMuted() const;

    /** Solo/unsolo this track. */
    void setSolo (bool solo);
    bool isSolo() const;

    /** Get the overdub stack for the active slot. */
    OverdubStack* getActiveOverdubStack();

    /** Get the clip currently in a slot, or nullptr. */
    te::WaveAudioClip* getClipInSlot (int slotIndex);

private:
    te::AudioTrack& track;
    int trackIndex;
    State state = State::idle;
    int activeSlotIndex = -1;

    // One OverdubStack per slot
    std::map<int, OverdubStack> overdubStacks;

    // Helpers
    void armTrackInput (bool arm);
    te::ClipSlot* getSlot (int slotIndex);
    juce::File getRecordingDirectory();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopTrack)
};
