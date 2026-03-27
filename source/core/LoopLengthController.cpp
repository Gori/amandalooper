#include "LoopLengthController.h"

void LoopLengthController::doubleLength (te::WaveAudioClip& clip)
{
    if (clip.isLooping())
    {
        auto loopLength = clip.getLoopLength();
        clip.setLoopRange ({ clip.getLoopStart(), loopLength * 2.0 });
    }
}

void LoopLengthController::halveLength (te::WaveAudioClip& clip)
{
    if (clip.isLooping())
    {
        auto loopLength = clip.getLoopLength();
        auto halfLength = loopLength * 0.5;

        // Don't halve below a reasonable minimum (e.g. 1 beat)
        if (halfLength.inSeconds() > 0.1)
            clip.setLoopRange ({ clip.getLoopStart(), halfLength });
    }
}

void LoopLengthController::toggleReverse (te::WaveAudioClip& clip)
{
    clip.setIsReversed (! clip.getIsReversed());
}

void LoopLengthController::setReversed (te::WaveAudioClip& clip, bool reversed)
{
    clip.setIsReversed (reversed);
}

bool LoopLengthController::isReversed (te::WaveAudioClip& clip)
{
    return clip.getIsReversed();
}
