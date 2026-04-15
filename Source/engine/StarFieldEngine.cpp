#include "StarFieldEngine.h"
namespace starflux::engine
{
void StarFieldEngine::rebuild(int seed, int maxStars)
{
    if (seed == currentSeed && (int) stars.size() == maxStars) return;
    currentSeed = seed;
    juce::Random rng((juce::int64) seed * 7919 + 17);
    stars.assign((size_t) maxStars, {});
    renderStars.assign((size_t) maxStars, {});

    for (auto& s : stars)
    {
        s.basePos = { rng.nextFloat() * 2.4f - 1.2f, rng.nextFloat() * 2.4f - 1.2f, rng.nextFloat() * 2.4f - 1.2f };
        s.pos = s.basePos;
        s.brightness = 0.62f + rng.nextFloat() * 0.58f;
        s.size = 0.45f + rng.nextFloat() * 1.0f;
        s.phase = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    }
}

void StarFieldEngine::wrapStar(Star& s, juce::Random& rng)
{
    auto wrap = [](float& v){ if (v > 1.25f) v = -1.25f; else if (v < -1.25f) v = 1.25f; };
    wrap(s.pos.x); wrap(s.pos.y); wrap(s.pos.z);
    if (std::abs(s.pos.x) > 1.2f || std::abs(s.pos.y) > 1.2f)
        s.pos.z = rng.nextFloat() * 2.4f - 1.2f;
}

void StarFieldEngine::update(float dt, float timelinePhase, const dsp::AnalysisFrame& analysis,
                             int motionPreset, float speed,
                             float density, float size, float brightness, float depth,
                             float motionMod, float sizeMod, float brightnessMod, float twinkleMod,
                             bool twinkleOn, float twinkleAmount, float twinkleSpeed)
{
    const int minVisibleStars = juce::jmin(700, (int) stars.size());
    const int byDensity = (int) std::round(stars.size() * juce::jlimit(0.20f, 1.0f, density));
    const int activeCount = juce::jlimit(1, (int) stars.size(), juce::jmax(minVisibleStars, byDensity));
    const float audioLift = 1.0f + juce::jlimit(0.0f, 0.95f, analysis.rms * 2.2f + analysis.transient * 0.35f);

    for (int i = 0; i < activeCount; ++i)
    {
        auto& s = stars[(size_t) i];
        // Base speed drives idle drift; motionMod adds audio/MIDI reactivity on top
        // Speed is always applied if motion preset != 0, regardless of audio level
        const float sspd = juce::jlimit(0.0f, 2.0f, speed + motionMod * speed);
        auto v = motion.computePresetVelocity(motionPreset, s.pos, timelinePhase * juce::MathConstants<float>::twoPi) * sspd;
        s.pos += v * dt;

        if (motionPreset == 2 && s.pos.z < -1.25f)
            s.pos = { runtimeRng.nextFloat()*2.4f-1.2f, runtimeRng.nextFloat()*2.4f-1.2f, 1.25f };

        wrapStar(s, runtimeRng);

        const float near = juce::jlimit(0.0f, 1.0f, 1.0f - ((s.pos.z + 1.25f) / 2.5f));
        float b = s.brightness * brightness * (0.78f + near * 0.52f) * (1.0f + brightnessMod) * audioLift;
        if (twinkleOn)
            b *= 1.0f + (twinkleAmount + twinkleMod) * 0.35f * std::sin(s.phase + timelinePhase * twinkleSpeed * 6.0f + analysis.high * 8.0f);

        b = juce::jmax(0.12f, b);

        const float depthSafe = juce::jlimit(0.35f, 1.45f, depth);
        float sz = size * s.size * (0.62f + near) * (1.0f + sizeMod * 0.45f) * (1.0f + analysis.transient * 0.12f);
        renderStars[(size_t) i] = { { s.pos.x, s.pos.y, s.pos.z * depthSafe }, b, juce::jlimit(0.35f, 2.4f, sz) };
    }

    for (size_t i = (size_t) activeCount; i < renderStars.size(); ++i)
        renderStars[i] = { {0,0,-10}, 0, 0 };
}
}
