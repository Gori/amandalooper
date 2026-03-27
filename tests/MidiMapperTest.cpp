#include <juce_audio_basics/juce_audio_basics.h>
#include "midi/MidiMapper.h"

class MidiMapperTest : public juce::UnitTest
{
public:
    MidiMapperTest() : UnitTest ("MidiMapper", "AMLP") {}

    void runTest() override
    {
        beginTest ("Add mapping and dispatch CC");
        {
            MidiMapper mapper;
            MidiMapper::Action receivedAction = MidiMapper::Action::stop;
            int receivedTrack = -1;

            mapper.setActionCallback ([&] (MidiMapper::Action action, int trackIdx, int)
            {
                receivedAction = action;
                receivedTrack = trackIdx;
            });

            MidiMapper::Mapping m;
            m.binding = { 1, 64, true }; // Channel 1, CC 64
            m.action = MidiMapper::Action::recordToggle;
            m.trackIndex = 0;
            mapper.addMapping (m);

            auto msg = juce::MidiMessage::controllerEvent (1, 64, 127);
            bool matched = mapper.processMidiMessage (msg);

            expect (matched);
            expect (receivedAction == MidiMapper::Action::recordToggle);
            expectEquals (receivedTrack, 0);
        }

        beginTest ("Dispatch Note On");
        {
            MidiMapper mapper;
            MidiMapper::Action received = MidiMapper::Action::stop;

            mapper.setActionCallback ([&] (MidiMapper::Action action, int, int)
            { received = action; });

            MidiMapper::Mapping m;
            m.binding = { 1, 60, false }; // Channel 1, Note 60
            m.action = MidiMapper::Action::overdub;
            mapper.addMapping (m);

            auto msg = juce::MidiMessage::noteOn (1, 60, 1.0f);
            expect (mapper.processMidiMessage (msg));
            expect (received == MidiMapper::Action::overdub);
        }

        beginTest ("Unmatched message returns false");
        {
            MidiMapper mapper;
            auto msg = juce::MidiMessage::controllerEvent (1, 99, 127);
            expect (! mapper.processMidiMessage (msg));
        }

        beginTest ("Any-channel matching (channel 0)");
        {
            MidiMapper mapper;
            bool triggered = false;
            mapper.setActionCallback ([&] (MidiMapper::Action, int, int) { triggered = true; });

            MidiMapper::Mapping m;
            m.binding = { 0, 64, true }; // Any channel, CC 64
            m.action = MidiMapper::Action::stopAll;
            mapper.addMapping (m);

            // Should match any channel
            expect (mapper.processMidiMessage (juce::MidiMessage::controllerEvent (1, 64, 127)));
            expect (triggered);

            triggered = false;
            expect (mapper.processMidiMessage (juce::MidiMessage::controllerEvent (5, 64, 127)));
            expect (triggered);
        }

        beginTest ("MIDI Learn mode");
        {
            MidiMapper mapper;

            mapper.startLearn (MidiMapper::Action::tapTempo);
            expect (mapper.isLearning());

            auto msg = juce::MidiMessage::controllerEvent (2, 80, 127);
            mapper.processMidiMessage (msg);

            expect (! mapper.isLearning());
            expectEquals (static_cast<int> (mapper.getMappings().size()), 1);

            auto& learned = mapper.getMappings()[0];
            expect (learned.action == MidiMapper::Action::tapTempo);
            expectEquals (learned.binding.channel, 2);
            expectEquals (learned.binding.ccOrNote, 80);
            expect (learned.binding.isCC);
        }

        beginTest ("Remove mapping");
        {
            MidiMapper mapper;
            MidiMapper::Mapping m;
            m.binding = { 1, 64, true };
            m.action = MidiMapper::Action::play;
            m.trackIndex = 0;
            mapper.addMapping (m);

            expectEquals (static_cast<int> (mapper.getMappings().size()), 1);
            mapper.removeMapping (MidiMapper::Action::play, 0);
            expect (mapper.getMappings().empty());
        }

        beginTest ("Save and load ValueTree");
        {
            MidiMapper mapper;

            MidiMapper::Mapping m1;
            m1.binding = { 1, 64, true };
            m1.action = MidiMapper::Action::recordToggle;
            m1.trackIndex = 0;
            mapper.addMapping (m1);

            MidiMapper::Mapping m2;
            m2.binding = { 2, 60, false };
            m2.action = MidiMapper::Action::overdub;
            m2.trackIndex = 1;
            mapper.addMapping (m2);

            auto state = mapper.saveToValueTree();

            MidiMapper mapper2;
            mapper2.loadFromValueTree (state);

            expectEquals (static_cast<int> (mapper2.getMappings().size()), 2);
            expect (mapper2.getMappings()[0].action == MidiMapper::Action::recordToggle);
            expect (mapper2.getMappings()[1].action == MidiMapper::Action::overdub);
        }

        beginTest ("Action names");
        {
            expect (MidiMapper::getActionName (MidiMapper::Action::recordToggle) == "Record Toggle");
            expect (MidiMapper::getActionName (MidiMapper::Action::stopAll) == "Stop All");
        }
    }
};

static MidiMapperTest midiMapperTest;
