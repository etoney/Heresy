#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"

namespace heresy
{
class PresetManager
{
public:
    static juce::File userFolder();          // ~/Library/Application Support/Castle Danger Music Company/Heresy/Presets
    static juce::StringArray userPresetNames();
    static juce::File fileFor (const juce::String& name);
};
} // namespace heresy

class HeresyProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    HeresyProcessor();
    ~HeresyProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Heresy"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // presets
    void loadFactoryPreset (int index);
    bool loadUserPreset (const juce::String& name);
    bool saveUserPreset (const juce::String& name);
    bool deleteUserPreset (const juce::String& name);
    juce::String getPresetName() const;
    bool isUserPreset() const { return presetIsUser.load(); }
    int presetVersion() const { return presetCounter.load(); }

    void panic() { panicRequested = true; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    static juce::String hostName (int index);

private:
    void timerCallback() override;
    void applyValues (const heresy::Values& v);

    heresy::Engine engine;
    std::array<std::atomic<float>*, heresy::NumParams> rawParams {};
    heresy::Values values {};

    mutable juce::CriticalSection presetLock;
    juce::String presetName { "Init" };
    std::atomic<bool> presetIsUser { false };
    std::atomic<int> presetCounter { 0 };
    std::atomic<int> factoryIndex { 0 };
    std::atomic<bool> panicRequested { false };

    // reverb impulse reload (debounced, message thread)
    float pendingSize = -1; bool pendingRev = false; int pendingTicks = 0;
    bool prepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeresyProcessor)
};
