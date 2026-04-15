#include "PluginEditor.h"
#include "engine/ParameterIDs.h"

StarFluxAudioProcessorEditor::StarFluxAudioProcessorEditor(StarFluxAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), renderer(p)
{
    setSize(1100, 700);
    setResizable(true, true);
    setResizeLimits(820, 520, 1600, 1000);

    juce::Path squareOutline;
    squareOutline.addRectangle(1.0f, 1.0f, 14.0f, 14.0f);
    controlsButton.setShape(squareOutline, false, false, false);
    controlsButton.setTriggeredOnMouseDown(false);
    controlsButton.setOutline(juce::Colour(0xff79b9ff).withAlpha(0.72f), 1.2f);
    controlsButton.setOnColours(juce::Colours::transparentBlack, juce::Colour(0xff8fc8ff).withAlpha(0.20f),
                                juce::Colour(0xff9ad0ff).withAlpha(0.34f));
    controlsButton.onClick = [this]
    {
        controlsOpen = !controlsOpen;
        buttonAlpha = 1.0f;
        idleTimer = 0.0f;
    };
    addAndMakeVisible(controlsButton);

    // Title label — shown below button when controls are open
    title.setColour(juce::Label::textColourId, juce::Colour(0xffa0b8d8));
    title.setFont(juce::Font("Helvetica Neue", 11.5f, juce::Font::bold));
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);

    // Preset combo — shown below title when open
    presetBox.addItemList({ "Static", "Drift", "Forward", "Float" }, 1);
    presetBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff0a1018));
    presetBox.setColour(juce::ComboBox::textColourId,       juce::Colour(0xffa0b8d8));
    presetBox.setColour(juce::ComboBox::outlineColourId,    juce::Colour(0x553a84c6));
    addAndMakeVisible(presetBox);

    addAndMakeVisible(controlsPanel);

    openGL.setRenderer(&renderer);
    openGL.attachTo(*this);
    openGL.setComponentPaintingEnabled(true);
    openGL.setContinuousRepainting(true);

    juce::Random rng(0x5f5f91);
    fallbackStars.reserve(1400);
    for (int i = 0; i < 1400; ++i)
    {
        FallbackStar s;
        s.pos = { rng.nextFloat(), rng.nextFloat() };
        s.vel = { (rng.nextFloat() - 0.5f) * 0.013f, (rng.nextFloat() - 0.5f) * 0.013f };
        s.phase = rng.nextFloat() * juce::MathConstants<float>::twoPi;
        s.size = 0.35f + rng.nextFloat() * 1.3f;
        s.brightness = 0.5f + rng.nextFloat() * 0.7f;
        fallbackStars.push_back(s);
    }

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

    auto* densityP = processor.apvts.getRawParameterValue(starflux::params::density);
    auto* sizeP = processor.apvts.getRawParameterValue(starflux::params::size);
    auto* brightP = processor.apvts.getRawParameterValue(starflux::params::brightness);
    auto* twinkleP = processor.apvts.getRawParameterValue(starflux::params::twinkleOn);
    auto* twinkleAmtP = processor.apvts.getRawParameterValue(starflux::params::twinkleAmount);

    const float density = densityP ? densityP->load(std::memory_order_relaxed) : 0.62f;
    const float size    = sizeP ? sizeP->load(std::memory_order_relaxed) : 0.95f;
    const float bright  = brightP ? brightP->load(std::memory_order_relaxed) : 1.25f;
    const bool twinkleOn = twinkleP ? twinkleP->load(std::memory_order_relaxed) > 0.5f : true;
    const float twinkleAmount = twinkleAmtP ? twinkleAmtP->load(std::memory_order_relaxed) : 0.35f;

    const int visibleCount = juce::jlimit(280, (int) fallbackStars.size(),
                                          (int) std::round((float) fallbackStars.size() * juce::jlimit(0.2f, 1.0f, density)));
    const float baseRadius = juce::jmap(size, 0.2f, 1.8f, 0.8f, 2.4f);
    const float alphaScale = juce::jlimit(0.25f, 1.3f, bright * 0.72f);

    for (int i = 0; i < visibleCount; ++i)
    {
        const auto& s = fallbackStars[(size_t) i];
        const float cx = s.pos.x * (float) getWidth();
        const float cy = s.pos.y * (float) getHeight();
        const float tw = twinkleOn ? (1.0f + (0.15f + twinkleAmount * 0.25f) * std::sin(fallbackTime * 1.9f + s.phase)) : 1.0f;
        const float radius = baseRadius * s.size * (0.55f + 0.45f * tw);
        const float a = juce::jlimit(0.06f, 0.95f, s.brightness * alphaScale * (0.65f + 0.35f * tw));

        g.setColour(juce::Colour(0xff8fc5ff).withAlpha(a * 0.75f));
        g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

        if (a > 0.45f)
        {
            const float core = radius * 0.45f;
            g.setColour(juce::Colours::white.withAlpha(a * 0.65f));
            g.fillEllipse(cx - core, cy - core, core * 2.0f, core * 2.0f);
        }
    }
}

void StarFluxAudioProcessorEditor::resized()
{
    // ── Button: compact outlined square, top-right corner ──
    const int btnSize = 24;
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

    // Fade only applies to the closed-state access button.
    controlsButton.setAlpha(buttonAlpha);
    controlsPanel.setAlpha(1.0f);
    title.setAlpha(1.0f);
    presetBox.setAlpha(1.0f);
}

void StarFluxAudioProcessorEditor::mouseMove(const juce::MouseEvent&)
{
    buttonAlpha = 1.0f;
    idleTimer = 0.0f;
}

void StarFluxAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    lastDragPos = e.position;
    buttonAlpha = 1.0f;
    idleTimer  = 0.0f;
}

void StarFluxAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    buttonAlpha = 1.0f;
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
    constexpr float fadeDelay = 2.3f;
    constexpr float fadeRate  = 0.018f;
    constexpr float minButtonAlpha = 0.22f;

    // Animate drawer
    drawerAnim += ((controlsOpen ? 1.0f : 0.0f) - drawerAnim) * 0.18f;

    // Idle timer — only fade the small top-right access button while panel is closed.
    idleTimer += dt;
    if (controlsOpen)
    {
        buttonAlpha = 1.0f;
    }
    else if (idleTimer > fadeDelay)
    {
        buttonAlpha = juce::jmax(minButtonAlpha, buttonAlpha - fadeRate);
    }

    fallbackTime += dt;
    auto* speedP = processor.apvts.getRawParameterValue(starflux::params::motionSpeed);
    const float speed = speedP ? speedP->load(std::memory_order_relaxed) : 0.08f;
    const float drift = juce::jmap(speed, 0.0f, 1.5f, 0.25f, 3.5f) * dt;
    for (auto& s : fallbackStars)
    {
        s.pos += s.vel * drift;
        if (s.pos.x < 0.0f) s.pos.x += 1.0f;
        if (s.pos.x > 1.0f) s.pos.x -= 1.0f;
        if (s.pos.y < 0.0f) s.pos.y += 1.0f;
        if (s.pos.y > 1.0f) s.pos.y -= 1.0f;
    }

    resized();
    repaint();
}
