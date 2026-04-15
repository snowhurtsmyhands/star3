#pragma once
#include "CommonIncludes.h"
#include "StarState.h"
#include "MotionEngine.h"
#include "../dsp/AnalysisFrame.h"

namespace starflux::engine
{
class StarFieldEngine
{
public:
    void rebuild(int seed, int maxStars);
    void update(float dt, float timelinePhase, const dsp::AnalysisFrame& analysis,
                int motionPreset, float speed,
                float density, float size, float brightness, float depth,
                float motionMod, float sizeMod, float brightnessMod, float twinkleMod,
                bool twinkleOn, float twinkleAmount, float twinkleSpeed);

    const std::vector<RenderStar>& getRenderStars() const noexcept { return renderStars; }

private:
    static void wrapStar(Star& s, juce::Random& rng);
    std::vector<Star> stars;
    std::vector<RenderStar> renderStars;
    MotionEngine motion;
    juce::Random runtimeRng { 0x51a7f11 };
    int currentSeed = -1;
};
}
