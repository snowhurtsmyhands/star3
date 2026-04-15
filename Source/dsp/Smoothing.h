#pragma once
#include "CommonIncludes.h"
namespace starflux::dsp
{
struct SmoothValue
{
    void setAlpha(float a) { alpha = a; }
    float process(float in)
    {
        value += alpha * (in - value);
        return value;
    }

    float value = 0.0f;
    float alpha = 0.08f;
};
}
