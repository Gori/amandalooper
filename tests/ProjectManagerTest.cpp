#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "core/ProjectManager.h"

namespace te = tracktion::engine;

class ProjectManagerTest : public juce::UnitTest
{
public:
    ProjectManagerTest() : UnitTest ("ProjectManager", "AMLP") {}

    void runTest() override
    {
        te::Engine engine ("AMLPTest", nullptr, nullptr);
        auto editFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                            .getChildFile ("AMLP_project_test.tracktionedit");
        auto edit = te::createEmptyEdit (engine, editFile);
        edit->ensureNumberOfAudioTracks (4);
        edit->getTransport().ensureContextAllocated();

        beginTest ("Save project creates bundle");
        {
            ProjectManager manager (engine, *edit);

            auto bundleDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                 .getChildFile ("test_project.amlp");
            bundleDir.deleteRecursively();

            expect (manager.saveProject (bundleDir));
            expect (bundleDir.isDirectory());
            expect (bundleDir.getChildFile ("project.tracktionedit").existsAsFile());
            expect (bundleDir.getChildFile ("metadata.json").existsAsFile());
            expect (bundleDir.getChildFile ("audio").isDirectory());

            // Check metadata content
            auto metadata = bundleDir.getChildFile ("metadata.json").loadFileAsString();
            expect (metadata.contains ("\"version\""));
            expect (metadata.contains ("\"bpm\""));
            expect (metadata.contains ("\"trackCount\""));

            // Cleanup
            bundleDir.deleteRecursively();
        }

        beginTest ("Load project returns valid edit");
        {
            ProjectManager manager (engine, *edit);

            auto bundleDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                 .getChildFile ("test_project_load.amlp");
            bundleDir.deleteRecursively();

            manager.saveProject (bundleDir);

            auto loadedEdit = manager.loadProject (bundleDir);
            expect (loadedEdit != nullptr);

            if (loadedEdit != nullptr)
            {
                auto tracks = te::getAudioTracks (*loadedEdit);
                expectEquals (static_cast<int> (tracks.size()), 4);
            }

            bundleDir.deleteRecursively();
        }

        beginTest ("Load nonexistent project returns nullptr");
        {
            ProjectManager manager (engine, *edit);

            auto badDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                              .getChildFile ("nonexistent.amlp");

            auto result = manager.loadProject (badDir);
            expect (result == nullptr);
        }

        beginTest ("Create new project");
        {
            ProjectManager manager (engine, *edit);

            auto bundleDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                 .getChildFile ("test_new_project.amlp");
            bundleDir.deleteRecursively();

            auto newEdit = manager.createNewProject (bundleDir);
            expect (newEdit != nullptr);

            if (newEdit != nullptr)
            {
                auto tracks = te::getAudioTracks (*newEdit);
                expectEquals (static_cast<int> (tracks.size()), 4);
                expectEquals (newEdit->getSceneList().getNumScenes(), 8);
            }

            bundleDir.deleteRecursively();
        }

        editFile.deleteFile();
    }
};

static ProjectManagerTest projectManagerTest;
