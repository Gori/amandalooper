#pragma once

#include <tracktion_engine/tracktion_engine.h>
#include "LoopTrack.h"
#include "TempoDetector.h"
#include "QuantizeManager.h"

namespace te = tracktion::engine;

/**
    Orchestrates all LoopTracks.

    Manages the collection of loop tracks, the master tempo,
    and the first-loop-sets-tempo workflow.
*/
class LoopManager
{
public:
    explicit LoopManager (te::Edit& edit);

    // ---- Track Access ----
    int getTrackCount() const;
    LoopTrack* getTrack (int index);
    LoopTrack* addTrack();
    void removeTrack (int index);

    // ---- Master Tempo ----
    void setMasterBPM (double bpm);
    double getMasterBPM() const;
    bool hasMasterBPM() const { return masterBPMSet; }

    /**
        Call after the first loop is recorded to auto-detect BPM.
        Uses TempoDetector to calculate BPM from the loop length
        and sets the edit's tempo.
    */
    void detectAndSetTempoFromFirstLoop (double loopLengthSeconds);
    int getDetectedBarCount() const { return detectedBarCount; }

    // ---- Global Control ----
    void stopAllTracks();

    // ---- Scene Management ----
    void ensureScenes (int numScenes);

    // ---- Quantization ----
    QuantizeManager& getQuantizeManager() { return quantizeManager; }
    const QuantizeManager& getQuantizeManager() const { return quantizeManager; }

    te::Edit& getEdit() { return edit; }

private:
    te::Edit& edit;
    std::vector<std::unique_ptr<LoopTrack>> loopTracks;
    bool masterBPMSet = false;
    int detectedBarCount = 0;
    QuantizeManager quantizeManager;

    void rebuildLoopTracks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopManager)
};
