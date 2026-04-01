#include "PluginBrowserPanel.h"

int PluginBrowserPanel::PluginListModel::getNumRows()
{
    return plugins ? plugins->size() : 0;
}

void PluginBrowserPanel::PluginListModel::paintListBoxItem (int row, juce::Graphics& g,
                                                             int w, int h, bool selected)
{
    if (plugins == nullptr || row < 0 || row >= plugins->size())
        return;

    const auto& desc = (*plugins)[row];

    if (selected)
        g.fillAll (AmlpLookAndFeel::getSurfaceColour());

    g.setColour (AmlpLookAndFeel::getTextColour());
    g.setFont (juce::FontOptions (13.0f));
    g.drawText (desc.name + "  (" + desc.pluginFormatName + ")",
                6, 0, w - 12, h / 2, juce::Justification::centredLeft);

    g.setColour (AmlpLookAndFeel::getDimTextColour());
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (desc.manufacturerName,
                6, h / 2, w - 12, h / 2, juce::Justification::centredLeft);
}

//==============================================================================

PluginBrowserPanel::PluginBrowserPanel()
{
    titleLabel.setText ("Add Plugin", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, AmlpLookAndFeel::getTextColour());
    addAndMakeVisible (titleLabel);

    searchBox.setTextToShowWhenEmpty ("Search plugins...", AmlpLookAndFeel::getDimTextColour());
    searchBox.setColour (juce::TextEditor::backgroundColourId, AmlpLookAndFeel::getSurfaceColour());
    searchBox.setColour (juce::TextEditor::textColourId, AmlpLookAndFeel::getTextColour());
    searchBox.onTextChange = [this] { updateFilter(); };
    addAndMakeVisible (searchBox);

    listModel.plugins = &filteredPlugins;
    pluginList.setModel (&listModel);
    pluginList.setColour (juce::ListBox::backgroundColourId, AmlpLookAndFeel::getBackgroundColour());
    pluginList.setRowHeight (36);
    addAndMakeVisible (pluginList);

    scanButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    scanButton.onClick = [this] { if (onScan) onScan(); };
    addAndMakeVisible (scanButton);

    addButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getPlayingColour().darker (0.3f));
    addButton.onClick = [this]
    {
        auto row = pluginList.getSelectedRow();
        if (row >= 0 && row < filteredPlugins.size() && onPluginSelected)
            onPluginSelected (filteredPlugins[row]);
    };
    addAndMakeVisible (addButton);

    cancelButton.setColour (juce::TextButton::buttonColourId, AmlpLookAndFeel::getSurfaceColour());
    cancelButton.onClick = [this] { setVisible (false); };
    addAndMakeVisible (cancelButton);

    setSize (400, 500);
}

void PluginBrowserPanel::setPlugins (const juce::Array<juce::PluginDescription>& plugins)
{
    allPlugins = plugins;
    updateFilter();
}

void PluginBrowserPanel::updateFilter()
{
    auto searchText = searchBox.getText().toLowerCase();

    filteredPlugins.clear();

    for (auto& desc : allPlugins)
    {
        if (searchText.isEmpty()
            || desc.name.toLowerCase().contains (searchText)
            || desc.manufacturerName.toLowerCase().contains (searchText))
        {
            filteredPlugins.add (desc);
        }
    }

    pluginList.updateContent();
    pluginList.repaint();
}

void PluginBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (AmlpLookAndFeel::getPanelColour());
    g.setColour (AmlpLookAndFeel::getSurfaceColour());
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1), 6.0f, 2.0f);
}

void PluginBrowserPanel::resized()
{
    auto bounds = getLocalBounds().reduced (12);

    titleLabel.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (6);
    searchBox.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (6);

    auto buttons = bounds.removeFromBottom (30);
    cancelButton.setBounds (buttons.removeFromRight (70));
    buttons.removeFromRight (6);
    addButton.setBounds (buttons.removeFromRight (70));
    buttons.removeFromRight (6);
    scanButton.setBounds (buttons.removeFromRight (70));

    bounds.removeFromBottom (6);
    pluginList.setBounds (bounds);
}
