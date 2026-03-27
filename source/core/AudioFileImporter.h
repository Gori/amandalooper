#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Imports external audio files into clip slots.
*/
class AudioFileImporter
{
public:
    explicit AudioFileImporter (te::Edit& edit);

    /** Import an audio file into a specific track and slot.
        @return The created clip, or nullptr on failure.
    */
    te::WaveAudioClip::Ptr importFile (const juce::File& audioFile,
                                        te::AudioTrack& track,
                                        int slotIndex);

    /** Import with automatic time stretching to match master tempo. */
    te::WaveAudioClip::Ptr importFileWithTimeStretch (const juce::File& audioFile,
                                                       te::AudioTrack& track,
                                                       int slotIndex,
                                                       double targetBPM);

    /** Get supported audio file extensions. */
    static juce::StringArray getSupportedFormats();

private:
    te::Edit& edit;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFileImporter)
};
