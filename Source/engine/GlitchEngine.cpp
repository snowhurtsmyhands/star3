#include "GlitchEngine.h"
#include "CommonIncludes.h"
namespace starflux::engine
{
void GlitchEngine::prepare(double sampleRate)
{
    sr = sampleRate;
    rng.setSeedRandomly();
}

void GlitchEngine::update(float dt, float databendRandom, float frameSkipRandom)
{
    juce::ignoreUnused(sr);
    timer += dt;

    if (timer >= nextDatabend)
    {
        databendOffset = rng.nextFloat() * 2.0f - 1.0f;
        const auto jitter = juce::jmap(databendRandom, 0.0f, 1.0f, 0.16f, 0.02f);
        nextDatabend = timer + jitter * (0.4f + 0.6f * rng.nextFloat());
    }

    if (timer >= nextFrameEvent)
    {
        holdFrame = rng.nextFloat() < (0.1f + 0.6f * frameSkipRandom);
        const auto jitter = juce::jmap(frameSkipRandom, 0.0f, 1.0f, 0.20f, 0.03f);
        nextFrameEvent = timer + jitter * (0.3f + 0.7f * rng.nextFloat());
    }
}

bool GlitchEngine::shouldHoldFrame(float frameSkipAmount) noexcept
{
    if (frameSkipAmount <= 0.001f)
        return false;

    const float baseChance = juce::jmap(frameSkipAmount, 0.0f, 1.0f, 0.02f, 0.45f);
    if (!holdFrame)
        holdFrame = rng.nextFloat() < baseChance;
    else
        holdFrame = rng.nextFloat() < (baseChance * 0.55f);

    return holdFrame;
}
}
