#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiMapper.h"

/**
    Manages MIDI device input and bridges messages to the MidiMapper
    via a lock-free FIFO (MIDI thread -> message thread).
*/
class MidiInputHandler : private juce::MidiInputCallback,
                         private juce::Timer
{
public:
    explicit MidiInputHandler (MidiMapper& mapper);
    ~MidiInputHandler() override;

    /** Get available MIDI input devices. */
    static juce::StringArray getAvailableDevices();

    /** Enable a MIDI input device by name. */
    void enableDevice (const juce::String& deviceIdentifier);

    /** Disable a MIDI input device. */
    void disableDevice (const juce::String& deviceIdentifier);

    /** Disable all MIDI input devices. */
    void disableAllDevices();

    /** Get currently enabled device identifiers. */
    juce::StringArray getEnabledDevices() const;

private:
    MidiMapper& mapper;

    // Lock-free FIFO for MIDI thread -> message thread
    static constexpr int fifoSize = 256;
    juce::AbstractFifo fifo { fifoSize };
    juce::MidiMessage fifoBuffer[fifoSize];

    // Active MIDI inputs
    std::vector<std::unique_ptr<juce::MidiInput>> activeInputs;

    // MidiInputCallback (called on MIDI thread)
    void handleIncomingMidiMessage (juce::MidiInput* source,
                                    const juce::MidiMessage& message) override;

    // Timer callback (message thread, 60Hz) — reads FIFO and dispatches
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputHandler)
};
