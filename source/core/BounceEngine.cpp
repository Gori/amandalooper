#include "BounceEngine.h"

BounceEngine::BounceEngine (te::Edit& e)
    : edit (e)
{
}

//==============================================================================
// Bounce Files (mix multiple audio files)
//==============================================================================

void BounceEngine::bounceFiles (const std::vector<juce::File>& sourceFiles,
                                const juce::File& outputFile,
                                double sampleRate,
                                CompletionCallback onComplete)
{
    if (sourceFiles.empty() || bouncing.load())
    {
        if (onComplete)
            onComplete ({}, false);
        return;
    }

    bouncing = true;

    // Capture by value for thread safety
    auto files = sourceFiles;
    auto outFile = outputFile;
    auto sr = sampleRate;
    auto callback = std::move (onComplete);

    threadPool.addJob ([this, files, outFile, sr, callback]
    {
        mixAudioFiles (files, outFile, sr);
        bouncing = false;

        if (callback)
            juce::MessageManager::callAsync ([callback, outFile] { callback (outFile, outFile.existsAsFile()); });
    });
}

void BounceEngine::bounceTrack (te::AudioTrack& track,
                                tracktion::TimeRange timeRange,
                                const juce::File& outputFile,
                                bool includeEffects,
                                CompletionCallback onComplete)
{
    if (bouncing.load())
    {
        if (onComplete)
            onComplete ({}, false);
        return;
    }

    bouncing = true;

    auto outFile = outputFile;
    auto callback = std::move (onComplete);

    // Use TE's Renderer for track bouncing
    te::Renderer::Parameters params (edit);
    params.destFile = outFile;
    params.audioFormat = edit.engine.getAudioFileFormatManager()
                            .getWavFormat();
    params.time = timeRange;
    params.sampleRateForAudio = 44100.0;
    params.bitDepth = 24;
    params.usePlugins = includeEffects;
    params.useMasterPlugins = false;

    // Set track bitmask
    int trackIndex = 0;
    for (auto t : te::getAudioTracks (edit))
    {
        if (t == &track)
        {
            params.tracksToDo.setBit (trackIndex);
            break;
        }
        ++trackIndex;
    }

    threadPool.addJob ([this, params, callback, outFile]
    {
        auto resultFile = te::Renderer::renderToFile ("Bounce", params);
        bool success = resultFile.existsAsFile();
        bouncing = false;

        if (callback)
            juce::MessageManager::callAsync ([callback, outFile, success] { callback (outFile, success); });
    });
}

void BounceEngine::cancelAll()
{
    // ThreadPool doesn't support cancellation of running jobs,
    // but we can prevent new ones
    bouncing = false;
}

//==============================================================================
// Private: Mix audio files together
//==============================================================================

void BounceEngine::mixAudioFiles (const std::vector<juce::File>& sourceFiles,
                                   const juce::File& outputFile,
                                   double sampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Read all source files
    std::vector<std::unique_ptr<juce::AudioFormatReader>> readers;
    juce::int64 maxLength = 0;
    int maxChannels = 0;

    for (auto& file : sourceFiles)
    {
        auto reader = std::unique_ptr<juce::AudioFormatReader> (
            formatManager.createReaderFor (file));

        if (reader != nullptr)
        {
            maxLength = std::max (maxLength, reader->lengthInSamples);
            maxChannels = std::max (maxChannels, static_cast<int> (reader->numChannels));
            readers.push_back (std::move (reader));
        }
    }

    if (readers.empty() || maxLength == 0)
        return;

    if (maxChannels == 0)
        maxChannels = 2;

    // Create output writer using new API
    juce::WavAudioFormat wavFormat;
    auto outputStream = std::unique_ptr<juce::OutputStream> (outputFile.createOutputStream());
    if (outputStream == nullptr)
        return;

    auto options = juce::AudioFormatWriterOptions()
                       .withSampleRate (sampleRate)
                       .withNumChannels (maxChannels)
                       .withBitsPerSample (24);

    auto writer = wavFormat.createWriterFor (outputStream, options);

    if (writer == nullptr)
        return;

    // Mix in chunks
    const int blockSize = 4096;
    juce::AudioBuffer<float> mixBuffer (maxChannels, blockSize);
    juce::AudioBuffer<float> readBuffer (maxChannels, blockSize);

    juce::int64 samplesWritten = 0;

    while (samplesWritten < maxLength)
    {
        int samplesToProcess = static_cast<int> (
            std::min (static_cast<juce::int64> (blockSize), maxLength - samplesWritten));

        mixBuffer.clear();

        for (auto& reader : readers)
        {
            if (samplesWritten < reader->lengthInSamples)
            {
                readBuffer.clear();
                reader->read (&readBuffer, 0, samplesToProcess,
                              samplesWritten, true, true);

                for (int ch = 0; ch < maxChannels; ++ch)
                {
                    int srcCh = std::min (ch, static_cast<int> (reader->numChannels) - 1);
                    mixBuffer.addFrom (ch, 0, readBuffer, srcCh, 0, samplesToProcess);
                }
            }
        }

        writer->writeFromAudioSampleBuffer (mixBuffer, 0, samplesToProcess);
        samplesWritten += samplesToProcess;
    }
}
