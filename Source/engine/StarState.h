#pragma once

#include "CommonIncludes.h"

namespace starflux::engine
{
struct Star
{
    juce::Vector3D<float> basePos;
    juce::Vector3D<float> pos;
    float brightness = 1.0f;
    float size = 1.0f;
    float phase = 0.0f;
};

struct RenderStar
{
    juce::Vector3D<float> pos;
    float brightness = 1.0f;
    float size = 1.0f;
};
}
