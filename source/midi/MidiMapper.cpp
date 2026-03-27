#include "MidiMapper.h"

MidiMapper::MidiMapper() = default;

void MidiMapper::setActionCallback (ActionCallback callback)
{
    actionCallback = std::move (callback);
}

bool MidiMapper::processMidiMessage (const juce::MidiMessage& message)
{
    // Only process CC and Note On messages
    if (! message.isController() && ! message.isNoteOn())
        return false;

    // MIDI Learn mode
    if (learning)
    {
        Mapping newMapping;
        newMapping.binding = bindingFromMessage (message);
        newMapping.action = learnAction;
        newMapping.trackIndex = learnTrackIndex;
        newMapping.sceneIndex = learnSceneIndex;

        // Remove any existing mapping for this binding
        std::erase_if (mappings, [&] (const Mapping& m)
        {
            return m.binding.channel == newMapping.binding.channel
                && m.binding.ccOrNote == newMapping.binding.ccOrNote
                && m.binding.isCC == newMapping.binding.isCC;
        });

        mappings.push_back (newMapping);
        learning = false;
        return true;
    }

    // Normal dispatch
    if (auto mapping = findMapping (message))
    {
        if (actionCallback)
            actionCallback (mapping->action, mapping->trackIndex, mapping->sceneIndex);

        return true;
    }

    return false;
}

//==============================================================================
// Mapping Management
//==============================================================================

void MidiMapper::addMapping (const Mapping& mapping)
{
    mappings.push_back (mapping);
}

void MidiMapper::removeMapping (Action action, int trackIndex, int sceneIndex)
{
    std::erase_if (mappings, [&] (const Mapping& m)
    {
        return m.action == action
            && m.trackIndex == trackIndex
            && m.sceneIndex == sceneIndex;
    });
}

void MidiMapper::clearAllMappings()
{
    mappings.clear();
}

//==============================================================================
// MIDI Learn
//==============================================================================

void MidiMapper::startLearn (Action action, int trackIndex, int sceneIndex)
{
    learning = true;
    learnAction = action;
    learnTrackIndex = trackIndex;
    learnSceneIndex = sceneIndex;
}

void MidiMapper::stopLearn()
{
    learning = false;
}

//==============================================================================
// Persistence
//==============================================================================

juce::ValueTree MidiMapper::saveToValueTree() const
{
    juce::ValueTree state ("MidiMappings");

    for (auto& m : mappings)
    {
        juce::ValueTree child ("Mapping");
        child.setProperty ("channel", m.binding.channel, nullptr);
        child.setProperty ("ccOrNote", m.binding.ccOrNote, nullptr);
        child.setProperty ("isCC", m.binding.isCC, nullptr);
        child.setProperty ("action", static_cast<int> (m.action), nullptr);
        child.setProperty ("trackIndex", m.trackIndex, nullptr);
        child.setProperty ("sceneIndex", m.sceneIndex, nullptr);
        state.appendChild (child, nullptr);
    }

    return state;
}

void MidiMapper::loadFromValueTree (const juce::ValueTree& state)
{
    mappings.clear();

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto child = state.getChild (i);
        if (child.hasType ("Mapping"))
        {
            Mapping m;
            m.binding.channel = child.getProperty ("channel");
            m.binding.ccOrNote = child.getProperty ("ccOrNote");
            m.binding.isCC = child.getProperty ("isCC");
            m.action = static_cast<Action> (static_cast<int> (child.getProperty ("action")));
            m.trackIndex = child.getProperty ("trackIndex");
            m.sceneIndex = child.getProperty ("sceneIndex");
            mappings.push_back (m);
        }
    }
}

//==============================================================================
// Utilities
//==============================================================================

juce::String MidiMapper::getActionName (Action action)
{
    switch (action)
    {
        case Action::recordToggle:  return "Record Toggle";
        case Action::play:          return "Play";
        case Action::stop:          return "Stop";
        case Action::overdub:       return "Overdub";
        case Action::undoOverdub:   return "Undo Overdub";
        case Action::redoOverdub:   return "Redo Overdub";
        case Action::nextScene:     return "Next Scene";
        case Action::prevScene:     return "Previous Scene";
        case Action::launchScene:   return "Launch Scene";
        case Action::muteTrack:     return "Mute Track";
        case Action::soloTrack:     return "Solo Track";
        case Action::tapTempo:      return "Tap Tempo";
        case Action::doubleLength:  return "Double Length";
        case Action::halveLength:   return "Halve Length";
        case Action::reverse:       return "Reverse";
        case Action::stopAll:       return "Stop All";
    }
    return "Unknown";
}

//==============================================================================
// Private
//==============================================================================

std::optional<MidiMapper::Mapping> MidiMapper::findMapping (const juce::MidiMessage& message) const
{
    auto binding = bindingFromMessage (message);

    for (auto& m : mappings)
    {
        bool channelMatch = (m.binding.channel == 0) || (m.binding.channel == binding.channel);
        bool typeMatch = (m.binding.isCC == binding.isCC);
        bool numberMatch = (m.binding.ccOrNote == binding.ccOrNote);

        if (channelMatch && typeMatch && numberMatch)
            return m;
    }

    return std::nullopt;
}

MidiMapper::MidiBinding MidiMapper::bindingFromMessage (const juce::MidiMessage& message)
{
    MidiBinding binding;
    binding.channel = message.getChannel();

    if (message.isController())
    {
        binding.isCC = true;
        binding.ccOrNote = message.getControllerNumber();
    }
    else if (message.isNoteOn())
    {
        binding.isCC = false;
        binding.ccOrNote = message.getNoteNumber();
    }

    return binding;
}
