#include "EffectsChainManager.h"

EffectsChainManager::EffectsChainManager (te::Edit& e)
    : edit (e)
{
}

//==============================================================================
// Built-in plugins
//==============================================================================

te::Plugin::Ptr EffectsChainManager::addEffect (te::AudioTrack& track, EffectType type)
{
    auto xmlType = getXmlTypeName (type);
    auto plugin = edit.getPluginCache().createNewPlugin (xmlType, {});

    if (plugin != nullptr)
        track.pluginList.insertPlugin (plugin, -1, nullptr);

    return plugin;
}

const char* EffectsChainManager::getXmlTypeName (EffectType type)
{
    switch (type)
    {
        case EffectType::equaliser:   return te::EqualiserPlugin::xmlTypeName;
        case EffectType::compressor:  return te::CompressorPlugin::xmlTypeName;
        case EffectType::reverb:      return te::ReverbPlugin::xmlTypeName;
        case EffectType::delay:       return te::DelayPlugin::xmlTypeName;
        case EffectType::chorus:      return te::ChorusPlugin::xmlTypeName;
        case EffectType::phaser:      return te::PhaserPlugin::xmlTypeName;
        case EffectType::pitchShift:  return te::PitchShiftPlugin::xmlTypeName;
    }
    return "";
}

juce::String EffectsChainManager::getEffectName (EffectType type)
{
    switch (type)
    {
        case EffectType::equaliser:   return "EQ";
        case EffectType::compressor:  return "Compressor";
        case EffectType::reverb:      return "Reverb";
        case EffectType::delay:       return "Delay";
        case EffectType::chorus:      return "Chorus";
        case EffectType::phaser:      return "Phaser";
        case EffectType::pitchShift:  return "Pitch Shift";
    }
    return "Unknown";
}

//==============================================================================
// External VST3/AU plugins
//==============================================================================

te::Plugin::Ptr EffectsChainManager::addExternalPlugin (te::AudioTrack& track,
                                                         const juce::PluginDescription& desc)
{
    auto plugin = edit.getPluginCache().createNewPlugin (te::ExternalPlugin::xmlTypeName, desc);

    if (plugin != nullptr)
        track.pluginList.insertPlugin (plugin, -1, nullptr);

    return plugin;
}

te::Plugin::Ptr EffectsChainManager::addExternalPluginToMaster (const juce::PluginDescription& desc)
{
    auto plugin = edit.getPluginCache().createNewPlugin (te::ExternalPlugin::xmlTypeName, desc);

    if (plugin != nullptr)
        edit.getMasterPluginList().insertPlugin (plugin, -1, nullptr);

    return plugin;
}

//==============================================================================
// Common operations
//==============================================================================

void EffectsChainManager::removePlugin (te::PluginList& pluginList, int pluginIndex)
{
    if (auto* plugin = getUserPlugin (pluginList, pluginIndex))
        plugin->removeFromParent();
}

void EffectsChainManager::setBypass (te::PluginList& pluginList, int pluginIndex, bool bypassed)
{
    if (auto* plugin = getUserPlugin (pluginList, pluginIndex))
        plugin->setEnabled (! bypassed);
}

bool EffectsChainManager::isBypassed (te::PluginList& pluginList, int pluginIndex) const
{
    if (auto* plugin = getUserPlugin (pluginList, pluginIndex))
        return ! plugin->isEnabled();

    return false;
}

int EffectsChainManager::getNumUserPlugins (te::PluginList& pluginList) const
{
    int count = 0;
    for (auto plugin : pluginList)
        if (! isBuiltInPlugin (plugin))
            ++count;

    return count;
}

//==============================================================================
// Plugin scanning
//==============================================================================

juce::Array<juce::PluginDescription> EffectsChainManager::getAvailablePlugins() const
{
    return edit.engine.getPluginManager().knownPluginList.getTypes();
}

void EffectsChainManager::scanForPlugins()
{
    auto& pm = edit.engine.getPluginManager();
    auto& formatManager = pm.pluginFormatManager;
    auto& knownList = pm.knownPluginList;

    for (int i = 0; i < formatManager.getNumFormats(); ++i)
    {
        auto* format = formatManager.getFormat (i);
        auto searchPath = format->getDefaultLocationsToSearch();

        for (int j = 0; j < searchPath.getNumPaths(); ++j)
        {
            auto dir = searchPath[j];
            auto files = dir.findChildFiles (juce::File::findFiles, true);

            for (auto& file : files)
            {
                juce::OwnedArray<juce::PluginDescription> found;
                format->findAllTypesForFile (found, file.getFullPathName());

                for (auto* desc : found)
                    knownList.addType (*desc);
            }
        }
    }
}

//==============================================================================
// Master chain
//==============================================================================

te::PluginList& EffectsChainManager::getMasterPluginList()
{
    return edit.getMasterPluginList();
}

//==============================================================================
// Private
//==============================================================================

te::Plugin* EffectsChainManager::getUserPlugin (te::PluginList& pluginList, int index) const
{
    int count = 0;
    for (auto plugin : pluginList)
    {
        if (! isBuiltInPlugin (plugin))
        {
            if (count == index)
                return plugin;
            ++count;
        }
    }
    return nullptr;
}

bool EffectsChainManager::isBuiltInPlugin (te::Plugin* plugin)
{
    return dynamic_cast<te::VolumeAndPanPlugin*> (plugin) != nullptr
        || dynamic_cast<te::LevelMeterPlugin*> (plugin) != nullptr;
}
