// Heresy — synth engine. A C++ port of the Heresy web app's Web Audio engine.
// Depends only on juce_audio_basics and juce_dsp so it can be tested headless.
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "ParamDefs.h"

namespace heresy
{
using Values = std::array<float, NumParams>;

// Zavalishin TPT state-variable filter.
struct Svf
{
    float ic1 = 0, ic2 = 0, a1 = 0, a2 = 0, a3 = 0, k = 1;
    void set (float fc, float q, float sr) noexcept;
    inline void tick (float x, float& lp, float& bp, float& hp) noexcept
    {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        lp = v2; bp = v1; hp = x - k * v1 - v2;
    }
    void reset() noexcept { ic1 = ic2 = 0; }
};

struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void lowpass (float fc, float q, float sr) noexcept;
    inline float tick (float x) noexcept
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void reset() noexcept { z1 = z2 = 0; }
};

class Engine
{
public:
    static constexpr int maxVoices = 18;     // audible voices, as in the web app
    static constexpr int poolSize = 32;      // extra room for voices fading out after being stolen

    Engine();
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    // Call once per block with the raw parameter values (real units, choices as indices).
    void setParameters (const Values& v) noexcept { raw = v; }

    // Renders one block. midi sample positions must be inside [0, numSamples).
    void process (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi);

    // Reverb impulse. Safe to call from the message thread; generation is cheap.
    void loadImpulse (float sizeSeconds, bool reversed);
    float loadedSize() const noexcept { return irSize; }
    bool loadedReversed() const noexcept { return irReversed; }

    int getLatencySamples() const noexcept;
    void allNotesOff() noexcept;

private:
    struct Voice
    {
        bool active = false, released = false, killing = false, counted = false, formant = false;
        int owner = -1;            // MIDI note that triggered it
        uint64_t age = 0;
        float f = 440, gain = 0;
        double ph1 = 0, ph2 = 0, phSub = 0;
        // envelopes (captured at note start, like the web app)
        int ampStage = 0;          // 0 attack, 1 decay/sustain, 2 release
        float env = 0, atkInc = 1, decCoef = 0, sus = 0, relCoef = 0, killCoef = 0;
        int64_t remaining = 0;     // samples left after release/kill
        float pitchEnv = 0, pitchTc = 0.1f;
        float fenvTarget = 0, fenvVal = 0, fatkSamples = 1, fdecTc = 0.5f;
        int64_t fenvPos = 0;
        double driftPh = 0; float driftRate = 0.2f;
        int noiseColor = 1, noisePos = 0;
        int lastFilterType = 0;
        Svf filt, form[3];
    };

    struct Smooth
    {
        float value = 0, target = 0;
        void snap (float v) noexcept { value = target = v; }
        void tick (float coef) noexcept { value = target + (value - target) * coef; }
    };

    float eff (int id) const noexcept;
    void startVoice (int owner, int note, float scale, float velocity);
    void releaseVoice (Voice& v);
    void killVoice (Voice& v, float time);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void handleMidi (const juce::MidiMessage& m);
    void renderChunk (int start, int len);
    void updateSmoothers (int len);

    double sr = 48000;
    int maxBlock = 512;
    Values raw {};
    float modWheel = 0, bendCents = 0;
    bool sustainPedal = false;
    std::array<bool, 128> sustained {};
    std::array<int, 128> noteVoicing {};   // voice count started per note (for bookkeeping)
    std::array<Voice, poolSize> voices;
    uint64_t ageCounter = 0;
    juce::Random rng;

    // smoothed shared values (per 16-sample chunk)
    enum S { sO1lvl, sO2lvl, sSub, sNoise, sRing, sFm, sDrift, sO2fine, sO1oct, sO2semi, sCutoffLog, sReso,
             sVowel, sLpitch, sLfilt, NumS };
    std::array<Smooth, NumS> sm;
    bool smoothersReady = false;

    // LFO
    double lfoPh = 0; float lfoRand = 0;
    std::vector<float> lfoBuf;

    // buses
    std::vector<float> mono;
    std::vector<float> noiseBuf[3];
    int noiseLen = 1;

    // FX
    juce::dsp::Oversampling<float> oversampling { 1, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };
    juce::AudioBuffer<float> monoBlock;
    float lastDrive = -1;
    juce::SmoothedValue<float> makeup, tremDepth, wbDry, wbWet, wbDepth, dFb, dWet, rDry, rWet, masterGain;
    std::vector<float> wBuf; int wPos = 0; double wPh = 0;
    std::vector<float> dBuf; int dPos = 0; float dTime = 0.45f; Biquad dTone;
    juce::dsp::Convolution reverb { juce::dsp::Convolution::NonUniform { 512 } };
    juce::AudioBuffer<float> wet;
    float irSize = -1; bool irReversed = false;
    float compGr = 0, limGain = 1;
    float compAtk = 0, compRel = 0, limRel = 0;
};

// Helpers shared with tests.
juce::AudioBuffer<float> makeImpulse (double sampleRate, float size, bool reversed, juce::Random& rng);
} // namespace heresy
