#pragma once
#include "CommonIncludes.h"

namespace starflux::ui
{
struct LaneCard
{
    juce::Slider amount, freqMin, freqMax, attack, release;
    juce::ComboBox source;
};

class AdvancedPanel : public juce::Component
{
public:
    AdvancedPanel();
    void paint(juce::Graphics&) override;
    void resized() override;

    juce::Slider density, size, speed, brightness, depth;
    juce::Slider twinkleAmount, twinkleSpeed;
    juce::ToggleButton twinkleOn { "Twinkle" };

    LaneCard motionLane, sizeLane, brightnessLane, twinkleLane;
    juce::Slider midiAttack, midiDecay, midiSustain, midiRelease;

private:
    juce::Viewport viewport;
    juce::Component content;
    std::vector<std::unique_ptr<juce::Label>> labels;

    static void setupSlider(juce::Slider& s, const juce::String& name, int decimals = 2, bool percent = false);
    static void setupLane(LaneCard& lane, juce::Component& parent, const juce::String& title);
    int addTitle(int y, const juce::String& title, int w);
    int addSliderRow(int y, juce::Slider& s, int w);
    int addLaneCard(int y, LaneCard& lane, const juce::String& title, int w);
    void layoutContent(int w);
};
}
