#include <juce_core/juce_core.h>
#include "core/TempoDetector.h"

class TempoDetectorTest : public juce::UnitTest
{
public:
    TempoDetectorTest() : UnitTest ("TempoDetector", "AMLP") {}

    void runTest() override
    {
        beginTest ("Calculate BPM from loop length - 120 BPM, 4 bars");
        {
            // 4 bars at 120 BPM, 4/4 time = 16 beats = 8 seconds
            double length = 8.0;
            double bpm = TempoDetector::calculateBPMFromLoopLength (length);
            // With 4 bars (16 beats): 16/8*60 = 120
            expectWithinAbsoluteError (bpm, 120.0, 1.0);
        }

        beginTest ("Calculate BPM from loop length - 90 BPM, 2 bars");
        {
            // 2 bars at 90 BPM, 4/4 time = 8 beats
            // length = 8/90*60 = 5.333 seconds
            double length = (8.0 / 90.0) * 60.0;
            double bpm = TempoDetector::calculateBPMFromLoopLength (length);
            expectWithinAbsoluteError (bpm, 90.0, 2.0);
        }

        beginTest ("Calculate BPM from loop length - 140 BPM, 1 bar");
        {
            // 1 bar at 140 BPM = 4 beats
            // length = 4/140*60 = 1.714 seconds
            double length = (4.0 / 140.0) * 60.0;
            double bpm = TempoDetector::calculateBPMFromLoopLength (length);
            expectWithinAbsoluteError (bpm, 140.0, 2.0);
        }

        beginTest ("Zero/negative length returns 0");
        {
            expectEquals (TempoDetector::calculateBPMFromLoopLength (0.0), 0.0);
            expectEquals (TempoDetector::calculateBPMFromLoopLength (-1.0), 0.0);
        }

        beginTest ("Snap to musical BPM");
        {
            expectEquals (TempoDetector::snapToMusicalBPM (119.7), 120.0);
            expectEquals (TempoDetector::snapToMusicalBPM (120.3), 120.0);
            expectEquals (TempoDetector::snapToMusicalBPM (89.5), 90.0);
            expectEquals (TempoDetector::snapToMusicalBPM (0.0), 0.0);
        }

        beginTest ("Calculate loop length");
        {
            // 120 BPM, 4 bars, 4/4 = 16 beats = 8 seconds
            expectWithinAbsoluteError (
                TempoDetector::calculateLoopLength (120.0, 4),
                8.0, 0.001);

            // 90 BPM, 2 bars, 4/4 = 8 beats
            expectWithinAbsoluteError (
                TempoDetector::calculateLoopLength (90.0, 2),
                (8.0 / 90.0) * 60.0, 0.001);

            // Edge cases
            expectEquals (TempoDetector::calculateLoopLength (0.0, 4), 0.0);
            expectEquals (TempoDetector::calculateLoopLength (120.0, 0), 0.0);
        }
    }
};

static TempoDetectorTest tempoDetectorTest;
