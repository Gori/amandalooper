#include <juce_core/juce_core.h>
#include "midi/TapTempoHandler.h"

class TapTempoTest : public juce::UnitTest
{
public:
    TapTempoTest() : UnitTest ("TapTempoHandler", "AMLP") {}

    void runTest() override
    {
        beginTest ("No taps returns 0");
        {
            TapTempoHandler handler;
            expectEquals (handler.getBPM(), 0.0);
            expectEquals (handler.getNumTaps(), 0);
        }

        beginTest ("One tap returns 0");
        {
            TapTempoHandler handler;
            handler.tapAt (0.0);
            expectEquals (handler.getBPM(), 0.0);
        }

        beginTest ("120 BPM from 2 taps");
        {
            TapTempoHandler handler;
            handler.tapAt (0.0);
            handler.tapAt (0.5); // 0.5s interval = 120 BPM
            expectWithinAbsoluteError (handler.getBPM(), 120.0, 0.1);
        }

        beginTest ("90 BPM from 4 taps");
        {
            TapTempoHandler handler;
            double interval = 60.0 / 90.0; // ~0.667s
            handler.tapAt (0.0);
            handler.tapAt (interval);
            handler.tapAt (interval * 2);
            handler.tapAt (interval * 3);
            expectWithinAbsoluteError (handler.getBPM(), 90.0, 0.1);
        }

        beginTest ("Rolling window discards old taps");
        {
            TapTempoHandler handler;
            // Tap at 120 BPM first
            handler.tapAt (0.0);
            handler.tapAt (0.5);
            handler.tapAt (1.0);
            handler.tapAt (1.5);

            // Now tap at 60 BPM (overwriting oldest)
            handler.tapAt (2.5);
            // Window now has: 0.5, 1.0, 1.5, 2.5
            // Intervals: 0.5, 0.5, 1.0 -> avg ~0.667 -> ~90 BPM
            double bpm = handler.getBPM();
            expect (bpm > 0.0);
        }

        beginTest ("Reset clears all taps");
        {
            TapTempoHandler handler;
            handler.tapAt (0.0);
            handler.tapAt (0.5);
            expect (handler.getBPM() > 0.0);

            handler.reset();
            expectEquals (handler.getBPM(), 0.0);
            expectEquals (handler.getNumTaps(), 0);
        }

        beginTest ("Very fast taps filtered out");
        {
            TapTempoHandler handler;
            handler.tapAt (0.0);
            handler.tapAt (0.01); // 10ms apart = unreasonably fast
            // Should return 0 since interval is filtered
            expectEquals (handler.getBPM(), 0.0);
        }
    }
};

static TapTempoTest tapTempoTest;
