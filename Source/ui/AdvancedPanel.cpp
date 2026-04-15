#include "AdvancedPanel.h"

namespace starflux::ui
{
void AdvancedPanel::setupSlider(juce::Slider& s, const juce::String& name, int decimals, bool percent)
{
    s.setName(name);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    // Compact text box: bold, right-aligned, narrower
    s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 46, 16);
    s.setNumDecimalPlacesToDisplay(decimals);

    // Compact formatter: 3 sig-fig max, strip trailing zeros, bold via font set on slider
    s.textFromValueFunction = [decimals, percent](double v) -> juce::String
    {
        if (percent)
            return juce::String((int) std::round(v * 100.0)) + "%";

        // Auto-select precision: large values as integers, small values fewer decimals
        if (std::abs(v) >= 1000.0)
            return juce::String((int) std::round(v));
        if (std::abs(v) >= 100.0)
            return juce::String(v, 1);
        if (std::abs(v) >= 10.0)
            return juce::String(v, 2);
        return juce::String(v, juce::jmin(decimals, 3));
    };

    // Make text box font bold
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffc0d4ee));
    s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff080c12));
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x22ffffff));
}

void AdvancedPanel::setupLane(LaneCard& lane, juce::Component& parent, const juce::String&)
{
    lane.source.addItemList({ "Off", "Audio Env", "Transient", "MIDI Gate", "MIDI Vel" }, 1);
    parent.addAndMakeVisible(lane.source);
    setupSlider(lane.amount, "Amount", 2);
    setupSlider(lane.freqMin, "Freq Min", 0);
    setupSlider(lane.freqMax, "Freq Max", 0);
    setupSlider(lane.attack, "Attack", 3);
    setupSlider(lane.release, "Release", 3);
    for (auto* s : { &lane.amount, &lane.freqMin, &lane.freqMax, &lane.attack, &lane.release })
        parent.addAndMakeVisible(*s);
}

AdvancedPanel::AdvancedPanel()
{
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(&content, false);
    viewport.setScrollBarsShown(true, false);

    for (auto* s : { &density, &size, &speed, &brightness, &depth, &twinkleAmount, &twinkleSpeed, &midiAttack, &midiDecay, &midiSustain, &midiRelease })
    {
        setupSlider(*s, "");
        content.addAndMakeVisible(*s);
    }

    density.setName("Density");
    size.setName("Size");
    speed.setName("Speed");
    brightness.setName("Brightness");
    depth.setName("Depth");
    twinkleAmount.setName("Twinkle Amt");
    twinkleSpeed.setName("Twinkle Speed");
    midiAttack.setName("MIDI Attack"); midiDecay.setName("MIDI Decay"); midiSustain.setName("MIDI Sustain"); midiRelease.setName("MIDI Release");

    content.addAndMakeVisible(twinkleOn);

    setupLane(motionLane, content, "Motion React");
    setupLane(sizeLane, content, "Size React");
    setupLane(brightnessLane, content, "Brightness React");
    setupLane(twinkleLane, content, "Twinkle React");
}

void AdvancedPanel::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.58f));
    g.fillRoundedRectangle(r, 12.0f);
    g.setColour(juce::Colour(0xff3a84c6).withAlpha(0.25f));
    g.drawRoundedRectangle(r.reduced(0.5f), 12.0f, 1.0f);
}

int AdvancedPanel::addTitle(int y, const juce::String& title, int w)
{
    auto l = std::make_unique<juce::Label>();
    l->setText(title, juce::dontSendNotification);
    l->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.95f));
    l->setBounds(10, y, w - 20, 20);
    content.addAndMakeVisible(*l);
    labels.push_back(std::move(l));
    return y + 22;
}

int AdvancedPanel::addSliderRow(int y, juce::Slider& s, int w)
{
    auto l = std::make_unique<juce::Label>();
    l->setText(s.getName(), juce::dontSendNotification);
    l->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.78f));
    l->setBounds(10, y, w - 20, 14);
    content.addAndMakeVisible(*l);
    labels.push_back(std::move(l));
    s.setBounds(10, y + 14, w - 20, 28);
    return y + 44;
}

int AdvancedPanel::addLaneCard(int y, LaneCard& lane, const juce::String& title, int w)
{
    y = addTitle(y, title, w);
    lane.source.setBounds(10, y, w - 20, 24); y += 28;
    for (auto* s : { &lane.amount, &lane.freqMin, &lane.freqMax, &lane.attack, &lane.release }) y = addSliderRow(y, *s, w);
    return y + 4;
}

void AdvancedPanel::layoutContent(int w)
{
    for (auto& l : labels) content.removeChildComponent(l.get());
    labels.clear();

    int y = 10;
    y = addTitle(y, "SPACE", w);
    for (auto* s : { &density, &size, &speed, &brightness, &depth }) y = addSliderRow(y, *s, w);
    twinkleOn.setBounds(10, y, w - 20, 22); y += 24;
    for (auto* s : { &twinkleAmount, &twinkleSpeed }) y = addSliderRow(y, *s, w);

    y = addLaneCard(y + 8, motionLane, "Motion React", w);
    y = addLaneCard(y + 6, sizeLane, "Size React", w);
    y = addLaneCard(y + 6, brightnessLane, "Brightness React", w);
    y = addLaneCard(y + 6, twinkleLane, "Twinkle React", w);

    y = addTitle(y + 8, "MIDI", w);
    for (auto* s : { &midiAttack, &midiDecay, &midiSustain, &midiRelease }) y = addSliderRow(y, *s, w);

    content.setSize(w, y + 12);
}

void AdvancedPanel::resized()
{
    viewport.setBounds(getLocalBounds().reduced(8));
    layoutContent(juce::jmax(240, viewport.getWidth() - 10));
}
}
