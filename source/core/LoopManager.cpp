#include "LoopManager.h"

LoopManager::LoopManager (te::Edit& e)
    : edit (e), quantizeManager (e)
{
    rebuildLoopTracks();
}

//==============================================================================
// Track Access
//==============================================================================

int LoopManager::getTrackCount() const
{
    return static_cast<int> (loopTracks.size());
}

LoopTrack* LoopManager::getTrack (int index)
{
    if (index >= 0 && index < static_cast<int> (loopTracks.size()))
        return loopTracks[static_cast<size_t> (index)].get();

    return nullptr;
}

LoopTrack* LoopManager::addTrack()
{
    auto currentCount = te::getAudioTracks (edit).size();
    edit.ensureNumberOfAudioTracks (static_cast<int> (currentCount) + 1);

    rebuildLoopTracks();

    if (! loopTracks.empty())
        return loopTracks.back().get();

    return nullptr;
}

void LoopManager::removeTrack (int index)
{
    if (index >= 0 && index < static_cast<int> (loopTracks.size()))
    {
        auto& track = loopTracks[static_cast<size_t> (index)]->getTrack();
        edit.deleteTrack (&track);
        loopTracks.erase (loopTracks.begin() + index);
    }
}

//==============================================================================
// Master Tempo
//==============================================================================

void LoopManager::setMasterBPM (double bpm)
{
    if (bpm > 0.0)
    {
        edit.tempoSequence.insertTempo (tracktion::BeatPosition(),
                                        bpm, 0.0f);
        masterBPMSet = true;
    }
}

double LoopManager::getMasterBPM() const
{
    return edit.tempoSequence.getBpmAt (tracktion::TimePosition());
}

void LoopManager::setMasterKey (const juce::String& key)
{
    masterKey = key;
    masterKeySet = ! key.isEmpty();
}

juce::String LoopManager::getMasterKey() const
{
    return masterKey;
}

void LoopManager::detectAndSetTempoFromFirstLoop (double loopLengthSeconds)
{
    if (masterBPMSet)
        return;

    double bpm = TempoDetector::calculateBPMFromLoopLength (loopLengthSeconds);

    if (bpm > 0.0)
    {
        bpm = TempoDetector::snapToMusicalBPM (bpm);
        setMasterBPM (bpm);

        // Calculate the bar count that produced this BPM
        double beatsPerBar = 4.0;
        double totalBeats = (loopLengthSeconds * bpm) / 60.0;
        detectedBarCount = std::max (1, (int) std::round (totalBeats / beatsPerBar));
    }
}

//==============================================================================
// Global Control
//==============================================================================

void LoopManager::stopAllTracks()
{
    for (auto& lt : loopTracks)
        lt->stopSlot();
}

void LoopManager::ensureScenes (int numScenes)
{
    edit.getSceneList().ensureNumberOfScenes (numScenes);
}

//==============================================================================
// Private
//==============================================================================

void LoopManager::rebuildLoopTracks()
{
    loopTracks.clear();

    int index = 0;
    for (auto track : te::getAudioTracks (edit))
    {
        loopTracks.push_back (std::make_unique<LoopTrack> (*track, index, *this));
        ++index;
    }
}
