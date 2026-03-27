#include "InputRouter.h"

InputRouter::InputRouter (te::Edit& e)
    : edit (e)
{
}

juce::StringArray InputRouter::getAvailableInputs() const
{
    juce::StringArray result;
    auto& dm = edit.engine.getDeviceManager();

    for (int i = 0; i < dm.getNumWaveInDevices(); ++i)
        if (auto* device = dm.getWaveInDevice (i))
            result.add (device->getName());

    return result;
}

void InputRouter::assignInputToTrack (int inputIndex, te::AudioTrack& track)
{
    auto& dm = edit.engine.getDeviceManager();

    if (inputIndex < 0 || inputIndex >= dm.getNumWaveInDevices())
        return;

    auto* device = dm.getWaveInDevice (inputIndex);
    if (device == nullptr)
        return;

    device->setEnabled (true);
    device->setMonitorMode (te::InputDevice::MonitorMode::automatic);

    edit.getTransport().ensureContextAllocated();

    for (auto instance : edit.getAllInputDevices())
    {
        if (&instance->getInputDevice() == device)
        {
            instance->setTarget (track.itemID, true, &edit.getUndoManager(), 0);
            break;
        }
    }

    edit.restartPlayback();
}

void InputRouter::unassignInput (te::AudioTrack& track)
{
    for (auto instance : edit.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
        {
            if (te::isOnTargetTrack (*instance, track, 0))
            {
                [[ maybe_unused ]] auto result = instance->removeTarget (track.itemID, &edit.getUndoManager());
                break;
            }
        }
    }
}

juce::String InputRouter::getAssignedInputName (te::AudioTrack& track) const
{
    for (auto instance : edit.getAllInputDevices())
    {
        if (instance->getInputDevice().getDeviceType() == te::InputDevice::waveDevice)
        {
            if (te::isOnTargetTrack (*instance, track, 0))
                return instance->getInputDevice().getName();
        }
    }
    return {};
}

void InputRouter::setMonitoring (te::AudioTrack& track, te::InputDevice::MonitorMode mode)
{
    for (auto instance : edit.getAllInputDevices())
    {
        if (te::isOnTargetTrack (*instance, track, 0))
        {
            instance->getInputDevice().setMonitorMode (mode);
            break;
        }
    }
}

te::InputDevice::MonitorMode InputRouter::getMonitoring (te::AudioTrack& track) const
{
    for (auto instance : edit.getAllInputDevices())
    {
        if (te::isOnTargetTrack (*instance, track, 0))
            return instance->getInputDevice().getMonitorMode();
    }
    return te::InputDevice::MonitorMode::off;
}

void InputRouter::initialiseAudioDevice (int numInputChannels, int numOutputChannels)
{
    edit.engine.getDeviceManager().initialise (numInputChannels, numOutputChannels);
}
