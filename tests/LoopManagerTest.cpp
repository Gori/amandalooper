#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "core/AmlpEngine.h"
#include "core/LoopManager.h"

namespace te = tracktion::engine;

class LoopManagerTest : public juce::UnitTest
{
public:
    LoopManagerTest() : UnitTest ("LoopManager", "AMLP") {}

    void runTest() override
    {
        // Create a real engine + edit for integration testing
        te::Engine engine ("AMLPTest", nullptr, nullptr);
        auto editFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                            .getChildFile ("AMLP_test.tracktionedit");
        auto edit = te::createEmptyEdit (engine, editFile);
        edit->ensureNumberOfAudioTracks (4);
        edit->getTransport().ensureContextAllocated();

        beginTest ("LoopManager initializes with correct track count");
        {
            LoopManager manager (*edit);
            expectEquals (manager.getTrackCount(), 4);

            for (int i = 0; i < 4; ++i)
            {
                auto* track = manager.getTrack (i);
                expect (track != nullptr);
                expectEquals (track->getIndex(), i);
                expectEquals (static_cast<int> (track->getState()), static_cast<int> (LoopTrack::State::idle));
            }
        }

        beginTest ("LoopManager out-of-bounds returns nullptr");
        {
            LoopManager manager (*edit);
            expect (manager.getTrack (-1) == nullptr);
            expect (manager.getTrack (4) == nullptr);
            expect (manager.getTrack (100) == nullptr);
        }

        beginTest ("Master BPM defaults and can be set");
        {
            LoopManager manager (*edit);
            expect (! manager.hasMasterBPM());

            manager.setMasterBPM (120.0);
            expect (manager.hasMasterBPM());
            expectWithinAbsoluteError (manager.getMasterBPM(), 120.0, 0.1);
        }

        beginTest ("Detect BPM from first loop");
        {
            // Reset tempo
            auto edit2 = te::createEmptyEdit (engine, editFile);
            edit2->ensureNumberOfAudioTracks (4);

            LoopManager manager (*edit2);
            expect (! manager.hasMasterBPM());

            // 8 seconds = 4 bars at 120 BPM (4/4 time)
            manager.detectAndSetTempoFromFirstLoop (8.0);
            expect (manager.hasMasterBPM());
            expectWithinAbsoluteError (manager.getMasterBPM(), 120.0, 1.0);
        }

        beginTest ("Detect BPM only happens once");
        {
            auto edit3 = te::createEmptyEdit (engine, editFile);
            edit3->ensureNumberOfAudioTracks (4);

            LoopManager manager (*edit3);
            manager.detectAndSetTempoFromFirstLoop (8.0); // sets ~120 BPM

            double firstBPM = manager.getMasterBPM();
            manager.detectAndSetTempoFromFirstLoop (4.0); // would be different BPM, but should be ignored
            expectWithinAbsoluteError (manager.getMasterBPM(), firstBPM, 0.1);
        }

        beginTest ("Ensure scenes creates slots on all tracks");
        {
            auto edit4 = te::createEmptyEdit (engine, editFile);
            edit4->ensureNumberOfAudioTracks (4);

            LoopManager manager (*edit4);
            manager.ensureScenes (8);

            expectEquals (edit4->getSceneList().getNumScenes(), 8);
        }

        beginTest ("Track mute/solo");
        {
            LoopManager manager (*edit);
            auto* track = manager.getTrack (0);
            expect (track != nullptr);

            expect (! track->isMuted());
            track->setMuted (true);
            expect (track->isMuted());
            track->setMuted (false);
            expect (! track->isMuted());

            expect (! track->isSolo());
            track->setSolo (true);
            expect (track->isSolo());
            track->setSolo (false);
        }

        beginTest ("LoopTrack initial state");
        {
            LoopManager manager (*edit);
            auto* track = manager.getTrack (0);
            expect (track != nullptr);

            expectEquals (static_cast<int> (track->getState()), static_cast<int> (LoopTrack::State::idle));
            expectEquals (track->getActiveSlotIndex(), -1);
            expect (track->getActiveOverdubStack() == nullptr);
        }

        // Cleanup
        editFile.deleteFile();
    }
};

static LoopManagerTest loopManagerTest;
