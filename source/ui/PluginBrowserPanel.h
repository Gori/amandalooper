#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "AmlpLookAndFeel.h"

/**
    A panel for browsing and selecting available VST3/AU plugins.
*/
class PluginBrowserPanel : public juce::Component
{
public:
    PluginBrowserPanel();

    /** Set the list of available plugins. */
    void setPlugins (const juce::Array<juce::PluginDescription>& plugins);

    /** Callback when a plugin is selected. */
    std::function<void (const juce::PluginDescription&)> onPluginSelected;

    /** Callback to trigger a rescan. */
    std::function<void()> onScan;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::TextEditor searchBox;
    juce::ListBox pluginList;
    juce::TextButton scanButton { "Scan" };
    juce::TextButton addButton { "Add" };
    juce::TextButton cancelButton { "Cancel" };
    juce::Label titleLabel;

    juce::Array<juce::PluginDescription> allPlugins;
    juce::Array<juce::PluginDescription> filteredPlugins;

    void updateFilter();

    class PluginListModel : public juce::ListBoxModel
    {
    public:
        juce::Array<juce::PluginDescription>* plugins = nullptr;
        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override;
    };

    PluginListModel listModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBrowserPanel)
};
