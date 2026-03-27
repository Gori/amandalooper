#include "QuantizeManager.h"

QuantizeManager::QuantizeManager (te::Edit& e)
    : edit (e)
{
}

void QuantizeManager::setMode (QuantizeMode newMode)
{
    mode = newMode;
}

double QuantizeManager::getQuantizeIntervalBeats() const
{
    switch (mode)
    {
        case QuantizeMode::free:     return 0.0;
        case QuantizeMode::beat:     return 1.0;
        case QuantizeMode::bar:      return 4.0;
        case QuantizeMode::twoBars:  return 8.0;
        case QuantizeMode::fourBars: return 16.0;
    }
    return 0.0;
}

tracktion::TimePosition QuantizeManager::getNextQuantizedPosition (tracktion::TimePosition currentPos) const
{
    if (mode == QuantizeMode::free)
        return currentPos;

    auto& tempoSequence = edit.tempoSequence;
    auto currentBeat = tempoSequence.toBeats (currentPos);
    double interval = getQuantizeIntervalBeats();

    // Snap to next interval boundary
    double beatVal = currentBeat.inBeats();
    double nextBeat = std::ceil (beatVal / interval) * interval;

    // If we're exactly on a boundary, go to the next one
    if (std::abs (nextBeat - beatVal) < 0.001)
        nextBeat += interval;

    return tempoSequence.toTime (tracktion::BeatPosition::fromBeats (nextBeat));
}
