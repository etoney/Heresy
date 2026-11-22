#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FactoryPresets.h"

using namespace heresy;

// ---------------------------------------------------------------- presets on disk
juce::File PresetManager::userFolder()
{
   #if JUCE_MAC
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Application Support");
   #else
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #endif
    auto dir = base.getChildFile ("Castle Danger Music Company").getChildFile ("Heresy").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

juce::StringArray PresetManager::userPresetNames()
{
    juce::StringArray names;
    for (auto& f : userFolder().findChildFiles (juce::File::findFiles, false, "*.heresy"))
        names.add (f.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

juce::File PresetManager::fileFor (const juce::String& name)
{
    return userFolder().getChildFile (juce::File::createLegalFileName (name.trim()) + ".heresy");
}

// ---------------------------------------------------------------- parameters
juce::String HeresyProcessor::hostName (int i)
{
    static const char* names[NumParams] = {
        "Osc 1 Wave", "Osc 1 Octave", "Osc 1 Level", "Sub Level", "Osc 2 Wave", "Osc 2 Semitones", "Osc 2 Fine", "Osc 2 Level",
        "FM", "Ring Mod", "Drift", "Noise Level", "Noise Color",
        "Voicing", "Dive", "Dive Time",
        "Filter Type", "Cutoff", "Resonance", "Vowel", "Filter Env Amount", "Filter Env Attack", "Filter Env Decay",
        "Attack", "Decay", "Sustain", "Release",
        "LFO Shape", "LFO Rate", "LFO to Pitch", "LFO to Filter", "LFO to Amp",
        "Drive", "Bits", "Warble", "Warble Rate",
        "Delay Time", "Delay Feedback", "Delay Mix", "Reverb Size", "Reverb Mix", "Reverb Mode",
        "Torment", "Crush", "Volume"
    };
    return names[i];
}

juce::AudioProcessorValueTreeState::ParameterLayout HeresyProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < NumParams; ++i)
    {
        auto& d = params()[(size_t) i];
        const juce::ParameterID pid { d.id, 1 };
        if (! d.choiceIds.empty())
        {
            juce::StringArray items;
            for (auto& n : d.choiceNames) items.add (n);
            layout.add (std::make_unique<juce::AudioParameterChoice> (pid, hostName (i), items, (int) d.def));
            continue;
        }

        juce::NormalisableRange<float> range;
        if (d.log)
        {
            range = juce::NormalisableRange<float> (d.min, d.max,
                [] (float s, float e, float v) { return s * std::pow (e / s, v); },
                [] (float s, float e, float x) { return std::log (x / s) / std::log (e / s); },
                [] (float s, float e, float x) { return juce::jlimit (s, e, x); });
        }
        else
        {
            range = juce::NormalisableRange<float> (d.min, d.max, d.step);
        }

        auto attrs = juce::AudioParameterFloatAttributes()
                         .withStringFromValueFunction ([i] (float v, int) { return juce::String (formatValue (i, v)); })
                         .withValueFromStringFunction ([] (const juce::String& s) { return s.retainCharacters ("-0123456789.").getFloatValue(); });
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid, hostName (i), range, d.def, attrs));
    }
    return layout;
}

// ---------------------------------------------------------------- lifecycle
HeresyProcessor::HeresyProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "HERESY", createLayout())
{
    for (int i = 0; i < NumParams; ++i)
        rawParams[(size_t) i] = apvts.getRawParameterValue (params()[(size_t) i].id);
    startTimerHz (15);
}

HeresyProcessor::~HeresyProcessor() { stopTimer(); }

bool HeresyProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void HeresyProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (int i = 0; i < NumParams; ++i) values[(size_t) i] = rawParams[(size_t) i]->load();
    engine.setParameters (values);
    engine.prepare (sampleRate, juce::jmax (32, samplesPerBlock));
    setLatencySamples (engine.getLatencySamples());
    keyboardState.reset();
    prepared = true;
}

void HeresyProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    if (panicRequested.exchange (false)) engine.allNotesOff();

    for (int i = 0; i < NumParams; ++i) values[(size_t) i] = rawParams[(size_t) i]->load();
    engine.setParameters (values);

    buffer.clear();
    engine.process (buffer, midi);
}

void HeresyProcessor::timerCallback()
{
    if (! prepared) return;
    const float size = rawParams[rsize]->load();
    const bool rev = rawParams[rrev]->load() > 0.5f;
    const bool differs = std::abs (size - engine.loadedSize()) > 0.005f || rev != engine.loadedReversed();
    if (! differs) { pendingTicks = 0; return; }

    if (std::abs (size - pendingSize) > 0.005f || rev != pendingRev)
    {
        pendingSize = size; pendingRev = rev; pendingTicks = 0;   // still moving: wait
        return;
    }
    if (++pendingTicks >= 3)                                      // ~200 ms steady
    {
        engine.loadImpulse (size, rev);
        pendingTicks = 0;
    }
}

// ---------------------------------------------------------------- presets
void HeresyProcessor::applyValues (const Values& v)
{
    for (int i = 0; i < NumParams; ++i)
        if (auto* p = apvts.getParameter (params()[(size_t) i].id))
            p->setValueNotifyingHost (p->convertTo0to1 (v[(size_t) i]));
}

void HeresyProcessor::loadFactoryPreset (int index)
{
    if (index < 0 || index >= (int) factoryPresets().size()) return;
    applyValues (factoryValues ((size_t) index));
    {
        const juce::ScopedLock sl (presetLock);
        presetName = factoryPresets()[(size_t) index].name;
    }
    presetIsUser = false;
    factoryIndex = index;
    ++presetCounter;
}

bool HeresyProcessor::loadUserPreset (const juce::String& name)
{
    auto xml = juce::XmlDocument::parse (PresetManager::fileFor (name));
    if (xml == nullptr) return false;
    auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.isValid()) return false;

    Values v {};
    for (int i = 0; i < NumParams; ++i) v[(size_t) i] = params()[(size_t) i].def;
    for (int c = 0; c < tree.getNumChildren(); ++c)
    {
        auto child = tree.getChild (c);
        const int idx = indexOf (child.getProperty ("id").toString().toStdString());
        if (idx >= 0 && child.hasProperty ("value")) v[(size_t) idx] = (float) child.getProperty ("value");
    }
    applyValues (v);
    {
        const juce::ScopedLock sl (presetLock);
        presetName = name;
    }
    presetIsUser = true;
    ++presetCounter;
    return true;
}

bool HeresyProcessor::saveUserPreset (const juce::String& name)
{
    if (name.trim().isEmpty()) return false;
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
    {
        if (! xml->writeTo (PresetManager::fileFor (name))) return false;
        {
            const juce::ScopedLock sl (presetLock);
            presetName = juce::File::createLegalFileName (name.trim());
        }
        presetIsUser = true;
        ++presetCounter;
        return true;
    }
    return false;
}

bool HeresyProcessor::deleteUserPreset (const juce::String& name)
{
    const bool ok = PresetManager::fileFor (name).deleteFile();
    if (ok) loadFactoryPreset (0);
    return ok;
}

juce::String HeresyProcessor::getPresetName() const
{
    const juce::ScopedLock sl (presetLock);
    return presetName;
}

int HeresyProcessor::getNumPrograms() { return (int) factoryPresets().size(); }
int HeresyProcessor::getCurrentProgram() { return presetIsUser ? 0 : factoryIndex.load(); }
void HeresyProcessor::setCurrentProgram (int index) { loadFactoryPreset (index); }
const juce::String HeresyProcessor::getProgramName (int index)
{
    return juce::isPositiveAndBelow (index, (int) factoryPresets().size()) ? juce::String (factoryPresets()[(size_t) index].name) : juce::String();
}

// ---------------------------------------------------------------- state
void HeresyProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("preset", getPresetName(), nullptr);
    state.setProperty ("presetIsUser", presetIsUser.load(), nullptr);
    state.setProperty ("factoryIndex", factoryIndex.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void HeresyProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return;
    auto tree = juce::ValueTree::fromXml (*xml);
    {
        const juce::ScopedLock sl (presetLock);
        presetName = tree.getProperty ("preset", "Init").toString();
    }
    presetIsUser = (bool) tree.getProperty ("presetIsUser", false);
    factoryIndex = (int) tree.getProperty ("factoryIndex", 0);
    apvts.replaceState (tree);
    ++presetCounter;
}

juce::AudioProcessorEditor* HeresyProcessor::createEditor() { return new HeresyEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new HeresyProcessor(); }
