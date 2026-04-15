#include "StarRenderer.h"
#include "../PluginProcessor.h"
#include "../engine/ParameterIDs.h"
#include "CommonIncludes.h"
using namespace starflux;

namespace
{
float getParam(juce::AudioProcessorValueTreeState& apvts, const char* id, float fallback = 0.0f)
{
    if (auto* p = apvts.getRawParameterValue(id))
        return p->load(std::memory_order_relaxed);
    return fallback;
}

float bandEnergy(const dsp::AnalysisFrame& a, float minHz, float maxHz)
{
    const float lowW  = (minHz < 250.0f  ? 1.0f : 0.15f) * (maxHz > 40.0f ? 1.0f : 0.0f);
    const float midW  = (minHz < 4000.0f && maxHz > 200.0f) ? 1.0f : 0.15f;
    const float highW = (maxHz > 2000.0f ? 1.0f : 0.15f);
    const float w     = lowW + midW + highW;
    return (a.low * lowW + a.mid * midW + a.high * highW) / juce::jmax(0.001f, w);
}

float smoothTowards(float current, float target, float attack, float release, float dt)
{
    const float alpha = (target > current)
        ? (1.0f - std::exp(-dt / juce::jmax(0.001f, attack)))
        : (1.0f - std::exp(-dt / juce::jmax(0.001f, release)));
    return current + (target - current) * alpha;
}
}

namespace starflux::render
{
StarRenderer::StarRenderer(StarFluxAudioProcessor& p) : processor(p) {}

void StarRenderer::newOpenGLContextCreated()
{
    // Build once on context creation — NEVER rebuild per-frame (was the stars-invisible bug)
    lastSeed = (int) getParam(processor.apvts, params::seed, 777.0f);
    starField.rebuild(lastSeed, 2500);
    glitch.prepare(60.0);
}

void StarRenderer::renderOpenGL()
{
    if (auto* ctx = juce::OpenGLContext::getCurrentContext())
    {
        if (auto* target = ctx->getTargetComponent())
        {
            const auto fbW = juce::roundToInt((float) ctx->getRenderingScale() * (float) target->getWidth());
            const auto fbH = juce::roundToInt((float) ctx->getRenderingScale() * (float) target->getHeight());
            juce::gl::glViewport(0, 0, juce::jmax(1, fbW), juce::jmax(1, fbH));
        }
    }

    juce::gl::glDisable(juce::gl::GL_DEPTH_TEST);
    juce::gl::glDisable(juce::gl::GL_CULL_FACE);
    juce::gl::glDisable(juce::gl::GL_SCISSOR_TEST);
    juce::gl::glMatrixMode(juce::gl::GL_PROJECTION);
    juce::gl::glLoadIdentity();
    juce::gl::glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    juce::gl::glMatrixMode(juce::gl::GL_MODELVIEW);
    juce::gl::glLoadIdentity();

    juce::OpenGLHelpers::clear(juce::Colour::fromRGB(3, 5, 10));
    juce::gl::glEnable(juce::gl::GL_BLEND);
    juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);

    constexpr float dt = 1.0f / 60.0f;
    time += dt;
    auto& apvts = processor.apvts;
    auto analysis = processor.getLatestAnalysis();

    // Only rebuild if seed parameter actually changed
    const int newSeed = (int) getParam(apvts, params::seed, 777.0f);
    if (newSeed != lastSeed)
    {
        lastSeed = newSeed;
        starField.rebuild(lastSeed, 2500);
    }

    // MIDI envelope (ADSR)
    const float midiA = getParam(apvts, params::midiAttack,  0.03f);
    const float midiD = getParam(apvts, params::midiDecay,   0.15f);
    const float midiS = getParam(apvts, params::midiSustain, 0.8f);
    const float midiR = getParam(apvts, params::midiRelease, 0.3f);

    const float gate = processor.getMidiGate();
    const float vel  = processor.getMidiVelocity();
    if (gate > 0.5f)
        midiEnv = smoothTowards(midiEnv, vel, midiA, midiD, dt);
    else
        midiEnv = smoothTowards(midiEnv, 0.0f, midiA, midiR, dt);
    juce::ignoreUnused(midiS);

    // Lane modulation helper
    auto laneValue = [&](const char* amountId, const char* sourceId,
                         const char* minId, const char* maxId,
                         const char* attackId, const char* releaseId,
                         float& state) -> float
    {
        const float amount  = getParam(apvts, amountId);
        const int   source  = (int) getParam(apvts, sourceId);
        const float minHz   = getParam(apvts, minId,    20.0f);
        const float maxHz   = getParam(apvts, maxId,  8000.0f);
        const float attack  = getParam(apvts, attackId,  0.04f);
        const float release = getParam(apvts, releaseId, 0.20f);

        float raw = 0.0f;
        if      (source == 1) raw = bandEnergy(analysis, minHz, maxHz);
        else if (source == 2) raw = midiEnv;

        state = smoothTowards(state, juce::jlimit(0.0f, 1.0f, raw), attack, release, dt);
        return state * amount;
    };

    const float motionMod     = laneValue(params::motionLaneAmount,    params::motionLaneSource,
                                           params::motionLaneFreqMin,   params::motionLaneFreqMax,
                                           params::motionLaneAttack,    params::motionLaneRelease,    motionState);
    const float sizeMod       = laneValue(params::sizeLaneAmount,      params::sizeLaneSource,
                                           params::sizeLaneFreqMin,     params::sizeLaneFreqMax,
                                           params::sizeLaneAttack,      params::sizeLaneRelease,      sizeState);
    const float brightnessMod = laneValue(params::brightnessLaneAmount,params::brightnessLaneSource,
                                           params::brightnessLaneFreqMin,params::brightnessLaneFreqMax,
                                           params::brightnessLaneAttack,params::brightnessLaneRelease, brightState);
    const float twinkleMod    = laneValue(params::twinkleLaneAmount,   params::twinkleLaneSource,
                                           params::twinkleLaneFreqMin,  params::twinkleLaneFreqMax,
                                           params::twinkleLaneAttack,   params::twinkleLaneRelease,   twinkleState);

    const float baseSpeed    = getParam(apvts, params::motionSpeed,    0.05f);
    const int   motionPreset = (int) getParam(apvts, params::motionPreset);

    starField.update(dt, time, analysis,
                     motionPreset,
                     baseSpeed,
                     getParam(apvts, params::density,       0.45f),
                     getParam(apvts, params::size,          0.7f),
                     getParam(apvts, params::brightness,    1.0f),
                     getParam(apvts, params::depth,         1.0f),
                     motionMod,
                     sizeMod,
                     brightnessMod,
                     twinkleMod,
                     getParam(apvts, params::twinkleOn)      > 0.5f,
                     getParam(apvts, params::twinkleAmount,  0.35f),
                     getParam(apvts, params::twinkleSpeed,   1.0f));

    // View rotation
    const float yaw   = viewYaw.load(std::memory_order_relaxed);
    const float pitch = viewPitch.load(std::memory_order_relaxed);
    const float cy = std::cos(yaw),   sy = std::sin(yaw);
    const float cp = std::cos(pitch), sp = std::sin(pitch);

    // Global audio energy boost — makes stars pulse with the music
    const float audioEnergy    = juce::jlimit(0.0f, 1.0f, analysis.rms * 5.0f);
    const float audioBrightMul = 1.0f + audioEnergy * 0.65f;

    int starsDrawn = 0;
    for (const auto& s : starField.getRenderStars())
    {
        if (s.brightness <= 0.0005f)
            continue;

        juce::Vector3D<float> rp = s.pos;
        const float rx = rp.x * cy - rp.z * sy;
        const float rz = rp.x * sy + rp.z * cy;
        const float ry = rp.y * cp - rz * sp;
        rp = { rx, ry, rz * cp + rp.y * sp };

        const float depthScale = 1.0f / juce::jlimit(0.35f, 3.2f, 1.0f + rp.z * 0.72f);
        const float px = rp.x * depthScale;
        const float py = rp.y * depthScale;

        // Cull stars outside visible frustum
        if (std::abs(px) > 1.35f || std::abs(py) > 1.35f)
            continue;

        const float rad      = juce::jlimit(0.0015f, 0.030f, 0.0032f + s.size * depthScale * 0.011f);
        const float vignette = juce::jlimit(0.42f, 1.0f, 1.0f - 0.20f * (px * px + py * py));
        const float brt      = juce::jlimit(0.0f, 1.0f, s.brightness * vignette * audioBrightMul);

        // Cool blue-white tint
        const float r = brt * 0.80f;
        const float g = brt * 0.87f;
        const float b = brt * 1.05f;

        // Outer glow disc
        juce::gl::glBegin(juce::gl::GL_TRIANGLE_FAN);
        juce::gl::glColor4f(r, g, b, juce::jmax(0.30f, brt));
        juce::gl::glVertex2f(px, py);
        constexpr int segs = 10;
        for (int i = 0; i <= segs; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * (float) i / (float) segs;
            juce::gl::glVertex2f(px + std::cos(a) * rad,
                                  py + std::sin(a) * rad);
        }
        juce::gl::glEnd();
        ++starsDrawn;

        // Bright white core for larger / bright stars — premium sparkle
        if (brt > 0.45f)
        {
            const float cr = rad * 0.3f;
            juce::gl::glBegin(juce::gl::GL_TRIANGLE_FAN);
            juce::gl::glColor4f(1.0f, 1.0f, 1.0f, juce::jmax(0.35f, brt * 0.75f));
            juce::gl::glVertex2f(px, py);
            for (int i = 0; i <= segs; ++i)
            {
                const float a = juce::MathConstants<float>::twoPi * (float) i / (float) segs;
                juce::gl::glVertex2f(px + std::cos(a) * cr,
                                      py + std::sin(a) * cr);
            }
            juce::gl::glEnd();
        }
    }

    // Safety fallback: always draw at least a tiny center glint if everything was culled.
    outputHasVisibleStars.store(starsDrawn > 0, std::memory_order_relaxed);

    if (starsDrawn == 0)
    {
        juce::gl::glBegin(juce::gl::GL_TRIANGLE_FAN);
        juce::gl::glColor4f(0.75f, 0.88f, 1.0f, 0.95f);
        juce::gl::glVertex2f(0.0f, 0.0f);
        constexpr int segs = 12;
        constexpr float rad = 0.015f;
        for (int i = 0; i <= segs; ++i)
        {
            const float a = juce::MathConstants<float>::twoPi * (float) i / (float) segs;
            juce::gl::glVertex2f(std::cos(a) * rad, std::sin(a) * rad);
        }
        juce::gl::glEnd();
    }
}

void StarRenderer::openGLContextClosing() {}

void StarRenderer::adjustView(float yawDelta, float pitchDelta) noexcept
{
    auto y = viewYaw.load(std::memory_order_relaxed)   + yawDelta;
    auto p = viewPitch.load(std::memory_order_relaxed) + pitchDelta;
    viewYaw.store(y, std::memory_order_relaxed);
    viewPitch.store(juce::jlimit(-1.4f, 1.4f, p), std::memory_order_relaxed);
}
}
