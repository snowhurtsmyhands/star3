#include "PluginEditor.h"
#include "engine/ParameterIDs.h"

// ──────────────────────────────────────────────────────────────────────────────
// StarFlux Editor — Premium minimal UI
//
// Changes from original:
//  • Single small square "⊞" button (top-right), fades when idle
//  • Name + preset selector appear UNDER the controls button (collapsed when closed)
//  • NO persistent UI on screen — everything fades after idle timeout
//  • Controls panel slides in from the right
//  • Drag viewport to rotate (only when panel is not under cursor)
//  • Value numbers: compact, bold, 3 significant figures max
// ──────────────────────────────────────────────────────────────────────────────

StarFluxAudioProcessorEditor::StarFluxAudioProcessorEditor(StarFluxAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), renderer(p)
{
    setSize(1100, 700);
    setResizable(true, true);
    setResizeLimits(820, 520, 1600, 1000);

    // ── Controls toggle button: small square icon ──
    controlsButton.setButtonText(juce::String::fromUTF8("\xe2\x8a\x9e")); // ⊞ grid icon
    controlsButton.setColour(juce::TextButton::buttonColourId,    juce::Colour(0xff0d1520));
    controlsButton.setColour(juce::TextButton::buttonOnColourId,  juce::Colour(0xff1a2840));
    controlsButton.setColour(juce::TextButton::textColourOffId,   juce::Colour(0xffa0b8d8));
    controlsButton.setColour(juce::TextButton::textColourOnId,    juce::Colour(0xffc8dff5));
    controlsButton.onClick = [this]
    {
        controlsOpen = !controlsOpen;
        uiAlpha = 1.0f; // show UI when toggling
        idleTimer = 0.0f;
    };
    addAndMakeVisible(controlsButton);

    // Title label — shown below button when controls are open
    title.setColour(juce::Label::textColourId, juce::Colour(0xffa0b8d8));
    title.setFont(juce::Font("Helvetica Neue", 11.5f, juce::Font::bold));
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);

    // Preset combo — shown below title when open
    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0a1018));
    presetBox.setColour(juce::ComboBox::textColourId,       juce::Colour(0xffa0b8d8));
    presetBox.setColour(juce::ComboBox::outlineColourId,    juce::Colour(0x553a84c6));
    addAndMakeVisible(presetBox);

    addAndMakeVisible(controlsPanel);

    openGL.setRenderer(&renderer);
    openGL.attachTo(*this);
    openGL.setContinuousRepainting(true);

    using namespace starflux::params;
    auto& s = processor.apvts;
    auto sa=[this,&s](juce::Slider& sl,const char* id){ sliderAttachments.push_back(std::make_unique<SliderAttachment>(s,id,sl)); };
    auto ba=[this,&s](juce::Button& b, const char* id){ buttonAttachments.push_back(std::make_unique<ButtonAttachment>(s,id,b)); };
    auto ca=[this,&s](juce::ComboBox& c,const char* id){ comboAttachments.push_back(std::make_unique<ComboAttachment>(s,id,c)); };

    ca(presetBox, motionPreset);

    sa(controlsPanel.density,      density);
    sa(controlsPanel.size,         size);
    sa(controlsPanel.speed,        motionSpeed);
    sa(controlsPanel.brightness,   brightness);
    sa(controlsPanel.depth,        depth);
    ba(controlsPanel.twinkleOn,    twinkleOn);
    sa(controlsPanel.twinkleAmount,twinkleAmount);
    sa(controlsPanel.twinkleSpeed, twinkleSpeed);

    ca(controlsPanel.motionLane.source,   motionLaneSource);
    sa(controlsPanel.motionLane.amount,   motionLaneAmount);
    sa(controlsPanel.motionLane.freqMin,  motionLaneFreqMin);
    sa(controlsPanel.motionLane.freqMax,  motionLaneFreqMax);
    sa(controlsPanel.motionLane.attack,   motionLaneAttack);
    sa(controlsPanel.motionLane.release,  motionLaneRelease);

    ca(controlsPanel.sizeLane.source,     sizeLaneSource);
    sa(controlsPanel.sizeLane.amount,     sizeLaneAmount);
    sa(controlsPanel.sizeLane.freqMin,    sizeLaneFreqMin);
    sa(controlsPanel.sizeLane.freqMax,    sizeLaneFreqMax);
    sa(controlsPanel.sizeLane.attack,     sizeLaneAttack);
    sa(controlsPanel.sizeLane.release,    sizeLaneRelease);

    ca(controlsPanel.brightnessLane.source,   brightnessLaneSource);
    sa(controlsPanel.brightnessLane.amount,   brightnessLaneAmount);
    sa(controlsPanel.brightnessLane.freqMin,  brightnessLaneFreqMin);
    sa(controlsPanel.brightnessLane.freqMax,  brightnessLaneFreqMax);
    sa(controlsPanel.brightnessLane.attack,   brightnessLaneAttack);
    sa(controlsPanel.brightnessLane.release,  brightnessLaneRelease);

    ca(controlsPanel.twinkleLane.source,  twinkleLaneSource);
    sa(controlsPanel.twinkleLane.amount,  twinkleLaneAmount);
    sa(controlsPanel.twinkleLane.freqMin, twinkleLaneFreqMin);
    sa(controlsPanel.twinkleLane.freqMax, twinkleLaneFreqMax);
    sa(controlsPanel.twinkleLane.attack,  twinkleLaneAttack);
    sa(controlsPanel.twinkleLane.release, twinkleLaneRelease);

    sa(controlsPanel.midiAttack,   midiAttack);
    sa(controlsPanel.midiDecay,    midiDecay);
    sa(controlsPanel.midiSustain,  midiSustain);
    sa(controlsPanel.midiRelease,  midiRelease);

    startTimerHz(60);
}

StarFluxAudioProcessorEditor::~StarFluxAudioProcessorEditor() { openGL.detach(); }

void StarFluxAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(3, 5, 10));
}

void StarFluxAudioProcessorEditor::resized()
{
    // ── Button: 36×36 square, top-right corner ──
    const int btnSize = 36;
    const int margin  = 10;
    controlsButton.setBounds(getWidth() - btnSize - margin, margin, btnSize, btnSize);

    // ── Title + preset: below button when controls are open ──
    const bool showHeaderInfo = (drawerAnim > 0.01f);
    const int drawerWidth = juce::jlimit(280, 430, (int)(getWidth() * 0.34f));
    const int panelX = getWidth() - (int)(drawerWidth * drawerAnim) - margin;

    title.setBounds(panelX, btnSize + margin + 6,  drawerWidth, 16);
    presetBox.setBounds(panelX, btnSize + margin + 26, drawerWidth, 24);

    title.setVisible(showHeaderInfo);
    presetBox.setVisible(showHeaderInfo);

    // ── Sliding panel ──
    const int panelTop = btnSize + margin + 58;
    auto panel = juce::Rectangle<int>(panelX, panelTop,
                                      (int)(drawerWidth * drawerAnim),
                                      getHeight() - panelTop - margin);
    controlsPanel.setBounds(panel);
    controlsPanel.setVisible(drawerAnim > 0.01f);

    // Apply fade alpha
    controlsButton.setAlpha(uiAlpha);
    controlsPanel.setAlpha(uiAlpha);
    title.setAlpha(uiAlpha);
    presetBox.setAlpha(uiAlpha);
}

void StarFluxAudioProcessorEditor::mouseMove(const juce::MouseEvent&)
{
    uiAlpha  = 1.0f;
    idleTimer = 0.0f;
}

void StarFluxAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    lastDragPos = e.position;
    uiAlpha    = 1.0f;
    idleTimer  = 0.0f;
}

void StarFluxAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    uiAlpha   = 1.0f;
    idleTimer = 0.0f;
    auto d = e.position - lastDragPos;
    lastDragPos = e.position;

    // Only rotate view when not dragging inside the controls panel
    if (controlsPanel.isVisible() &&
        controlsPanel.getBounds().contains((int) e.position.x, (int) e.position.y))
        return;

    renderer.adjustView(d.x * 0.004f, d.y * 0.004f);
}

void StarFluxAudioProcessorEditor::timerCallback()
{
    constexpr float dt       = 1.0f / 60.0f;
    constexpr float fadeDelay = 3.0f;  // seconds before fade starts
    constexpr float fadeRate  = 0.008f;

    // Animate drawer
    drawerAnim += ((controlsOpen ? 1.0f : 0.0f) - drawerAnim) * 0.18f;

    // Idle timer — fade UI after inactivity
    idleTimer += dt;
    if (idleTimer > fadeDelay)
        uiAlpha = juce::jmax(0.0f, uiAlpha - fadeRate);

    resized();
}
