#include "SceneManager.h"

SceneManager::SceneManager (te::Edit& e)
    : edit (e)
{
}

//==============================================================================
// Scene CRUD
//==============================================================================

int SceneManager::getNumScenes() const
{
    return edit.getSceneList().getNumScenes();
}

juce::String SceneManager::getSceneName (int index) const
{
    auto scenes = edit.getSceneList().getScenes();
    if (index >= 0 && index < scenes.size())
        return scenes[index]->state.getProperty (te::IDs::name).toString();

    return {};
}

void SceneManager::setSceneName (int index, const juce::String& name)
{
    auto scenes = edit.getSceneList().getScenes();
    if (index >= 0 && index < scenes.size())
        scenes[index]->state.setProperty (te::IDs::name, name, &edit.getUndoManager());
}

void SceneManager::createScene (const juce::String& name)
{
    int insertIdx = edit.getSceneList().getNumScenes();
    auto* scene = edit.getSceneList().insertScene (insertIdx);

    if (scene != nullptr && name.isNotEmpty())
        scene->state.setProperty (te::IDs::name, name, nullptr);
}

void SceneManager::deleteScene (int index)
{
    if (auto* scene = getScene (index))
        edit.getSceneList().deleteScene (*scene);
}

void SceneManager::duplicateScene (int index)
{
    // Create a new scene at the end
    // Note: clip duplication within slots would require copying clips,
    // which we'll handle when we have the full bounce system.
    auto* sourceScene = getScene (index);
    if (sourceScene == nullptr)
        return;

    int insertIdx = edit.getSceneList().getNumScenes();
    auto* newScene = edit.getSceneList().insertScene (insertIdx);
    if (newScene != nullptr)
        newScene->state.setProperty (te::IDs::name,
                                      sourceScene->name.get() + " (copy)",
                                      nullptr);
}

//==============================================================================
// Scene Control
//==============================================================================

void SceneManager::launchScene (int index)
{
    if (index < 0 || index >= getNumScenes())
        return;

    // Launch all clips in this scene row across all tracks
    for (auto track : te::getAudioTracks (edit))
    {
        auto slots = track->getClipSlotList().getClipSlots();
        if (index < slots.size())
        {
            if (auto* clip = slots[index]->getClip())
            {
                if (auto handle = clip->getLaunchHandle())
                    handle->play ({});
            }
        }
    }

    activeSceneIndex = index;
}

void SceneManager::stopScene (int index)
{
    if (index < 0 || index >= getNumScenes())
        return;

    for (auto track : te::getAudioTracks (edit))
    {
        auto slots = track->getClipSlotList().getClipSlots();
        if (index < slots.size())
        {
            if (auto* clip = slots[index]->getClip())
            {
                if (auto handle = clip->getLaunchHandle())
                    handle->stop ({});
            }
        }
    }

    if (activeSceneIndex == index)
        activeSceneIndex = -1;
}

void SceneManager::stopAllScenes()
{
    for (int i = 0; i < getNumScenes(); ++i)
        stopScene (i);

    activeSceneIndex = -1;
}

//==============================================================================
// Private
//==============================================================================

te::Scene* SceneManager::getScene (int index)
{
    auto scenes = edit.getSceneList().getScenes();
    if (index >= 0 && index < scenes.size())
        return scenes[index];

    return nullptr;
}
