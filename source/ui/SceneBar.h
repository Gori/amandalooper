#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AmlpLookAndFeel.h"

/**
    Bottom scene bar with scene launch buttons and stop all.
*/
class SceneBar : public juce::Component
{
public:
    SceneBar();

    std::function<void (int)> onLaunchScene;
    std::function<void()> onStopAll;

    void setNumScenes (int numScenes);
    void setActiveScene (int sceneIndex);
    void setSceneName (int index, const juce::String& name);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::OwnedArray<juce::TextButton> sceneButtons;
    juce::TextButton stopAllButton { "STOP ALL" };
    int activeScene = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneBar)
};
