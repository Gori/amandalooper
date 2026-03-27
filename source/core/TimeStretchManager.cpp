#include "TimeStretchManager.h"

void TimeStretchManager::setClipTempo (te::WaveAudioClip& clip, double targetBPM)
{
    auto& loopInfo = clip.getLoopInfo();
    auto audioFileInfo = te::AudioFile (clip.edit.engine, clip.getOriginalFile()).getInfo();
    double clipBPM = loopInfo.getBpm (audioFileInfo);

    if (clipBPM > 0.0 && targetBPM > 0.0)
    {
        double ratio = clipBPM / targetBPM;
        clip.setSpeedRatio (ratio);
    }
}

void TimeStretchManager::setClipPitch (te::WaveAudioClip& clip, float semitones)
{
    clip.setPitchChange (semitones);
}

float TimeStretchManager::getClipPitch (te::WaveAudioClip& clip)
{
    return clip.getPitchChange();
}

double TimeStretchManager::getClipSpeedRatio (te::WaveAudioClip& clip)
{
    return clip.getSpeedRatio();
}

void TimeStretchManager::setClipSpeedRatio (te::WaveAudioClip& clip, double ratio)
{
    clip.setSpeedRatio (ratio);
}

void TimeStretchManager::matchClipToMasterTempo (te::WaveAudioClip& clip)
{
    double masterBPM = clip.edit.tempoSequence.getBpmAt (tracktion::TimePosition());
    setClipTempo (clip, masterBPM);
}
