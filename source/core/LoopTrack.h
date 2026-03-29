#pragma once

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "OverdubStack.h"

namespace te = tracktion::engine;

class LoopManager;

/**
    A looper track wrapping a te::AudioTrack.

    Manages the record/play/overdub state machine for one track.
    Supports count-in (waits for beat 1) and auto-stop (fixed bar length)
    when BPM is set.

    Thread safety: all methods must be called on the message thread.
*/
class LoopTrack : private juce::Timer
{
public:
    enum class State
    {
        idle,
        countingIn,
        recording,
        playing,
        overdubbing
    };

    LoopTrack (te::AudioTrack& track, int index, LoopManager& loopManager);
    ~LoopTrack() override;

    // ---- State Machine ----
    State getState() const { return state; }
    int getIndex() const { return trackIndex; }

    void startRecording (int slotIndex);
    void stopRecording();
    void startOverdub();
    void stopOverdub();
    void undoLastOverdub();
    void redoOverdub();

    // ---- Slot Control ----
    void triggerSlot (int slotIndex);
    void stopSlot();
    int getActiveSlotIndex() const { return activeSlotIndex; }

    // ---- Track Properties ----
    te::AudioTrack& getTrack() { return track; }
    const te::AudioTrack& getTrack() const { return track; }

    void setMuted (bool muted);
    bool isMuted() const;
    void setSolo (bool solo);
    bool isSolo() const;

    OverdubStack* getActiveOverdubStack();
    te::WaveAudioClip* getClipInSlot (int slotIndex);

    void setRecordingBarCount (int bars) { recordingBarCount = bars; }
    int getRecordingBarCount() const { return recordingBarCount; }

    /** Get count-in info for UI display. Returns beats remaining (0 = not counting in). */
    int getCountInBeatsRemaining() const;

private:
    te::AudioTrack& track;
    LoopManager& manager;
    int trackIndex;
    State state = State::idle;
    int activeSlotIndex = -1;
    int recordingBarCount = 0; // 0 = match first loop

    // Count-in and auto-stop
    int countInTargetBar = 0;
    double autoStopTimeSeconds = 0.0;
    double recordingStartTime = 0.0;

    std::map<int, OverdubStack> overdubStacks;

    void armTrackInput (bool arm);
    te::ClipSlot* getSlot (int slotIndex);
    juce::File getRecordingDirectory();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopTrack)
};
