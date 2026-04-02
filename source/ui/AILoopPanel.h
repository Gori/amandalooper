#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../core/AILoopClient.h"
#include "AmlpLookAndFeel.h"

class AILoopPanel : public juce::Component
{
public:
    AILoopPanel();
    ~AILoopPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void (const AILoopParams&)> onGenerate;
    std::function<void ()> onClose;

    void setServerState (AILoopClient::ServerState state, float setupProgress);
    void setGenerationState (AILoopClient::GenState state, float progress, const juce::String& errorMsg);

    void prepareForGeneration (int masterBpm, int detectedBars, const juce::String& masterKey);

private:
    juce::Label titleLabel { {}, "AI Loop Generator" };
    juce::Label promptLabel { {}, "Prompt:" };
    juce::TextEditor promptEditor;
    
    juce::Label bpmLabel { {}, "BPM:" };
    juce::Label bpmValueLabel { {}, "120" };
    
    juce::Label barsLabel { {}, "Bars:" };
    juce::Label barsValueLabel { {}, "4" };
    
    juce::Label keyLabel { {}, "Key:" };
    juce::Label keyValueLabel { {}, "" };
    
    juce::Label stepsLabel { {}, "Steps:" };
    juce::Slider stepsSlider;
    
    juce::Label cfgLabel { {}, "CFG:" };
    juce::Slider cfgSlider;

    juce::Label seedLabel { {}, "Seed:" };
    juce::TextEditor seedEditor;

    juce::TextButton generateButton { "Generate" };
    juce::TextButton closeButton { "X" };

    juce::Label statusLabel;
    double progressValue = 0.0;
    juce::ProgressBar progressBar { progressValue };

    AILoopClient::ServerState currentServerState = AILoopClient::ServerState::notStarted;
    AILoopClient::GenState currentGenState = AILoopClient::GenState::idle;

    void updateUIState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AILoopPanel)
};
