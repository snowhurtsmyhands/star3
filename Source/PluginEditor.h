#pragma once
#include "CommonIncludes.h"
#include "PluginProcessor.h"
#include "render/StarRenderer.h"
#include "ui/AdvancedPanel.h"

class StarFluxAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit StarFluxAudioProcessorEditor(StarFluxAudioProcessor&);
    ~StarFluxAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    void timerCallback() override;

    StarFluxAudioProcessor& processor;
    juce::OpenGLContext openGL;
    starflux::render::StarRenderer renderer;

    juce::Label title { {}, "StarFlux" };
    juce::ComboBox presetBox;
    juce::ShapeButton controlsButton { "controlsButton",
                                       juce::Colours::transparentBlack,
                                       juce::Colours::transparentBlack,
                                       juce::Colours::transparentBlack };
    starflux::ui::AdvancedPanel controlsPanel;

    bool controlsOpen = false;
    float drawerAnim  = 0.0f;
    float buttonAlpha = 1.0f;
    float idleTimer   = 0.0f;
    juce::Point<float> lastDragPos;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;
};
