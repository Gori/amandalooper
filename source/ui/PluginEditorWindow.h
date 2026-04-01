#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include "AmlpLookAndFeel.h"

namespace te = tracktion::engine;

/**
    A window that hosts an external plugin's native editor UI.
*/
class PluginEditorWindow : public juce::DocumentWindow
{
public:
    PluginEditorWindow (te::ExternalPlugin& plugin)
        : DocumentWindow (plugin.getName(),
                          AmlpLookAndFeel::getPanelColour(),
                          DocumentWindow::closeButton)
    {
        if (auto* proc = plugin.getAudioPluginInstance())
        {
            if (auto* editor = proc->createEditorIfNeeded())
            {
                setContentOwned (editor, true);
                setResizable (true, false);
                centreWithSize (getWidth(), getHeight());
                setVisible (true);
            }
        }
    }

    void closeButtonPressed() override
    {
        delete this;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditorWindow)
};
