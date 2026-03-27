#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Controls loop length operations on WaveAudioClips.
*/
class LoopLengthController
{
public:
    /** Double the loop length by extending the clip's loop range. */
    static void doubleLength (te::WaveAudioClip& clip);

    /** Halve the loop length by shrinking the clip's loop range. */
    static void halveLength (te::WaveAudioClip& clip);

    /** Toggle reverse playback on a clip. */
    static void toggleReverse (te::WaveAudioClip& clip);

    /** Set reverse playback on a clip. */
    static void setReversed (te::WaveAudioClip& clip, bool reversed);

    /** Get whether the clip is reversed. */
    static bool isReversed (te::WaveAudioClip& clip);
};
