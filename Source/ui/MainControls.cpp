#include "MainControls.h"
#include "CommonIncludes.h"
namespace starflux::ui
{
void MainControls::setupSlider(juce::Slider& s, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setName(name);
}

MainControls::MainControls()
{
    setupSlider(density, "Density");
    setupSlider(size, "Size");
    setupSlider(brightness, "Brightness");
    setupSlider(reactivity, "Reactivity");
    setupSlider(depth, "Depth");

    addAndMakeVisible(density);
    addAndMakeVisible(size);
    addAndMakeVisible(brightness);
    addAndMakeVisible(reactivity);
    addAndMakeVisible(depth);

    colorMode.addItemList({ "White", "Blue", "RGB", "Gradient Drift" }, 1);
    motionPreset.addItemList({ "Static", "Top-Down", "Left-Right", "Diagonal", "Outward", "Inward", "Swirl" }, 1);

    addAndMakeVisible(colorMode);
    addAndMakeVisible(motionPreset);
    addAndMakeVisible(toggles);
}

void MainControls::resized()
{
    auto r = getLocalBounds().reduced(6);
    auto top = r.removeFromTop(120);

    const int w = top.getWidth() / 5;
    density.setBounds(top.removeFromLeft(w));
    size.setBounds(top.removeFromLeft(w));
    brightness.setBounds(top.removeFromLeft(w));
    reactivity.setBounds(top.removeFromLeft(w));
    depth.setBounds(top);

    auto row2 = r.removeFromTop(28);
    colorMode.setBounds(row2.removeFromLeft(row2.getWidth() / 2).reduced(2));
    motionPreset.setBounds(row2.reduced(2));

    toggles.setBounds(r.removeFromTop(28));
}
}
