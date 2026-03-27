#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

class AmlpEngine
{
public:
    AmlpEngine();
    ~AmlpEngine();

    te::Engine& getEngine()             { return *engine; }
    te::Edit& getEdit()                 { return *edit; }
    te::TransportControl& getTransport();

    void showAudioDeviceSettings (juce::Component& parent);
    void setupInputsForTracks();

private:
    std::unique_ptr<te::Engine> engine;
    std::unique_ptr<te::Edit> edit;

    void createDefaultEdit();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmlpEngine)
};
