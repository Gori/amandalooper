#include "MidiInputHandler.h"

MidiInputHandler::MidiInputHandler (MidiMapper& m)
    : mapper (m)
{
    startTimerHz (60);
}

MidiInputHandler::~MidiInputHandler()
{
    stopTimer();
    disableAllDevices();
}

juce::StringArray MidiInputHandler::getAvailableDevices()
{
    juce::StringArray result;
    for (auto& device : juce::MidiInput::getAvailableDevices())
        result.add (device.name);
    return result;
}

void MidiInputHandler::enableDevice (const juce::String& deviceIdentifier)
{
    // Check if already enabled
    for (auto& input : activeInputs)
        if (input->getIdentifier() == deviceIdentifier)
            return;

    auto devices = juce::MidiInput::getAvailableDevices();
    for (auto& device : devices)
    {
        if (device.identifier == deviceIdentifier || device.name == deviceIdentifier)
        {
            auto input = juce::MidiInput::openDevice (device.identifier, this);
            if (input != nullptr)
            {
                input->start();
                activeInputs.push_back (std::move (input));
            }
            return;
        }
    }
}

void MidiInputHandler::disableDevice (const juce::String& deviceIdentifier)
{
    std::erase_if (activeInputs, [&] (const std::unique_ptr<juce::MidiInput>& input)
    {
        return input->getIdentifier() == deviceIdentifier
            || input->getName() == deviceIdentifier;
    });
}

void MidiInputHandler::disableAllDevices()
{
    activeInputs.clear();
}

juce::StringArray MidiInputHandler::getEnabledDevices() const
{
    juce::StringArray result;
    for (auto& input : activeInputs)
        result.add (input->getName());
    return result;
}

//==============================================================================
// MIDI thread callback
//==============================================================================

void MidiInputHandler::handleIncomingMidiMessage (juce::MidiInput*,
                                                   const juce::MidiMessage& message)
{
    // Write to lock-free FIFO (MIDI thread)
    int start1, size1, start2, size2;
    fifo.prepareToWrite (1, start1, size1, start2, size2);

    if (size1 > 0)
        fifoBuffer[start1] = message;

    fifo.finishedWrite (size1 + size2);
}

//==============================================================================
// Message thread timer — reads FIFO and dispatches
//==============================================================================

void MidiInputHandler::timerCallback()
{
    int start1, size1, start2, size2;
    fifo.prepareToRead (fifo.getNumReady(), start1, size1, start2, size2);

    for (int i = 0; i < size1; ++i)
        mapper.processMidiMessage (fifoBuffer[start1 + i]);

    for (int i = 0; i < size2; ++i)
        mapper.processMidiMessage (fifoBuffer[start2 + i]);

    fifo.finishedRead (size1 + size2);
}
