#pragma once
#include "CommonIncludes.h"
namespace starflux::engine
{
class MotionEngine
{
public:
    juce::Vector3D<float> computePresetVelocity(int preset, const juce::Vector3D<float>& pos, float time) const noexcept;
};
}
