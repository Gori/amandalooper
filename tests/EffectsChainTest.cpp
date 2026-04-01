#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>
#include "effects/EffectsChainManager.h"

namespace te = tracktion::engine;

class EffectsChainTest : public juce::UnitTest
{
public:
    EffectsChainTest() : UnitTest ("EffectsChainManager", "AMLP") {}

    void runTest() override
    {
        te::Engine engine ("AMLPTest", nullptr, nullptr);
        auto editFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                            .getChildFile ("AMLP_effects_test.tracktionedit");
        auto edit = te::createEmptyEdit (engine, editFile);
        edit->ensureNumberOfAudioTracks (2);

        auto tracks = te::getAudioTracks (*edit);
        auto* track = tracks[0];

        beginTest ("Initially no user effects");
        {
            EffectsChainManager manager (*edit);
            expectEquals (manager.getNumUserPlugins (track->pluginList), 0);
        }

        beginTest ("Add effects");
        {
            EffectsChainManager manager (*edit);

            auto reverb = manager.addEffect (*track, EffectsChainManager::EffectType::reverb);
            expect (reverb != nullptr);
            expectEquals (manager.getNumUserPlugins (track->pluginList), 1);

            auto delay = manager.addEffect (*track, EffectsChainManager::EffectType::delay);
            expect (delay != nullptr);
            expectEquals (manager.getNumUserPlugins (track->pluginList), 2);
        }

        beginTest ("Bypass and unbypass");
        {
            EffectsChainManager manager (*edit);
            expect (! manager.isBypassed (track->pluginList, 0));

            manager.setBypass (track->pluginList, 0, true);
            expect (manager.isBypassed (track->pluginList, 0));

            manager.setBypass (track->pluginList, 0, false);
            expect (! manager.isBypassed (track->pluginList, 0));
        }

        beginTest ("Remove effect");
        {
            EffectsChainManager manager (*edit);
            int before = manager.getNumUserPlugins (track->pluginList);

            manager.removePlugin (track->pluginList, 0);
            expectEquals (manager.getNumUserPlugins (track->pluginList), before - 1);
        }

        beginTest ("Effect names");
        {
            expect (EffectsChainManager::getEffectName (EffectsChainManager::EffectType::reverb) == "Reverb");
            expect (EffectsChainManager::getEffectName (EffectsChainManager::EffectType::compressor) == "Compressor");
            expect (EffectsChainManager::getEffectName (EffectsChainManager::EffectType::equaliser) == "EQ");
        }

        beginTest ("Add all effect types without crashing");
        {
            EffectsChainManager manager (*edit);
            auto* track2 = tracks[1];

            manager.addEffect (*track2, EffectsChainManager::EffectType::equaliser);
            manager.addEffect (*track2, EffectsChainManager::EffectType::compressor);
            manager.addEffect (*track2, EffectsChainManager::EffectType::chorus);
            manager.addEffect (*track2, EffectsChainManager::EffectType::phaser);
            manager.addEffect (*track2, EffectsChainManager::EffectType::pitchShift);

            expect (manager.getNumUserPlugins (track2->pluginList) >= 5);
        }

        editFile.deleteFile();
    }
};

static EffectsChainTest effectsChainTest;
