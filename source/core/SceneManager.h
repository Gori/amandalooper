#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Manages scenes (horizontal slices across all tracks).

    A scene is a set of clip slots — one per track. Launching a scene
    triggers all clips in that row across all tracks simultaneously.
*/
class SceneManager
{
public:
    explicit SceneManager (te::Edit& edit);

    // ---- Scene CRUD ----
    int getNumScenes() const;
    juce::String getSceneName (int index) const;
    void setSceneName (int index, const juce::String& name);

    void createScene (const juce::String& name = {});
    void deleteScene (int index);
    void duplicateScene (int index);

    // ---- Scene Control ----

    /** Launch all clips in a scene row. Existing clips keep playing until
        the new ones start (quantized). */
    void launchScene (int index);

    /** Stop all clips in a scene row. */
    void stopScene (int index);

    /** Stop all clips across all scenes. */
    void stopAllScenes();

    /** Get the currently active scene index, or -1 if none. */
    int getActiveSceneIndex() const { return activeSceneIndex; }

private:
    te::Edit& edit;
    int activeSceneIndex = -1;

    te::Scene* getScene (int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneManager)
};
