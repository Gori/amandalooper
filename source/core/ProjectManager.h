#pragma once

#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Manages project save/load as .amlp bundles.

    A project bundle is a directory containing:
      - project.tracktionedit  (the TE Edit file)
      - audio/                 (all referenced audio files)
      - metadata.json          (BPM, track count, etc.)

    Also handles autosave.
*/
class ProjectManager : private juce::Timer
{
public:
    ProjectManager (te::Engine& engine, te::Edit& edit);
    ~ProjectManager() override;

    // ---- Save / Load ----

    /** Save the project to a .amlp bundle directory.
        Creates the directory if it doesn't exist. */
    bool saveProject (const juce::File& destination);

    /** Load a project from a .amlp bundle. Returns the new Edit, or nullptr on failure. */
    std::unique_ptr<te::Edit> loadProject (const juce::File& source);

    /** Create a fresh project with default settings. */
    std::unique_ptr<te::Edit> createNewProject (const juce::File& location);

    /** Get the current project file, or empty File if unsaved. */
    juce::File getCurrentProjectFile() const { return currentProjectFile; }

    /** Has the project been modified since last save? */
    bool isModified() const;

    // ---- Autosave ----

    /** Enable/disable periodic autosave. */
    void setAutosaveEnabled (bool enabled);
    bool isAutosaveEnabled() const { return autosaveEnabled; }

    // ---- Recent Projects ----

    juce::StringArray getRecentProjects() const;
    void addToRecentProjects (const juce::File& projectFile);

private:
    te::Engine& engine;
    te::Edit& edit;
    juce::File currentProjectFile;
    bool autosaveEnabled = true;

    void timerCallback() override;
    void copyAudioFilesToBundle (const juce::File& bundleDir);
    void writeMetadata (const juce::File& bundleDir);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectManager)
};
