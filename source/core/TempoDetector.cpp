#include "TempoDetector.h"
#include <cmath>
#include <initializer_list>

double TempoDetector::calculateBPMFromLoopLength (double lengthInSeconds,
                                                   int beatsPerBar,
                                                   double minBPM,
                                                   double maxBPM)
{
    if (lengthInSeconds <= 0.0 || beatsPerBar <= 0)
        return 0.0;

    // Try different bar counts (1, 2, 4, 8, 16, 32) and find the one
    // that produces a BPM in the plausible range closest to common tempos.
    double bestBPM = 0.0;
    double bestScore = 1e9;

    for (int numBars : { 1, 2, 4, 8, 16, 32 })
    {
        double totalBeats = numBars * beatsPerBar;
        double bpm = (totalBeats / lengthInSeconds) * 60.0;

        if (bpm < minBPM || bpm > maxBPM)
            continue;

        // Score: prefer BPMs closer to common tempos (80-160 range)
        // and prefer smaller bar counts (simpler interpretation)
        double distFromCenter = std::abs (bpm - 120.0);
        double barPenalty = numBars * 0.5;
        double score = distFromCenter + barPenalty;

        if (score < bestScore)
        {
            bestScore = score;
            bestBPM = bpm;
        }
    }

    return bestBPM;
}

double TempoDetector::snapToMusicalBPM (double rawBPM)
{
    if (rawBPM <= 0.0)
        return 0.0;

    return std::round (rawBPM);
}

double TempoDetector::calculateLoopLength (double bpm, int numBars, int beatsPerBar)
{
    if (bpm <= 0.0 || numBars <= 0 || beatsPerBar <= 0)
        return 0.0;

    double totalBeats = numBars * beatsPerBar;
    return (totalBeats / bpm) * 60.0;
}
