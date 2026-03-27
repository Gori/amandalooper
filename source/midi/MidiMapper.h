#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <functional>
#include <map>
#include <optional>

/**
    Maps MIDI CC/Note messages to looper actions.

    Supports MIDI learn mode: call startLearn(), send a MIDI message,
    and the mapping is automatically created.

    Thread safety: all methods must be called on the message thread.
*/
class MidiMapper
{
public:
    enum class Action
    {
        recordToggle,
        play,
        stop,
        overdub,
        undoOverdub,
        redoOverdub,
        nextScene,
        prevScene,
        launchScene,
        muteTrack,
        soloTrack,
        tapTempo,
        doubleLength,
        halveLength,
        reverse,
        stopAll
    };

    /** Callback invoked when a mapped action is triggered. */
    using ActionCallback = std::function<void (Action action, int trackIndex, int sceneIndex)>;

    MidiMapper();

    /** Set the callback that receives dispatched actions. */
    void setActionCallback (ActionCallback callback);

    /** Process an incoming MIDI message. Returns true if it matched a mapping. */
    bool processMidiMessage (const juce::MidiMessage& message);

    // ---- Mapping Management ----

    struct MidiBinding
    {
        int channel = 0;    // 1-16, or 0 for any channel
        int ccOrNote = 0;   // CC number or note number
        bool isCC = true;   // true = CC, false = Note On
    };

    struct Mapping
    {
        MidiBinding binding;
        Action action;
        int trackIndex = -1;   // -1 = global (not track-specific)
        int sceneIndex = -1;   // -1 = not scene-specific
    };

    /** Add a mapping. */
    void addMapping (const Mapping& mapping);

    /** Remove all mappings for a specific action + track/scene combo. */
    void removeMapping (Action action, int trackIndex = -1, int sceneIndex = -1);

    /** Remove all mappings. */
    void clearAllMappings();

    /** Get all current mappings. */
    const std::vector<Mapping>& getMappings() const { return mappings; }

    // ---- MIDI Learn ----

    /** Start learning: the next MIDI message will be mapped to this action. */
    void startLearn (Action action, int trackIndex = -1, int sceneIndex = -1);

    /** Cancel learn mode. */
    void stopLearn();

    /** Is learn mode active? */
    bool isLearning() const { return learning; }

    // ---- Persistence ----

    /** Save mappings to a ValueTree. */
    juce::ValueTree saveToValueTree() const;

    /** Load mappings from a ValueTree. */
    void loadFromValueTree (const juce::ValueTree& state);

    // ---- Utilities ----

    /** Get a human-readable name for an action. */
    static juce::String getActionName (Action action);

private:
    std::vector<Mapping> mappings;
    ActionCallback actionCallback;

    bool learning = false;
    Action learnAction = Action::recordToggle;
    int learnTrackIndex = -1;
    int learnSceneIndex = -1;

    std::optional<Mapping> findMapping (const juce::MidiMessage& message) const;
    static MidiBinding bindingFromMessage (const juce::MidiMessage& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMapper)
};
