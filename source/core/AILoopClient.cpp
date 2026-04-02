#include "AILoopClient.h"
#include <juce_events/juce_events.h>

AILoopClient::AILoopClient() : juce::Thread ("AILoopClient")
{
}

AILoopClient::~AILoopClient()
{
    stopServer();
}

void AILoopClient::ensureServerRunning()
{
    if (serverState.load() != ServerState::notStarted && serverState.load() != ServerState::error)
        return;

    auto amlpDir = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".amlp");
    auto readyFile = amlpDir.getChildFile ("ai-ready");

    if (! readyFile.existsAsFile())
    {
        serverState = ServerState::settingUp;
        setupProgress = 0.0f;
        
        auto setupScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory().getChildFile("Resources").getChildFile("setup_ai_env.py");
            
        if (!setupScript.existsAsFile())
            setupScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                .getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("setup_ai_env.py");
                
        if (!setupScript.existsAsFile())
            setupScript = juce::File::getCurrentWorkingDirectory().getChildFile("scripts").getChildFile("setup_ai_env.py");

        juce::StringArray args;
        args.add("python3");
        args.add(setupScript.getFullPathName());

        setupProcess.start (args);
        startThread(); // Start polling thread
    }
    else
    {
        serverState = ServerState::loading;
        
        auto venvDir = amlpDir.getChildFile ("ai-venv");
        auto pythonExe = venvDir.getChildFile ("bin").getChildFile ("python");
        
        auto serverScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory().getChildFile("Resources").getChildFile("ai_loop_server.py");
            
        if (!serverScript.existsAsFile())
            serverScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                .getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("ai_loop_server.py");

        if (!serverScript.existsAsFile())
            serverScript = juce::File::getCurrentWorkingDirectory().getChildFile("scripts").getChildFile("ai_loop_server.py");

        juce::StringArray args;
        args.add(pythonExe.getFullPathName());
        args.add(serverScript.getFullPathName());

        pythonProcess.start (args);
        startThread();
    }
}

void AILoopClient::stopServer()
{
    stopThread (2000);
    if (setupProcess.isRunning())
        setupProcess.kill();
    if (pythonProcess.isRunning())
        pythonProcess.kill();
        
    serverState = ServerState::notStarted;
    genState = GenState::idle;
}

void AILoopClient::startGeneration (const AILoopParams& params)
{
    if (serverState.load() != ServerState::ready || genState.load() == GenState::running)
        return;

    juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
    jsonObj->setProperty ("prompt", params.prompt);
    jsonObj->setProperty ("bpm", params.bpm);
    jsonObj->setProperty ("bars", params.bars);
    jsonObj->setProperty ("key", params.key);
    jsonObj->setProperty ("steps", params.steps);
    jsonObj->setProperty ("cfgScale", params.cfgScale);
    jsonObj->setProperty ("seed", params.seed);
    jsonObj->setProperty ("seconds_total", params.secondsTotal);

    juce::MemoryOutputStream m;

    juce::JSON::writeToStream (m, juce::var (jsonObj.get()));
    auto jsonStr = m.toString();

    genState = GenState::running;
    genProgress = 0.0f;
    errorMessage = "";

    auto response = sendPostRequest ("/generate", jsonStr);
    auto vResponse = juce::JSON::parse (response);
    if (vResponse.isObject())
    {
        if (auto* obj = vResponse.getDynamicObject())
        {
            if (obj->hasProperty ("job_id"))
            {
                currentJobId = obj->getProperty ("job_id").toString();
                return; // Thread will now handle polling
            }
        }
    }

    genState = GenState::error;
    errorMessage = "Failed to start generation";
}

void AILoopClient::cancelGeneration()
{
    genState = GenState::idle;
}

juce::File AILoopClient::getGeneratedFile() const
{
    return juce::File (generatedPath);
}

juce::String AILoopClient::getErrorMessage() const
{
    return errorMessage;
}

void AILoopClient::run()
{
    while (! threadShouldExit())
    {
        auto sState = serverState.load();
        
        if (sState == ServerState::settingUp)
        {
            checkSetupProcess();
        }
        else if (sState == ServerState::loading)
        {
            if (pollHealth())
                serverState = ServerState::ready;
        }
        else if (sState == ServerState::ready && genState.load() == GenState::running)
        {
            checkJobStatus();
        }

        wait (1000); // 1 second polling
    }
}

void AILoopClient::checkSetupProcess()
{
    char buffer[1024];
    // This will block if the process is running and hasn't output anything,
    // which is fine since we are in a background thread and setup takes time.
    int bytesRead = setupProcess.readProcessOutput (buffer, sizeof(buffer)-1);
    
    if (bytesRead > 0)
    {
        buffer[bytesRead] = 0;
        juce::String output (buffer);
        
        auto lines = juce::StringArray::fromLines(output);
        for (auto& line : lines)
        {
            if (line.startsWith("PROGRESS:"))
            {
                auto valStr = line.replace("PROGRESS:", "").upToFirstOccurrenceOf(":", false, false);
                setupProgress = valStr.getFloatValue();
            }
        }
    }

    if (! setupProcess.isRunning() && bytesRead == 0)
    {
        auto amlpDir = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".amlp");
        if (amlpDir.getChildFile ("ai-ready").existsAsFile())
        {
            serverState = ServerState::loading;
            
            // Launch server now
            auto pythonExe = amlpDir.getChildFile ("ai-venv").getChildFile ("bin").getChildFile ("python");
            auto serverScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                .getParentDirectory().getChildFile("Resources").getChildFile("ai_loop_server.py");
                
            if (!serverScript.existsAsFile())
                serverScript = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getChildFile("Resources").getChildFile("ai_loop_server.py");
    
            if (!serverScript.existsAsFile())
                serverScript = juce::File::getCurrentWorkingDirectory().getChildFile("scripts").getChildFile("ai_loop_server.py");
            
            juce::StringArray args;
            args.add(pythonExe.getFullPathName());
            args.add(serverScript.getFullPathName());
            pythonProcess.start (args);
        }
        else
        {
            serverState = ServerState::error;
        }
    }
}

bool AILoopClient::pollHealth()
{
    auto response = sendGetRequest ("/health");
    auto vResponse = juce::JSON::parse (response);
    if (vResponse.isObject())
    {
        auto* obj = vResponse.getDynamicObject();
        if (obj->getProperty("status").toString() == "ready")
            return true;
    }
    return false;
}

void AILoopClient::checkJobStatus()
{
    if (currentJobId.isEmpty()) return;

    auto response = sendGetRequest ("/status/" + currentJobId);
    auto vResponse = juce::JSON::parse (response);
    
    if (vResponse.isObject())
    {
        auto* obj = vResponse.getDynamicObject();
        auto st = obj->getProperty("state").toString();
        
        if (st == "running")
        {
            genProgress = static_cast<float> (obj->getProperty("progress"));
        }
        else if (st == "done")
        {
            genProgress = 1.0f;
            generatedPath = obj->getProperty("output_path").toString();
            genState = GenState::done;
            currentJobId = "";
            
            if (onComplete)
            {
                juce::File wavFile (generatedPath);
                juce::MessageManager::callAsync ([cb = this->onComplete, f = wavFile] { cb (f, true); });
            }
        }
        else if (st == "error")
        {
            errorMessage = obj->getProperty("error").toString();
            genState = GenState::error;
            currentJobId = "";
            if (onComplete)
            {
                juce::MessageManager::callAsync ([cb = this->onComplete] { cb (juce::File(), false); });
            }
        }
    }
}

juce::String AILoopClient::sendPostRequest (const juce::String& endpoint, const juce::String& jsonBody)
{
    juce::URL url ("http://127.0.0.1:8976" + endpoint);
    url = url.withPOSTData (jsonBody);
    
    int statusCode = 0;
    std::unique_ptr<juce::InputStream> stream (url.createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                               .withConnectionTimeoutMs (5000)
                                               .withExtraHeaders ("Content-Type: application/json\r\n")
                                               .withStatusCode (&statusCode)));
    if (stream != nullptr)
        return stream->readEntireStreamAsString();
        
    return "{}";
}

juce::String AILoopClient::sendGetRequest (const juce::String& endpoint)
{
    juce::URL url ("http://127.0.0.1:8976" + endpoint);
    
    int statusCode = 0;
    std::unique_ptr<juce::InputStream> stream (url.createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                               .withConnectionTimeoutMs (5000)
                                               .withStatusCode (&statusCode)));
    if (stream != nullptr)
        return stream->readEntireStreamAsString();
        
    return "{}";
}
