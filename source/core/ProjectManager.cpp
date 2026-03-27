#include "ProjectManager.h"

ProjectManager::ProjectManager (te::Engine& e, te::Edit& ed)
    : engine (e), edit (ed)
{
    // Autosave every 30 seconds
    if (autosaveEnabled)
        startTimer (30000);
}

ProjectManager::~ProjectManager()
{
    stopTimer();
}

//==============================================================================
// Save / Load
//==============================================================================

bool ProjectManager::saveProject (const juce::File& destination)
{
    auto bundleDir = destination;

    // Ensure .amlp extension on directory name
    if (! bundleDir.getFileName().endsWith (".amlp"))
        bundleDir = bundleDir.withFileExtension ("amlp");

    bundleDir.createDirectory();

    // Save the Edit XML
    auto editFile = bundleDir.getChildFile ("project.tracktionedit");
    te::EditFileOperations ops (edit);

    if (! ops.writeToFile (editFile, false))
        return false;

    // Copy audio files
    copyAudioFilesToBundle (bundleDir);

    // Write metadata
    writeMetadata (bundleDir);

    currentProjectFile = bundleDir;
    addToRecentProjects (bundleDir);

    return true;
}

std::unique_ptr<te::Edit> ProjectManager::loadProject (const juce::File& source)
{
    auto editFile = source.getChildFile ("project.tracktionedit");

    if (! editFile.existsAsFile())
        return nullptr;

    auto loadedEdit = te::loadEditFromFile (engine, editFile);

    if (loadedEdit != nullptr)
    {
        currentProjectFile = source;
        addToRecentProjects (source);
    }

    return loadedEdit;
}

std::unique_ptr<te::Edit> ProjectManager::createNewProject (const juce::File& location)
{
    auto bundleDir = location;
    if (! bundleDir.getFileName().endsWith (".amlp"))
        bundleDir = bundleDir.withFileExtension ("amlp");

    bundleDir.createDirectory();

    auto editFile = bundleDir.getChildFile ("project.tracktionedit");
    auto newEdit = te::createEmptyEdit (engine, editFile);

    if (newEdit != nullptr)
    {
        newEdit->ensureNumberOfAudioTracks (4);
        newEdit->getSceneList().ensureNumberOfScenes (8);

        te::EditFileOperations ops (*newEdit);
        ops.writeToFile (editFile, false);

        writeMetadata (bundleDir);
        currentProjectFile = bundleDir;
    }

    return newEdit;
}

bool ProjectManager::isModified() const
{
    return edit.hasChangedSinceSaved();
}

//==============================================================================
// Autosave
//==============================================================================

void ProjectManager::setAutosaveEnabled (bool enabled)
{
    autosaveEnabled = enabled;

    if (enabled)
        startTimer (30000);
    else
        stopTimer();
}

void ProjectManager::timerCallback()
{
    if (currentProjectFile.exists() && isModified())
    {
        te::EditFileOperations ops (edit);
        ops.saveTempVersion (false);
    }
}

//==============================================================================
// Recent Projects
//==============================================================================

juce::StringArray ProjectManager::getRecentProjects() const
{
    auto propsFile = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                         .getChildFile ("AMLP")
                         .getChildFile ("recent_projects.txt");

    juce::StringArray recent;
    if (propsFile.existsAsFile())
        recent.addLines (propsFile.loadFileAsString());

    return recent;
}

void ProjectManager::addToRecentProjects (const juce::File& projectFile)
{
    auto propsDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                        .getChildFile ("AMLP");
    propsDir.createDirectory();

    auto propsFile = propsDir.getChildFile ("recent_projects.txt");

    auto recent = getRecentProjects();
    recent.removeString (projectFile.getFullPathName());
    recent.insert (0, projectFile.getFullPathName());

    // Keep max 20 recent projects
    while (recent.size() > 20)
        recent.remove (recent.size() - 1);

    propsFile.replaceWithText (recent.joinIntoString ("\n"));
}

//==============================================================================
// Private
//==============================================================================

void ProjectManager::copyAudioFilesToBundle (const juce::File& bundleDir)
{
    auto audioDir = bundleDir.getChildFile ("audio");
    audioDir.createDirectory();

    // Find all audio files referenced by clips in the edit
    for (auto track : te::getAudioTracks (edit))
    {
        for (auto clip : track->getClips())
        {
            if (auto* wac = dynamic_cast<te::WaveAudioClip*> (clip))
            {
                auto sourceFile = wac->getOriginalFile();
                if (sourceFile.existsAsFile())
                {
                    auto destFile = audioDir.getChildFile (sourceFile.getFileName());
                    if (! destFile.existsAsFile())
                        sourceFile.copyFileTo (destFile);
                }
            }
        }

        // Also check clip slots
        for (auto* slot : track->getClipSlotList().getClipSlots())
        {
            if (auto* wac = dynamic_cast<te::WaveAudioClip*> (slot->getClip()))
            {
                auto sourceFile = wac->getOriginalFile();
                if (sourceFile.existsAsFile())
                {
                    auto destFile = audioDir.getChildFile (sourceFile.getFileName());
                    if (! destFile.existsAsFile())
                        sourceFile.copyFileTo (destFile);
                }
            }
        }
    }
}

void ProjectManager::writeMetadata (const juce::File& bundleDir)
{
    auto metaFile = bundleDir.getChildFile ("metadata.json");

    auto trackCount = te::getAudioTracks (edit).size();
    auto bpm = edit.tempoSequence.getBpmAt (tracktion::TimePosition());
    auto sceneCount = edit.getSceneList().getNumScenes();

    juce::String json;
    json << "{\n";
    json << "  \"version\": \"0.1.0\",\n";
    json << "  \"bpm\": " << bpm << ",\n";
    json << "  \"trackCount\": " << static_cast<int> (trackCount) << ",\n";
    json << "  \"sceneCount\": " << sceneCount << ",\n";
    json << "  \"createdAt\": \"" << juce::Time::getCurrentTime().toISO8601 (true) << "\"\n";
    json << "}\n";

    metaFile.replaceWithText (json);
}
