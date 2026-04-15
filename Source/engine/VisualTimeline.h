#pragma once
#include "CommonIncludes.h"


namespace starflux::engine
{
struct PlayheadState
{
    bool hasTempo = false;
    bool isPlaying = false;
    double bpm = 120.0;
    double ppqPosition = 0.0;
    bool hasLoop = false;
    double loopStartPpq = 0.0;
    double loopEndPpq = 0.0;
};

class VisualTimeline
{
public:
    void reset() noexcept;
    void advance(double dtSeconds, const PlayheadState& host, bool followHostLoop, bool visualLoopEnable,
                 double startBeats, double endBeats, bool smoothWrap, bool snapToBars) noexcept;

    float getPhase() const noexcept { return phase; }
    float getWrappedBeat() const noexcept { return wrappedBeat; }

private:
    double internalBeats = 0.0;
    float phase = 0.0f;
    float wrappedBeat = 0.0f;
};
}
