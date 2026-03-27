#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
    Manages per-track effects chains using Tracktion Engine's built-in plugins.
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

    /** Add an effect to a track's plugin chain. Returns the created plugin, or nullptr. */
    te::Plugin::Ptr addEffect (te::AudioTrack& track, EffectType type);

    /** Remove an effect from a track by index in the plugin list. */
    void removeEffect (te::AudioTrack& track, int pluginIndex);

    /** Bypass/unbypass an effect on a track. */
    void setBypass (te::AudioTrack& track, int pluginIndex, bool bypassed);

    /** Check if an effect is bypassed. */
    bool isBypassed (te::AudioTrack& track, int pluginIndex) const;

    /** Get number of user plugins on a track (excludes built-in volume/level plugins). */
    int getNumEffects (te::AudioTrack& track) const;

    /** Get the XML type name for an effect type. */
    static const char* getXmlTypeName (EffectType type);

    /** Get a human-readable name for an effect type. */
    static juce::String getEffectName (EffectType type);

private:
    te::Edit& edit;

    te::Plugin* getUserPlugin (te::AudioTrack& track, int index) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsChainManager)
};
