#pragma once

#include "CommonIncludes.h"

namespace starflux::ui
{
class ToggleButtonRow : public juce::Component
{
public:
    ToggleButtonRow();
    void resized() override;

    juce::ToggleButton constellation { "Constellation" };
    juce::ToggleButton voidMode { "Void" };
    juce::ToggleButton crt { "CRT" };
    juce::ToggleButton twinkle { "Twinkle" };
    juce::ToggleButton wave { "Wave" };
};
}
