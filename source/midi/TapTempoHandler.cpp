#include "TapTempoHandler.h"

void TapTempoHandler::tap()
{
    tapAt (juce::Time::getMillisecondCounterHiRes() / 1000.0);
}

void TapTempoHandler::tapAt (double timeInSeconds)
{
    tapTimes[static_cast<size_t> (writeIndex)] = timeInSeconds;
    writeIndex = (writeIndex + 1) % maxTaps;

    if (numTaps < maxTaps)
        ++numTaps;
}

double TapTempoHandler::getBPM() const
{
    if (numTaps < 2)
        return 0.0;

    // Calculate average interval between taps
    double totalInterval = 0.0;
    int intervals = 0;

    // Walk through the taps in order
    int count = numTaps;
    int readIndex = (writeIndex - count + maxTaps) % maxTaps;

    for (int i = 0; i < count - 1; ++i)
    {
        int idx1 = (readIndex + i) % maxTaps;
        int idx2 = (readIndex + i + 1) % maxTaps;

        double interval = tapTimes[static_cast<size_t> (idx2)]
                        - tapTimes[static_cast<size_t> (idx1)];

        // Filter out unreasonable intervals (< 0.15s = >400 BPM, > 3s = <20 BPM)
        if (interval > 0.15 && interval < 3.0)
        {
            totalInterval += interval;
            ++intervals;
        }
    }

    if (intervals == 0)
        return 0.0;

    double avgInterval = totalInterval / intervals;
    return 60.0 / avgInterval;
}

void TapTempoHandler::reset()
{
    numTaps = 0;
    writeIndex = 0;
    tapTimes.fill (0.0);
}
