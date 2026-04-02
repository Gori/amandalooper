#include "KeyDetector.h"
#include <keyfinder.h>

static juce::String keyToString (KeyFinder::key_t key)
{
    switch (key)
    {
        case KeyFinder::A_MAJOR:      return "A major";
        case KeyFinder::A_MINOR:      return "A minor";
        case KeyFinder::B_FLAT_MAJOR: return "Bb major";
        case KeyFinder::B_FLAT_MINOR: return "Bb minor";
        case KeyFinder::B_MAJOR:      return "B major";
        case KeyFinder::B_MINOR:      return "B minor";
        case KeyFinder::C_MAJOR:      return "C major";
        case KeyFinder::C_MINOR:      return "C minor";
        case KeyFinder::D_FLAT_MAJOR: return "Db major";
        case KeyFinder::D_FLAT_MINOR: return "Db minor";
        case KeyFinder::D_MAJOR:      return "D major";
        case KeyFinder::D_MINOR:      return "D minor";
        case KeyFinder::E_FLAT_MAJOR: return "Eb major";
        case KeyFinder::E_FLAT_MINOR: return "Eb minor";
        case KeyFinder::E_MAJOR:      return "E major";
        case KeyFinder::E_MINOR:      return "E minor";
        case KeyFinder::F_MAJOR:      return "F major";
        case KeyFinder::F_MINOR:      return "F minor";
        case KeyFinder::G_FLAT_MAJOR: return "Gb major";
        case KeyFinder::G_FLAT_MINOR: return "Gb minor";
        case KeyFinder::G_MAJOR:      return "G major";
        case KeyFinder::G_MINOR:      return "G minor";
        case KeyFinder::A_FLAT_MAJOR: return "Ab major";
        case KeyFinder::A_FLAT_MINOR: return "Ab minor";
        case KeyFinder::SILENCE:      return "";
        default:                      return "";
    }
}

juce::String KeyDetector::detectKey (const juce::File& audioFile)
{
    if (! audioFile.existsAsFile())
        return {};

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (audioFile));
    if (reader == nullptr)
        return {};

    auto numSamples  = (int) reader->lengthInSamples;
    auto numChannels = (int) reader->numChannels;
    auto sampleRate  = (unsigned int) reader->sampleRate;

    if (numSamples <= 0)
        return {};

    // Read audio into a JUCE buffer
    juce::AudioBuffer<float> buffer (numChannels, numSamples);
    reader->read (&buffer, 0, numSamples, 0, true, true);

    // Feed into libkeyfinder (expects interleaved mono or multi-channel)
    KeyFinder::AudioData audioData;
    audioData.setChannels (numChannels);
    audioData.setFrameRate (sampleRate);
    audioData.addToSampleCount (numSamples * numChannels);

    for (int i = 0; i < numSamples; ++i)
        for (int ch = 0; ch < numChannels; ++ch)
            audioData.setSample (i * numChannels + ch, buffer.getSample (ch, i));

    KeyFinder::KeyFinder kf;
    auto result = kf.keyOfAudio (audioData);

    return keyToString (result);
}
