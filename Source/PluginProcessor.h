#pragma once

#include "CommonIncludes.h"
#include "dsp/AudioAnalysis.h"
#include "engine/VisualTimeline.h"

struct StarFluxSnapshot
{
    std::atomic<float> rms { 0.0f }, peak { 0.0f }, low { 0.0f }, mid { 0.0f }, high { 0.0f }, transient { 0.0f };
    std::atomic<float> midiGate { 0.0f }, midiVelocity { 0.0f };
    std::atomic<double> ppq { 0.0 }, bpm { 120.0 }, loopStart { 0.0 }, loopEnd { 0.0 };
    std::atomic<int> flags { 0 }; // bit0 hasTempo bit1 isPlaying bit2 hasLoop
};

class StarFluxAudioProcessor final : public juce::AudioProcessor
{
public:
    StarFluxAudioProcessor();
    ~StarFluxAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    starflux::dsp::AnalysisFrame getLatestAnalysis() const noexcept;
    starflux::engine::PlayheadState getPlayheadState() const noexcept;
    float getMidiGate() const noexcept { return snapshot.midiGate.load(std::memory_order_relaxed); }
    float getMidiVelocity() const noexcept { return snapshot.midiVelocity.load(std::memory_order_relaxed); }

private:
    starflux::dsp::AudioAnalysis analysis;
    StarFluxSnapshot snapshot;
    int activeMidiNotes = 0;
    float lastMidiVelocity = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StarFluxAudioProcessor)
};
