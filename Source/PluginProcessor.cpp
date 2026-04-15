#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "engine/ParameterIDs.h"
#include "CommonIncludes.h"

StarFluxAudioProcessor::StarFluxAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{}

void StarFluxAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) { analysis.prepare(sampleRate, samplesPerBlock); }
void StarFluxAudioProcessor::releaseResources() {}

bool StarFluxAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void StarFluxAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn()) { ++activeMidiNotes; lastMidiVelocity = juce::jmax(lastMidiVelocity, msg.getFloatVelocity()); }
        else if (msg.isNoteOff() && activeMidiNotes > 0) { --activeMidiNotes; if (activeMidiNotes == 0) lastMidiVelocity = 0.0f; }
    }

    snapshot.midiGate.store(activeMidiNotes > 0 ? 1.0f : 0.0f, std::memory_order_relaxed);
    snapshot.midiVelocity.store(lastMidiVelocity, std::memory_order_relaxed);

    auto frame = analysis.processBlock(buffer);
    snapshot.rms.store(frame.rms, std::memory_order_relaxed);
    snapshot.peak.store(frame.peak, std::memory_order_relaxed);
    snapshot.low.store(frame.low, std::memory_order_relaxed);
    snapshot.mid.store(frame.mid, std::memory_order_relaxed);
    snapshot.high.store(frame.high, std::memory_order_relaxed);
    snapshot.transient.store(frame.transient, std::memory_order_relaxed);
}

void StarFluxAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StarFluxAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout StarFluxAudioProcessor::createParameterLayout()
{
    using namespace juce;
    using namespace starflux::params;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto f = [](auto id, auto name, float min, float max, float d){ return std::make_unique<AudioParameterFloat>(id, name, NormalisableRange<float>(min,max), d); };

    p.push_back(f(density, "Density", 0.05f, 1.0f, 0.62f));
    p.push_back(f(size, "Size", 0.2f, 1.8f, 0.95f));
    p.push_back(f(brightness, "Brightness", 0.1f, 2.0f, 1.25f));
    p.push_back(std::make_unique<AudioParameterChoice>(motionPreset, "Motion Preset", StringArray{ "Static", "Drift", "Forward", "Float" }, 1)); // default: Drift
    p.push_back(f(motionSpeed, "Speed", 0.0f, 1.5f, 0.08f)); // small non-zero default so stars move
    p.push_back(f(depth, "Depth", 0.2f, 2.0f, 0.8f));
    p.push_back(f(seed, "Seed", 0.0f, 9999.0f, 777.0f));

    p.push_back(std::make_unique<AudioParameterBool>(twinkleOn, "Twinkle", true));
    p.push_back(f(twinkleAmount, "Twinkle Amount", 0.0f, 1.0f, 0.35f));
    p.push_back(f(twinkleSpeed, "Twinkle Speed", 0.1f, 4.0f, 1.0f));

    auto lane = [&](auto amount, auto source, auto fmin, auto fmax, auto a, auto r, const String& name)
    {
        p.push_back(f(amount, name + " Amount", 0.0f, 2.0f, 0.0f));
        p.push_back(std::make_unique<AudioParameterChoice>(source, name + " Source", StringArray{ "Off", "Audio", "MIDI" }, 0));
        p.push_back(f(fmin, name + " Freq Min", 20.0f, 12000.0f, 20.0f));
        p.push_back(f(fmax, name + " Freq Max", 200.0f, 20000.0f, 6000.0f));
        p.push_back(f(a, name + " Attack", 0.001f, 1.0f, 0.035f));
        p.push_back(f(r, name + " Release", 0.005f, 2.0f, 0.25f));
    };

    lane(motionLaneAmount, motionLaneSource, motionLaneFreqMin, motionLaneFreqMax, motionLaneAttack, motionLaneRelease, "Motion React");
    lane(sizeLaneAmount, sizeLaneSource, sizeLaneFreqMin, sizeLaneFreqMax, sizeLaneAttack, sizeLaneRelease, "Size React");
    lane(brightnessLaneAmount, brightnessLaneSource, brightnessLaneFreqMin, brightnessLaneFreqMax, brightnessLaneAttack, brightnessLaneRelease, "Brightness React");
    lane(twinkleLaneAmount, twinkleLaneSource, twinkleLaneFreqMin, twinkleLaneFreqMax, twinkleLaneAttack, twinkleLaneRelease, "Twinkle React");

    p.push_back(f(midiAttack, "MIDI Attack", 0.001f, 1.0f, 0.03f));
    p.push_back(f(midiDecay, "MIDI Decay", 0.001f, 1.0f, 0.15f));
    p.push_back(f(midiSustain, "MIDI Sustain", 0.0f, 1.0f, 0.8f));
    p.push_back(f(midiRelease, "MIDI Release", 0.001f, 2.0f, 0.30f));

    return { p.begin(), p.end() };
}

starflux::dsp::AnalysisFrame StarFluxAudioProcessor::getLatestAnalysis() const noexcept
{
    starflux::dsp::AnalysisFrame f;
    f.rms = snapshot.rms.load(std::memory_order_relaxed);
    f.peak = snapshot.peak.load(std::memory_order_relaxed);
    f.low = snapshot.low.load(std::memory_order_relaxed);
    f.mid = snapshot.mid.load(std::memory_order_relaxed);
    f.high = snapshot.high.load(std::memory_order_relaxed);
    f.transient = snapshot.transient.load(std::memory_order_relaxed);
    return f;
}

starflux::engine::PlayheadState StarFluxAudioProcessor::getPlayheadState() const noexcept { return {}; }
juce::AudioProcessorEditor* StarFluxAudioProcessor::createEditor() { return new StarFluxAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new StarFluxAudioProcessor(); }
