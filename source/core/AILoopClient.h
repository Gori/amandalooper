#pragma once

#include <juce_core/juce_core.h>

struct AILoopParams
{
    juce::String prompt;
    int bpm = 120;
    int bars = 4;
    juce::String key = "C major";
    int steps = 100;
    float cfgScale = 6.0f;
    int seed = -1;
    float secondsTotal = 8.0f;
};

class AILoopClient : private juce::Thread
{
public:
    enum class ServerState { notStarted, settingUp, loading, ready, error };
    enum class GenState { idle, running, done, error };

    AILoopClient();
    ~AILoopClient() override;

    // Server lifecycle
    void ensureServerRunning();
    void stopServer();
    ServerState getServerState() const { return serverState.load(); }
    float getSetupProgress() const { return setupProgress.load(); }

    // Generation lifecycle
    void startGeneration (const AILoopParams& params);
    void cancelGeneration();
    GenState getGenerationState() const { return genState.load(); }
    float getGenerationProgress() const { return genProgress.load(); }
    juce::File getGeneratedFile() const;
    juce::String getErrorMessage() const;

    std::function<void(const juce::File&, bool)> onComplete;

private:
    std::atomic<ServerState> serverState { ServerState::notStarted };
    std::atomic<float> setupProgress { 0.0f };

    std::atomic<GenState> genState { GenState::idle };
    std::atomic<float> genProgress { 0.0f };
    juce::String generatedPath;
    juce::String errorMessage;
    juce::String currentJobId;

    juce::ChildProcess pythonProcess;
    juce::ChildProcess setupProcess;

    void run() override;

    void checkSetupProcess();
    bool pollHealth();
    void checkJobStatus();

    juce::String sendPostRequest (const juce::String& endpoint, const juce::String& jsonBody);
    juce::String sendGetRequest (const juce::String& endpoint);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AILoopClient)
};
