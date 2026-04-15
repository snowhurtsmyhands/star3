#include "VisualTimeline.h"
#include "CommonIncludes.h"
namespace starflux::engine
{
void VisualTimeline::reset() noexcept
{
    internalBeats = 0.0;
    phase = 0.0f;
    wrappedBeat = 0.0f;
}

void VisualTimeline::advance(double dtSeconds, const PlayheadState& host, bool followHostLoop, bool visualLoopEnable,
                             double startBeats, double endBeats, bool smoothWrap, bool snapToBars) noexcept
{
    const auto bpm = host.hasTempo ? host.bpm : 120.0;
    internalBeats += (dtSeconds * bpm) / 60.0;

    auto beat = host.isPlaying && host.hasTempo ? host.ppqPosition : internalBeats;
    auto loopStart = startBeats;
    auto loopEnd = juce::jmax(startBeats + 0.25, endBeats);

    if (followHostLoop && host.hasLoop)
    {
        loopStart = host.loopStartPpq;
        loopEnd = juce::jmax(host.loopStartPpq + 0.25, host.loopEndPpq);
        beat = host.ppqPosition;
    }

    if (snapToBars)
    {
        loopStart = std::floor(loopStart / 4.0) * 4.0;
        loopEnd = std::ceil(loopEnd / 4.0) * 4.0;
    }

    wrappedBeat = (float) beat;

    if (visualLoopEnable || (followHostLoop && host.hasLoop))
    {
        const auto len = juce::jmax(0.25, loopEnd - loopStart);
        double rel = std::fmod(beat - loopStart, len);
        if (rel < 0.0)
            rel += len;

        const auto raw = (float) (rel / len);
        phase = smoothWrap ? raw * raw * (3.0f - 2.0f * raw) : raw;
        wrappedBeat = (float) (loopStart + rel);
    }
    else
    {
        phase = (float) std::fmod(beat * 0.125, 1.0);
    }
}
}
