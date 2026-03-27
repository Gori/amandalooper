#pragma once

#include <juce_core/juce_core.h>
#include <vector>

/**
    Manages overdub layers for a single clip slot.

    Each layer is an audio file. The "current" state is the result of
    mixing all active layers together. Undo removes the last layer,
    redo restores it.

    Thread safety: all methods must be called on the message thread.
*/
class OverdubStack
{
public:
    OverdubStack() = default;

    /** Add a new overdub layer. Clears any undone layers. */
    void pushLayer (const juce::File& audioFile);

    /** Remove the top layer (undo). Returns the removed file, or empty File if stack is empty. */
    juce::File popLayer();

    /** Restore the last undone layer (redo). Returns the restored file, or empty File if nothing to redo. */
    juce::File restoreLayer();

    /** Returns true if there are layers that can be undone. */
    bool canUndo() const;

    /** Returns true if there are layers that can be redone. */
    bool canRedo() const;

    /** Returns all currently active layers (not including undone ones). */
    std::vector<juce::File> getActiveLayers() const;

    /** Returns the number of active layers. */
    int getNumActiveLayers() const;

    /** Returns the total number of layers including undone ones. */
    int getTotalLayers() const;

    /** Clear all layers. */
    void clear();

    /** Get the base layer (first recording). Returns empty File if no layers. */
    juce::File getBaseLayer() const;

    /** Get the top (most recent) active layer. Returns empty File if no layers. */
    juce::File getTopLayer() const;

private:
    std::vector<juce::File> layers;
    int activeLayers = 0;  // Number of active layers (layers before this index are active)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OverdubStack)
};
