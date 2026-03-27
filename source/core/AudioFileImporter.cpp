#include "AudioFileImporter.h"
#include "TimeStretchManager.h"

AudioFileImporter::AudioFileImporter (te::Edit& e)
    : edit (e)
{
}

te::WaveAudioClip::Ptr AudioFileImporter::importFile (const juce::File& audioFile,
                                                       te::AudioTrack& track,
                                                       int slotIndex)
{
    if (! audioFile.existsAsFile())
        return nullptr;

    auto slots = track.getClipSlotList().getClipSlots();
    if (slotIndex < 0 || slotIndex >= slots.size())
        return nullptr;

    auto* slot = slots[slotIndex];

    // Get audio file info for clip length
    te::AudioFile af (edit.engine, audioFile);
    auto info = af.getInfo();

    if (info.lengthInSamples == 0)
        return nullptr;

    auto lengthInSeconds = info.getLengthInSeconds();

    auto clip = te::insertWaveClip (*slot,
                                     audioFile.getFileNameWithoutExtension(),
                                     audioFile,
                                     { { tracktion::TimePosition(),
                                         tracktion::TimeDuration::fromSeconds (lengthInSeconds) } },
                                     te::DeleteExistingClips::yes);

    if (clip != nullptr)
        clip->setLoopDefaults();

    return clip;
}

te::WaveAudioClip::Ptr AudioFileImporter::importFileWithTimeStretch (const juce::File& audioFile,
                                                                      te::AudioTrack& track,
                                                                      int slotIndex,
                                                                      double targetBPM)
{
    auto clip = importFile (audioFile, track, slotIndex);

    if (clip != nullptr && targetBPM > 0.0)
    {
        clip->setAutoTempo (true);
        TimeStretchManager::matchClipToMasterTempo (*clip);
    }

    return clip;
}

juce::StringArray AudioFileImporter::getSupportedFormats()
{
    return { "*.wav", "*.aiff", "*.aif", "*.flac", "*.mp3", "*.ogg" };
}
