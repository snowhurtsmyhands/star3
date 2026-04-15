#pragma once

#include "CommonIncludes.h"

namespace starflux::engine
{
class GlitchEngine
{
public:
    void prepare(double sampleRate);
    void update(float dt, float databendRandom, float frameSkipRandom);

    float getDatabendOffset() const noexcept { return databendOffset; }
    bool shouldHoldFrame(float frameSkipAmount) noexcept;

private:
    juce::Random rng;
    float timer = 0.0f;
    float nextDatabend = 0.12f;
    float nextFrameEvent = 0.09f;
    float databendOffset = 0.0f;
    bool holdFrame = false;
    double sr = 60.0;
};
}
