#include "OverdubStack.h"

void OverdubStack::pushLayer (const juce::File& audioFile)
{
    // Remove any undone layers (they're invalidated by new work)
    layers.resize (static_cast<size_t> (activeLayers));
    layers.push_back (audioFile);
    activeLayers = static_cast<int> (layers.size());
}

juce::File OverdubStack::popLayer()
{
    if (activeLayers <= 0)
        return {};

    --activeLayers;
    return layers[static_cast<size_t> (activeLayers)];
}

juce::File OverdubStack::restoreLayer()
{
    if (activeLayers >= static_cast<int> (layers.size()))
        return {};

    auto file = layers[static_cast<size_t> (activeLayers)];
    ++activeLayers;
    return file;
}

bool OverdubStack::canUndo() const
{
    return activeLayers > 0;
}

bool OverdubStack::canRedo() const
{
    return activeLayers < static_cast<int> (layers.size());
}

std::vector<juce::File> OverdubStack::getActiveLayers() const
{
    return { layers.begin(), layers.begin() + activeLayers };
}

int OverdubStack::getNumActiveLayers() const
{
    return activeLayers;
}

int OverdubStack::getTotalLayers() const
{
    return static_cast<int> (layers.size());
}

void OverdubStack::clear()
{
    layers.clear();
    activeLayers = 0;
}

juce::File OverdubStack::getBaseLayer() const
{
    if (activeLayers <= 0)
        return {};

    return layers[0];
}

juce::File OverdubStack::getTopLayer() const
{
    if (activeLayers <= 0)
        return {};

    return layers[static_cast<size_t> (activeLayers - 1)];
}
