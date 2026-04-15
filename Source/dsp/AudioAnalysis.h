#pragma once

#include "CommonIncludes.h"
#include "AnalysisFrame.h"
#include "Smoothing.h"

namespace starflux::dsp
{
class AudioAnalysis
{
public:
    void prepare(double sampleRate, int maximumBlockSize);
    AnalysisFrame processBlock(const juce::AudioBuffer<float>& buffer) noexcept;

private:
    juce::dsp::IIR::Filter<float> lowL, lowR, highL, highR;
    SmoothValue rmsS, lowS, midS, highS;
    float prevRms = 0.0f;
    double sr = 44100.0;
};
}
