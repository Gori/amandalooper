#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <tracktion_engine/tracktion_engine.h>
#include <functional>

namespace te = tracktion::engine;

/**
    Handles bounce/resampling operations.

    All bounce operations run on a background thread and call the
    completion callback on the message thread when done.
*/
class BounceEngine
{
public:
    using CompletionCallback = std::function<void (const juce::File& result, bool success)>;

    explicit BounceEngine (te::Edit& edit);

    /**
        Bounce (mix down) multiple audio files into a single file.
        Used for flattening overdub layers.

        @param sourceFiles      Files to mix together
        @param outputFile       Destination file
        @param sampleRate       Sample rate for output
        @param onComplete       Called on message thread when done
    */
    void bounceFiles (const std::vector<juce::File>& sourceFiles,
                      const juce::File& outputFile,
                      double sampleRate,
                      CompletionCallback onComplete);

    /**
        Bounce a track's output over a time range to a file.

        @param track            Track to render
        @param timeRange        Time range to render
        @param outputFile       Destination file
        @param includeEffects   Whether to include track effects
        @param onComplete       Called on message thread when done
    */
    void bounceTrack (te::AudioTrack& track,
                      tracktion::TimeRange timeRange,
                      const juce::File& outputFile,
                      bool includeEffects,
                      CompletionCallback onComplete);

    /** Cancel any running bounce operation. */
    void cancelAll();

    /** Is a bounce currently running? */
    bool isBouncing() const { return bouncing.load(); }

private:
    te::Edit& edit;
    std::atomic<bool> bouncing { false };
    juce::ThreadPool threadPool { 1 };

    void mixAudioFiles (const std::vector<juce::File>& sourceFiles,
                        const juce::File& outputFile,
                        double sampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BounceEngine)
};
