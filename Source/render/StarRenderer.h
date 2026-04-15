#pragma once

#include "CommonIncludes.h"
#include "../engine/StarFieldEngine.h"
#include "../engine/VisualTimeline.h"
#include "../engine/GlitchEngine.h"

class StarFluxAudioProcessor;

namespace starflux::render
{
class StarRenderer : public juce::OpenGLRenderer
{
public:
    explicit StarRenderer(StarFluxAudioProcessor& p);
    ~StarRenderer() override = default;

    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;

    void adjustView(float yawDelta, float pitchDelta) noexcept;

private:
    StarFluxAudioProcessor& processor;
    engine::StarFieldEngine starField;
    engine::VisualTimeline timeline;
    engine::GlitchEngine glitch;
    std::atomic<float> viewYaw   { 0.0f };
    std::atomic<float> viewPitch { 0.0f };
    float time    = 0.0f;
    int   lastSeed = -1;  // track seed so we only rebuild on change

    // Per-render persistent smoothing state
    float midiEnv     = 0.0f;
    float motionState = 0.0f;
    float sizeState   = 0.0f;
    float brightState = 0.0f;
    float twinkleState= 0.0f;
};
}
