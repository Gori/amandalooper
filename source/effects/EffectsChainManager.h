#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Manages per-track and master plugin chains.
    Supports both built-in TE plugins and external VST3/AU plugins.
*/
class EffectsChainManager
{
public:
    enum class EffectType
    {
        equaliser,
        compressor,
        reverb,
        delay,
        chorus,
        phaser,
        pitchShift
    };

    explicit EffectsChainManager (te::Edit& edit);

    // ---- Built-in plugins ----
    te::Plugin::Ptr addEffect (te::AudioTrack& track, EffectType type);
    static const char* getXmlTypeName (EffectType type);
    static juce::String getEffectName (EffectType type);

    // ---- External VST3/AU plugins ----
    te::Plugin::Ptr addExternalPlugin (te::AudioTrack& track, const juce::PluginDescription& desc);
    te::Plugin::Ptr addExternalPluginToMaster (const juce::PluginDescription& desc);

    // ---- Common operations ----
    void removePlugin (te::PluginList& pluginList, int pluginIndex);
    void setBypass (te::PluginList& pluginList, int pluginIndex, bool bypassed);
    bool isBypassed (te::PluginList& pluginList, int pluginIndex) const;
    int getNumUserPlugins (te::PluginList& pluginList) const;

    // ---- Plugin scanning ----
    juce::Array<juce::PluginDescription> getAvailablePlugins() const;
    void scanForPlugins();

    // ---- Master chain ----
    te::PluginList& getMasterPluginList();

private:
    te::Edit& edit;

    te::Plugin* getUserPlugin (te::PluginList& pluginList, int index) const;
    static bool isBuiltInPlugin (te::Plugin* plugin);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsChainManager)
};
