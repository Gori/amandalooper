#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Wraps Tracktion Engine's built-in click track.
*/
class Metronome
{
public:
    explicit Metronome (te::Edit& edit);

    void setEnabled (bool enabled);
    bool isEnabled() const;

    void setVolume (float gain);
    float getVolume() const;

    void setAccentBars (bool accent);
    bool getAccentBars() const;

    /** Only play click during recording (not during playback). */
    void setRecordingOnly (bool recordingOnly);
    bool isRecordingOnly() const;

private:
    te::Edit& edit;
};
