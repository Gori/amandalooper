#pragma once

/**
    Calculates BPM from loop length and other tempo utilities.
    No dependency on Tracktion Engine — pure math.
*/
class TempoDetector
{
public:
    /**
        Calculate BPM from a recorded loop's duration.
        Assumes the loop is a whole number of bars in the given time signature.

        @param lengthInSeconds  Duration of the recorded loop
        @param beatsPerBar      Beats per bar (e.g. 4 for 4/4 time)
        @param minBPM           Minimum plausible BPM (default 40)
        @param maxBPM           Maximum plausible BPM (default 240)
        @return                 Estimated BPM, or 0 if no plausible BPM found
    */
    static double calculateBPMFromLoopLength (double lengthInSeconds,
                                              int beatsPerBar = 4,
                                              double minBPM = 40.0,
                                              double maxBPM = 240.0);

    /**
        Snap a raw BPM to the nearest "musical" BPM.
        Musical BPMs are whole numbers, with preference for common tempos
        (60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180).

        @param rawBPM   The calculated BPM
        @return         Snapped BPM (rounded to nearest integer)
    */
    static double snapToMusicalBPM (double rawBPM);

    /**
        Calculate the loop length in seconds for a given BPM and bar count.

        @param bpm          Tempo in beats per minute
        @param numBars      Number of bars
        @param beatsPerBar  Beats per bar (default 4)
        @return             Duration in seconds
    */
    static double calculateLoopLength (double bpm, int numBars, int beatsPerBar = 4);
};
