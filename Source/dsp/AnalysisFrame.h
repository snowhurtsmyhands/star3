#include "CommonIncludes.h"
#pragma once

namespace starflux::dsp
{
struct AnalysisFrame
{
    float rms = 0.0f;
    float peak = 0.0f;
    float low = 0.0f;
    float mid = 0.0f;
    float high = 0.0f;
    float transient = 0.0f;
};
}
