#include "SceneBar.h"

SceneBar::SceneBar()
{
    stopAllButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getRecordingColour().darker (0.5f));
    stopAllButton.onClick = [this] { if (onStopAll) onStopAll(); };
    addAndMakeVisible (stopAllButton);
}

void SceneBar::setNumScenes (int numScenes)
{
    sceneButtons.clear();
    for (int i = 0; i < numScenes; ++i)
    {
        auto* btn = new juce::TextButton ("Scene " + juce::String (i + 1));
        btn->setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
        btn->onClick = [this, i] { if (onLaunchScene) onLaunchScene (i); };
        addAndMakeVisible (btn);
        sceneButtons.add (btn);
    }
    resized();
}

void SceneBar::setActiveScene (int sceneIndex)
{
    activeScene = sceneIndex;
    for (int i = 0; i < sceneButtons.size(); ++i)
    {
        sceneButtons[i]->setColour (juce::TextButton::buttonColourId,
            i == sceneIndex ? AmlpLookAndFeel::getPlayingColour().darker (0.3f)
                            : AmlpLookAndFeel::getSurfaceColour());
    }
}

void SceneBar::setSceneName (int index, const juce::String& name)
{
    if (index >= 0 && index < sceneButtons.size())
        sceneButtons[index]->setButtonText (name);
}

void SceneBar::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
}

void SceneBar::resized()
{
    auto bounds = getLocalBounds().reduced (6);

    stopAllButton.setBounds (bounds.removeFromRight (100));
    bounds.removeFromRight (6);

    int btnW = sceneButtons.isEmpty() ? 0 : std::min (100, bounds.getWidth() / sceneButtons.size());
    for (auto* btn : sceneButtons)
    {
        btn->setBounds (bounds.removeFromLeft (btnW).reduced (2));
    }
}
