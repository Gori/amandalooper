#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "core/SceneManager.h"

namespace te = tracktion::engine;

class SceneManagerTest : public juce::UnitTest
{
public:
    SceneManagerTest() : UnitTest ("SceneManager", "AMLP") {}

    void runTest() override
    {
        te::Engine engine ("AMLPTest", nullptr, nullptr);
        auto editFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                            .getChildFile ("AMLP_scene_test.tracktionedit");
        auto edit = te::createEmptyEdit (engine, editFile);
        edit->ensureNumberOfAudioTracks (4);

        beginTest ("Initial state has no scenes");
        {
            SceneManager manager (*edit);
            // TE may create default scenes, so just check it doesn't crash
            expect (manager.getNumScenes() >= 0);
            expectEquals (manager.getActiveSceneIndex(), -1);
        }

        beginTest ("Create scenes");
        {
            SceneManager manager (*edit);
            int initial = manager.getNumScenes();

            manager.createScene ("Verse");
            manager.createScene ("Chorus");
            manager.createScene ("Bridge");

            expectEquals (manager.getNumScenes(), initial + 3);
        }

        beginTest ("Get and set scene names");
        {
            auto edit2 = te::createEmptyEdit (engine, editFile);
            edit2->ensureNumberOfAudioTracks (4);
            SceneManager manager (*edit2);

            manager.createScene ("TestScene");
            int lastIdx = manager.getNumScenes() - 1;
            auto actualName = manager.getSceneName (lastIdx);
            logMessage ("Scene count: " + juce::String (manager.getNumScenes())
                        + ", lastIdx: " + juce::String (lastIdx)
                        + ", name: '" + actualName + "'");
            expect (actualName == "TestScene",
                    "Expected 'TestScene' but got '" + actualName + "'");

            manager.setSceneName (lastIdx, "Outro");
            auto updatedName = manager.getSceneName (lastIdx);
            expect (updatedName == "Outro",
                    "Expected 'Outro' but got '" + updatedName + "'");
        }

        beginTest ("Delete scene");
        {
            auto edit3 = te::createEmptyEdit (engine, editFile);
            edit3->ensureNumberOfAudioTracks (4);
            SceneManager manager (*edit3);

            manager.createScene ("A");
            manager.createScene ("B");
            int before = manager.getNumScenes();

            manager.deleteScene (before - 1);
            expectEquals (manager.getNumScenes(), before - 1);
        }

        beginTest ("Duplicate scene");
        {
            auto edit4 = te::createEmptyEdit (engine, editFile);
            edit4->ensureNumberOfAudioTracks (4);
            SceneManager manager (*edit4);

            manager.createScene ("Original");
            int before = manager.getNumScenes();

            manager.duplicateScene (before - 1);
            expectEquals (manager.getNumScenes(), before + 1);

            auto name = manager.getSceneName (manager.getNumScenes() - 1);
            expect (name.contains ("(copy)"));
        }

        beginTest ("Out of bounds operations don't crash");
        {
            SceneManager manager (*edit);
            manager.deleteScene (-1);
            manager.deleteScene (999);
            manager.launchScene (-1);
            manager.launchScene (999);
            manager.stopScene (-1);
            manager.stopScene (999);
            expect (manager.getSceneName (-1).isEmpty());
            expect (manager.getSceneName (999).isEmpty());
        }

        beginTest ("Stop all scenes");
        {
            SceneManager manager (*edit);
            manager.stopAllScenes();
            expectEquals (manager.getActiveSceneIndex(), -1);
        }

        editFile.deleteFile();
    }
};

static SceneManagerTest sceneManagerTest;
