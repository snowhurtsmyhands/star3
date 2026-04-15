#include "MotionEngine.h"
namespace starflux::engine
{
juce::Vector3D<float> MotionEngine::computePresetVelocity(int preset, const juce::Vector3D<float>& pos, float t) const noexcept
{
    switch (preset)
    {
        case 1: return { 0.22f, -0.12f, 0.0f }; // Drift
        case 2: return { 0.0f, 0.0f, -0.45f }; // Forward
        case 3: return { 0.12f * std::sin(t * 0.7f + pos.y * 2.0f), 0.08f * std::cos(t * 0.8f + pos.x * 2.0f), -0.05f }; // Float
        default: return { 0.0f, 0.0f, 0.0f }; // Static
    }
}
}
