#pragma once

#include <juce_core/juce_core.h>
#include <array>

/**
    Calculates BPM from tap intervals.

    Uses a rolling window of the last 4 taps. After 2+ taps,
    getBPM() returns the calculated tempo.
*/
class TapTempoHandler
{
public:
    TapTempoHandler() = default;

    /** Record a tap at the current time. */
    void tap();

    /** Record a tap at a specific time (for testing). */
    void tapAt (double timeInSeconds);

    /** Get the calculated BPM, or 0 if not enough taps. */
    double getBPM() const;

    /** Reset all taps. */
    void reset();

    /** Get number of taps recorded. */
    int getNumTaps() const { return numTaps; }

private:
    static constexpr int maxTaps = 4;
    std::array<double, maxTaps> tapTimes {};
    int numTaps = 0;
    int writeIndex = 0;
};
