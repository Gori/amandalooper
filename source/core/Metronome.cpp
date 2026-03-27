#include "Metronome.h"

Metronome::Metronome (te::Edit& e)
    : edit (e)
{
}

void Metronome::setEnabled (bool enabled)
{
    edit.clickTrackEnabled = enabled;
}

bool Metronome::isEnabled() const
{
    return edit.clickTrackEnabled;
}

void Metronome::setVolume (float gain)
{
    edit.setClickTrackVolume (gain);
}

float Metronome::getVolume() const
{
    return edit.clickTrackGain.get();
}

void Metronome::setAccentBars (bool accent)
{
    edit.clickTrackEmphasiseBars = accent;
}

bool Metronome::getAccentBars() const
{
    return edit.clickTrackEmphasiseBars;
}

void Metronome::setRecordingOnly (bool recordingOnly)
{
    edit.clickTrackRecordingOnly = recordingOnly;
}

bool Metronome::isRecordingOnly() const
{
    return edit.clickTrackRecordingOnly;
}
