#pragma once

#include "CommonIncludes.h"
#include "ToggleButtonRow.h"

namespace starflux::ui
{
class MainControls : public juce::Component
{
public:
    MainControls();
    void resized() override;

    juce::Slider density, size, brightness, reactivity, depth;
    juce::ComboBox colorMode, motionPreset;
    ToggleButtonRow toggles;

private:
    static void setupSlider(juce::Slider& s, const juce::String& name);
};
}
