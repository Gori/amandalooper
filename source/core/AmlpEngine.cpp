#include "AmlpEngine.h"
#include "AmlpUIBehaviour.h"

AmlpEngine::AmlpEngine()
{
    engine = std::make_unique<te::Engine> (
        "AMLP",
        std::make_unique<AmlpUIBehaviour>(),
        nullptr);

    createDefaultEdit();
}

AmlpEngine::~AmlpEngine()
{
    edit->getTransport().stop (false, false);
    edit.reset();
    engine.reset();
}

te::TransportControl& AmlpEngine::getTransport()
{
    return edit->getTransport();
}

void AmlpEngine::showAudioDeviceSettings (juce::Component& /*parent*/)
{
    juce::DialogWindow::LaunchOptions options;
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.content.setOwned (new juce::AudioDeviceSelectorComponent (
        engine->getDeviceManager().deviceManager,
        0, 512, 0, 512,
        true, true, true, false));
    options.content->setSize (600, 500);
    options.launchAsync();
}

void AmlpEngine::createDefaultEdit()
{
    engine->getDeviceManager().initialise (2, 2);

    const auto editFile = juce::File::getSpecialLocation (
        juce::File::tempDirectory).getChildFile ("AMLP_default.tracktionedit");

    edit = te::createEmptyEdit (*engine, editFile);
    edit->playInStopEnabled = true;
    edit->ensureNumberOfAudioTracks (4);
    edit->getTransport().ensureContextAllocated();

    // Defer input setup until the message loop is running and TE's async
    // device initialization has completed (matching the RecordingDemo pattern
    // where setup happens in response to user interaction, not in constructor).
    juce::Timer::callAfterDelay (1000, [this]
    {
        setupInputsForTracks();
    });
}

void AmlpEngine::setupInputsForTracks()
{
    auto& dm = engine->getDeviceManager();

    for (int i = 0; i < dm.getNumWaveInDevices(); ++i)
    {
        if (auto* wip = dm.getWaveInDevice (i))
        {
            wip->setStereoPair (false);
            wip->setEnabled (true);
            wip->setMonitorMode (te::InputDevice::MonitorMode::off);
        }
    }

    // Reallocate context so it picks up the newly enabled wave inputs
    edit->getTransport().freePlaybackContext();
    edit->getTransport().ensureContextAllocated();

    auto audioTracks = te::getAudioTracks (*edit);
    int trackNum = 0;

    for (auto instance : edit->getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
        {
            if (trackNum < (int) audioTracks.size())
            {
                auto* track = audioTracks[(size_t) trackNum];
                instance->setTarget (track->itemID, true, &edit->getUndoManager(), 0);
                instance->setRecordingEnabled (track->itemID, true);
                ++trackNum;
            }
        }
    }

}
