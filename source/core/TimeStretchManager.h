#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Wraps Tracktion Engine's time stretching and pitch shifting capabilities.
*/
class TimeStretchManager
{
public:
    /** Set a clip's playback speed to match a target BPM.
        The clip must have LoopInfo with a known BPM. */
    static void setClipTempo (te::WaveAudioClip& clip, double targetBPM);

    /** Pitch shift a clip by a number of semitones (-48 to +48). */
    static void setClipPitch (te::WaveAudioClip& clip, float semitones);

    /** Get the current pitch shift in semitones. */
    static float getClipPitch (te::WaveAudioClip& clip);

    /** Get the current speed ratio. */
    static double getClipSpeedRatio (te::WaveAudioClip& clip);

    /** Set the speed ratio directly. */
    static void setClipSpeedRatio (te::WaveAudioClip& clip, double ratio);

    /** Auto-match a clip to the edit's master tempo. */
    static void matchClipToMasterTempo (te::WaveAudioClip& clip);
};
