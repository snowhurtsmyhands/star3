#include "AudioAnalysis.h"
#include "CommonIncludes.h"
namespace starflux::dsp
{
void AudioAnalysis::prepare(double sampleRate, int maximumBlockSize)
{
    juce::ignoreUnused(maximumBlockSize);
    sr = sampleRate;

    auto low = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 200.0f);
    auto high = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 2500.0f);

    *lowL.coefficients = *low;
    *lowR.coefficients = *low;
    *highL.coefficients = *high;
    *highR.coefficients = *high;

    rmsS.setAlpha(0.12f);
    lowS.setAlpha(0.10f);
    midS.setAlpha(0.10f);
    highS.setAlpha(0.10f);
}

AnalysisFrame AudioAnalysis::processBlock(const juce::AudioBuffer<float>& buffer) noexcept
{
    AnalysisFrame out;
    const auto n = buffer.getNumSamples();
    if (n <= 0)
        return out;

    double sumSq = 0.0;
    float peak = 0.0f;
    double lowSum = 0.0, highSum = 0.0, fullSum = 0.0;

    for (int i = 0; i < n; ++i)
    {
        const auto l = buffer.getSample(0, i);
        const auto r = buffer.getNumChannels() > 1 ? buffer.getSample(1, i) : l;
        const auto m = 0.5f * (l + r);

        sumSq += m * m;
        peak = juce::jmax(peak, std::abs(m));

        auto lo = 0.5f * (lowL.processSample(l) + lowR.processSample(r));
        auto hi = 0.5f * (highL.processSample(l) + highR.processSample(r));

        lowSum += std::abs(lo);
        highSum += std::abs(hi);
        fullSum += std::abs(m);
    }

    const auto rms = std::sqrt(sumSq / (double) n);
    const float lowE = (float) (lowSum / n);
    const float highE = (float) (highSum / n);
    const float fullE = (float) (fullSum / n);
    const float midE = juce::jmax(0.0f, fullE - (0.6f * lowE + 0.7f * highE));

    out.rms = rmsS.process((float) rms);
    out.peak = peak;
    out.low = lowS.process(lowE);
    out.mid = midS.process(midE);
    out.high = highS.process(highE);

    const auto delta = out.rms - prevRms;
    out.transient = juce::jlimit(0.0f, 1.0f, juce::jmap(delta, 0.0f, 0.2f, 0.0f, 1.0f));
    prevRms = out.rms;

    juce::ignoreUnused(sr);
    return out;
}
}
