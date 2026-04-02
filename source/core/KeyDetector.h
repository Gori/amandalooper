#pragma once

#include <juce_audio_formats/juce_audio_formats.h>

/**
    Detects the musical key of an audio file using libkeyfinder.
    Stateless — all methods are static. Mirrors TempoDetector pattern.
*/
class KeyDetector
{
public:
    /**
        Detect the musical key of an audio file.

        @param audioFile  The audio file to analyse
        @return           Human-readable key string (e.g. "C major", "A minor"),
                          or empty string if detection fails or audio is silence.
    */
    static juce::String detectKey (const juce::File& audioFile);
};
