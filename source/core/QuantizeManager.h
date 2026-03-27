#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Controls recording quantization — snapping record start/stop to beat/bar boundaries.
*/
class QuantizeManager
{
public:
    enum class QuantizeMode
    {
        free,       // No quantization
        beat,       // Snap to nearest beat
        bar,        // Snap to nearest bar
        twoBars,    // Snap to nearest 2 bars
        fourBars    // Snap to nearest 4 bars
    };

    explicit QuantizeManager (te::Edit& edit);

    void setMode (QuantizeMode newMode);
    QuantizeMode getMode() const { return mode; }

    /** Is quantization active? */
    bool isQuantized() const { return mode != QuantizeMode::free; }

    /** Get the next quantized position after the given time. */
    tracktion::TimePosition getNextQuantizedPosition (tracktion::TimePosition currentPos) const;

    /** Get the quantize interval in beats. Returns 0 for free mode. */
    double getQuantizeIntervalBeats() const;

private:
    te::Edit& edit;
    QuantizeMode mode = QuantizeMode::free;
};
