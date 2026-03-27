#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Manages multiple audio input assignment to tracks.

    Allows assigning specific physical inputs (e.g., guitar on input 1,
    vocals on input 2) to different looper tracks.
*/
class InputRouter
{
public:
    explicit InputRouter (te::Edit& edit);

    /** Get available wave (audio) input device names. */
    juce::StringArray getAvailableInputs() const;

    /** Assign a physical audio input to a track.
        @param inputIndex   Index into getAvailableInputs()
        @param track        The track to assign the input to
    */
    void assignInputToTrack (int inputIndex, te::AudioTrack& track);

    /** Remove input assignment from a track. */
    void unassignInput (te::AudioTrack& track);

    /** Get the name of the input currently assigned to a track, or empty. */
    juce::String getAssignedInputName (te::AudioTrack& track) const;

    /** Set input monitoring mode for a track. */
    void setMonitoring (te::AudioTrack& track, te::InputDevice::MonitorMode mode);

    /** Get input monitoring mode for a track. */
    te::InputDevice::MonitorMode getMonitoring (te::AudioTrack& track) const;

    /** Initialize audio device with given channel counts. */
    void initialiseAudioDevice (int numInputChannels = 2, int numOutputChannels = 2);

private:
    te::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputRouter)
};
