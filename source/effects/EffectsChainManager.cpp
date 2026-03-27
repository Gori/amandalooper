#include "EffectsChainManager.h"

EffectsChainManager::EffectsChainManager (te::Edit& e)
    : edit (e)
{
}

te::Plugin::Ptr EffectsChainManager::addEffect (te::AudioTrack& track, EffectType type)
{
    auto xmlType = getXmlTypeName (type);
    auto plugin = edit.getPluginCache().createNewPlugin (xmlType, {});

    if (plugin != nullptr)
        track.pluginList.insertPlugin (plugin, -1, nullptr);

    return plugin;
}

void EffectsChainManager::removeEffect (te::AudioTrack& track, int pluginIndex)
{
    if (auto* plugin = getUserPlugin (track, pluginIndex))
        plugin->removeFromParent();
}

void EffectsChainManager::setBypass (te::AudioTrack& track, int pluginIndex, bool bypassed)
{
    if (auto* plugin = getUserPlugin (track, pluginIndex))
        plugin->setEnabled (! bypassed);
}

bool EffectsChainManager::isBypassed (te::AudioTrack& track, int pluginIndex) const
{
    if (auto* plugin = getUserPlugin (track, pluginIndex))
        return ! plugin->isEnabled();

    return false;
}

int EffectsChainManager::getNumEffects (te::AudioTrack& track) const
{
    int count = 0;
    for (auto plugin : track.pluginList)
    {
        // Skip built-in plugins (volume, level meter, etc.)
        if (dynamic_cast<te::VolumeAndPanPlugin*> (plugin) == nullptr
            && dynamic_cast<te::LevelMeterPlugin*> (plugin) == nullptr)
        {
            ++count;
        }
    }
    return count;
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
// Private
//==============================================================================

te::Plugin* EffectsChainManager::getUserPlugin (te::AudioTrack& track, int index) const
{
    int count = 0;
    for (auto plugin : track.pluginList)
    {
        if (dynamic_cast<te::VolumeAndPanPlugin*> (plugin) == nullptr
            && dynamic_cast<te::LevelMeterPlugin*> (plugin) == nullptr)
        {
            if (count == index)
                return plugin;
            ++count;
        }
    }
    return nullptr;
}
