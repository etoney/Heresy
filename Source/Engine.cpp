#include "Engine.h"

namespace heresy
{
namespace
{
constexpr float twoPi = juce::MathConstants<float>::twoPi;
constexpr float voiceGain = 0.24f;                   // VOICE_GAIN in the web app
constexpr int chunkSize = 16;
const float vowels[5][3] = { { 800, 1150, 2900 }, { 400, 1600, 2700 }, { 270, 1850, 2600 }, { 450, 800, 2830 }, { 325, 700, 2530 } };
const float formantGain[3] = { 1.0f * 2.4f, 0.55f * 2.4f, 0.32f * 2.4f };
const float formantQ[3] = { 8, 11, 13 };

inline float clampf (float v, float a, float b) noexcept { return v < a ? a : (v > b ? b : v); }
inline float centsToRatio (float c) noexcept { return std::exp2 (c / 1200.0f); }

// Web Audio lowpass/highpass Q is a resonance in dB. Convert to a filter Q the way Chrome does.
inline float webQdB (float dB) noexcept
{
    const float g = std::pow (10.0f, 0.05f * std::max (0.0f, dB));
    const float inner = 16.0f - 16.0f / (g * g);
    const float d = std::sqrt (std::max (1.0e-6f, (4.0f - std::sqrt (std::max (0.0f, inner))) * 0.5f));
    return 1.0f / d;
}

inline float polyBlep (float t, float dt) noexcept
{
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

inline float osc (int wave, float ph, float dt) noexcept
{
    switch (wave)
    {
        case Sine: return std::sin (twoPi * ph);
        case Triangle: return ph < 0.5f ? 4.0f * ph - 1.0f : 3.0f - 4.0f * ph;
        case Saw: return 2.0f * ph - 1.0f - polyBlep (ph, dt);
        default:
        {
            float s = ph < 0.5f ? 1.0f : -1.0f;
            s += polyBlep (ph, dt);
            float p2 = ph + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;
            s -= polyBlep (p2, dt);
            return s;
        }
    }
}

inline double wrap (double p) noexcept
{
    p -= std::floor (p);
    return p;
}
} // namespace

void Svf::set (float fc, float q, float sr) noexcept
{
    fc = clampf (fc, 10.0f, std::min (sr * 0.45f, 22000.0f));
    const float g = std::tan (juce::MathConstants<float>::pi * fc / sr);
    k = 1.0f / std::max (0.05f, q);
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}

void Biquad::lowpass (float fc, float q, float sr) noexcept
{
    const float w = twoPi * fc / sr, cw = std::cos (w), al = std::sin (w) / (2.0f * q);
    const float a0 = 1.0f + al;
    b0 = (1.0f - cw) * 0.5f / a0; b1 = (1.0f - cw) / a0; b2 = b0;
    a1 = -2.0f * cw / a0; a2 = (1.0f - al) / a0;
}

juce::AudioBuffer<float> makeImpulse (double sampleRate, float size, bool reversed, juce::Random& rng)
{
    const int len = std::max (1, (int) (sampleRate * size));
    juce::AudioBuffer<float> b (2, len);
    double power = 0;
    for (int ch = 0; ch < 2; ++ch)
    {
        auto* d = b.getWritePointer (ch);
        float y = 0;
        const float fadeIn = (float) (sampleRate * 0.012);
        for (int i = 0; i < len; ++i)
        {
            const float t = (float) i / (float) len;
            const float w = rng.nextFloat() * 2.0f - 1.0f;
            const float a = 0.25f + 0.6f * (1.0f - t);
            y += (w - y) * a;
            d[i] = y * std::pow (1.0f - t, 2.6f) * ((float) i < fadeIn ? (float) i / fadeIn : 1.0f);
            power += (double) d[i] * d[i];
        }
        if (reversed) std::reverse (d, d + len);
    }
    // Same normalisation as a Web Audio ConvolverNode (normalize = true).
    power = std::sqrt (power / (2.0 * len));
    if (! std::isfinite (power) || power < 0.000125) power = 0.000125;
    const float scale = (float) (1.0 / power * std::pow (10.0, -58.0 * 0.05) * 44100.0 / sampleRate);
    b.applyGain (scale);
    return b;
}

Engine::Engine()
{
    for (size_t i = 0; i < (size_t) NumParams; ++i) raw[i] = params()[i].def;
    noteVoicing.fill (0);
    sustained.fill (false);
}

void Engine::prepare (double sampleRate, int maxBlockSize)
{
    sr = sampleRate;
    maxBlock = std::max (1, maxBlockSize);
    lfoBuf.assign ((size_t) maxBlock, 0.0f);
    mono.assign ((size_t) maxBlock, 0.0f);
    monoBlock.setSize (1, maxBlock);
    wet.setSize (2, maxBlock);

    // noise: 3 s loops, same recipes as the web app
    noiseLen = (int) (sr * 3.0);
    for (int c = 0; c < 3; ++c)
    {
        auto& d = noiseBuf[c];
        d.assign ((size_t) noiseLen, 0.0f);
        float b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, last = 0;
        for (int i = 0; i < noiseLen; ++i)
        {
            const float w = rng.nextFloat() * 2.0f - 1.0f;
            if (c == White) d[(size_t) i] = w * 0.5f;
            else if (c == Pink)
            {
                b0 = 0.99886f * b0 + w * 0.0555179f; b1 = 0.99332f * b1 + w * 0.0750759f;
                b2 = 0.969f * b2 + w * 0.153852f;    b3 = 0.8665f * b3 + w * 0.3104856f;
                b4 = 0.55f * b4 + w * 0.5329522f;    b5 = -0.7616f * b5 - w * 0.016898f;
                d[(size_t) i] = (b0 + b1 + b2 + b3 + b4 + b5 + b6 + w * 0.5362f) * 0.12f;
                b6 = w * 0.115926f;
            }
            else { last = (last + 0.02f * w) / 1.02f; d[(size_t) i] = last * 3.2f; }
        }
    }

    oversampling.reset();
    oversampling.initProcessing ((size_t) maxBlock);

    wBuf.assign ((size_t) (sr * 0.2) + 8, 0.0f);
    dBuf.assign ((size_t) (sr * 2.5) + 8, 0.0f);
    dTone.lowpass (3200.0f, webQdB (1.0f), (float) sr);

    for (auto* s : { &makeup, &tremDepth, &wbDry, &wbWet, &wbDepth, &dFb, &dWet, &rDry, &rWet, &masterGain })
        s->reset (sr, 0.03);

    compAtk = std::exp (-1.0f / (0.002f * (float) sr));
    compRel = std::exp (-1.0f / (0.2f * (float) sr));
    limRel = std::exp (-1.0f / (0.2f * (float) sr));

    reverb.prepare ({ sr, (juce::uint32) maxBlock, 2 });
    loadImpulse (raw[rsize], raw[rrev] > 0.5f);

    smoothersReady = false;
    lastDrive = -1;
    reset();
}

void Engine::reset()
{
    for (auto& v : voices) v = Voice();
    sustained.fill (false);
    std::fill (wBuf.begin(), wBuf.end(), 0.0f);
    std::fill (dBuf.begin(), dBuf.end(), 0.0f);
    dTone.reset();
    reverb.reset();
    oversampling.reset();
    compGr = 0; limGain = 1;
    lfoRand = rng.nextFloat() * 2.0f - 1.0f;
}

void Engine::loadImpulse (float size, bool reversed)
{
    irSize = size; irReversed = reversed;
    juce::Random r;
    reverb.loadImpulseResponse (makeImpulse (sr, size, reversed, r), sr,
                                juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no,
                                juce::dsp::Convolution::Normalise::no);
}

int Engine::getLatencySamples() const noexcept
{
    return (int) std::round (oversampling.getLatencyInSamples());
}

float Engine::eff (int id) const noexcept
{
    float v = raw[(size_t) id];
    for (auto& m : macroRoutes())
        if (m.target == id)
        {
            float macro = raw[(size_t) m.macro];
            if (m.macro == torment) macro = clampf (macro + modWheel, 0.0f, 1.0f);   // mod wheel adds Torment
            v += macro * m.amount;
        }
    auto& d = params()[(size_t) id];
    return clampf (v, d.min, d.max);
}

// ---------------------------------------------------------------- notes
void Engine::startVoice (int owner, int note, float scale, float velocity)
{
    // keep at most maxVoices audible: fade out the oldest
    int counted = 0;
    Voice* oldest = nullptr;
    for (auto& v : voices)
        if (v.active && v.counted)
        {
            ++counted;
            if (oldest == nullptr || v.age < oldest->age) oldest = &v;
        }
    if (counted >= maxVoices && oldest != nullptr) killVoice (*oldest, 0.03f);

    Voice* slot = nullptr;
    for (auto& v : voices) if (! v.active) { slot = &v; break; }
    if (slot == nullptr)
    {
        for (auto& v : voices) if (slot == nullptr || v.age < slot->age) slot = &v;
    }

    Voice& v = *slot;
    v = Voice();
    v.active = true; v.counted = true;
    v.owner = owner;
    v.age = ++ageCounter;
    v.f = 440.0f * std::exp2 ((float) (note - 69) / 12.0f);
    v.gain = voiceGain * scale * (0.35f + 0.65f * velocity);
    v.formant = (int) raw[ftype] == Formant;
    v.lastFilterType = v.formant ? LowPass : (int) raw[ftype];

    const float fsr = (float) sr;
    v.ampStage = 0;
    v.env = 0;
    v.atkInc = 1.0f / std::max (1.0f, raw[atk] * fsr);
    v.sus = raw[sus];
    v.decCoef = std::exp (-1.0f / std::max (1.0f, raw[dec] / 3.0f * fsr));

    v.pitchEnv = raw[penv] * 100.0f;
    v.pitchTc = std::max (0.0005f, raw[ptime] / 4.0f);

    v.fenvTarget = raw[fenv] * 1200.0f;
    v.fatkSamples = std::max (1.0f, raw[fatk] * fsr);
    v.fdecTc = std::max (0.0005f, raw[fdec] / 3.0f);
    v.fenvPos = 0; v.fenvVal = 0;

    v.driftRate = 0.06f + rng.nextFloat() * 0.35f;
    v.noiseColor = juce::jlimit (0, 2, (int) raw[noiseColor]);
    v.noisePos = rng.nextInt (juce::jmax (1, std::min (noiseLen, (int) (sr * 2.5))));
}

void Engine::releaseVoice (Voice& v)
{
    if (v.released) return;
    v.released = true;
    v.ampStage = 2;
    const float rl = raw[rel];
    v.relCoef = std::exp (-1.0f / std::max (1.0f, rl / 5.0f * (float) sr));
    v.remaining = (int64_t) ((rl * 1.25f + 0.05f) * (float) sr);
}

void Engine::killVoice (Voice& v, float time)
{
    v.released = true; v.killing = true; v.counted = false;
    v.ampStage = 2;
    v.relCoef = std::exp (-1.0f / std::max (1.0f, time / 4.0f * (float) sr));
    v.remaining = (int64_t) ((time + 0.02f) * (float) sr);
}

void Engine::noteOn (int note, float velocity)
{
    // retrigger: a held note is stopped first, as in the web app
    for (auto& v : voices)
        if (v.active && v.owner == note && ! v.released) releaseVoice (v);
    sustained[(size_t) note] = false;

    auto& iv = voicings()[(size_t) juce::jlimit (0, (int) voicings().size() - 1, (int) raw[voicing])].iv;
    const float scale = 1.0f / std::sqrt ((float) iv.size());
    for (int i : iv)
        startVoice (note, note + i, scale, velocity);
}

void Engine::noteOff (int note)
{
    if (sustainPedal) { sustained[(size_t) note] = true; return; }
    for (auto& v : voices)
        if (v.active && v.owner == note && ! v.released) releaseVoice (v);
}

void Engine::allNotesOff() noexcept
{
    for (auto& v : voices)
        if (v.active) killVoice (v, 0.05f);
    sustained.fill (false);
    sustainPedal = false;
}

void Engine::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn()) noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff()) noteOff (m.getNoteNumber());
    else if (m.isPitchWheel()) bendCents = (float) (m.getPitchWheelValue() - 8192) / 8192.0f * 200.0f;
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        if (cc == 1) modWheel = (float) m.getControllerValue() / 127.0f;
        else if (cc == 64)
        {
            sustainPedal = m.getControllerValue() >= 64;
            if (! sustainPedal)
                for (int n = 0; n < 128; ++n)
                    if (sustained[(size_t) n]) { sustained[(size_t) n] = false; noteOff (n); }
        }
        else if (cc == 120 || cc == 123) allNotesOff();
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
}

// ---------------------------------------------------------------- render
void Engine::updateSmoothers (int len)
{
    const float targets[NumS] = {
        raw[o1lvl], raw[o2lvl], raw[sub], eff (noise), eff (ring), eff (fm), eff (drift), eff (o2fine),
        raw[o1oct], raw[o2semi], std::log (raw[cutoff]), raw[reso], raw[vowel], eff (lpitch), raw[lfilt]
    };
    const float coef = std::exp (-(float) len / (0.025f * (float) sr));
    for (int i = 0; i < NumS; ++i)
    {
        sm[(size_t) i].target = targets[i];
        if (! smoothersReady) sm[(size_t) i].value = targets[i];
        else sm[(size_t) i].tick (coef);
    }
    smoothersReady = true;
}

void Engine::renderChunk (int start, int len)
{
    updateSmoothers (len);
    const float fsr = (float) sr, invSr = 1.0f / fsr;
    const float lfoNow = lfoBuf[(size_t) start];
    const float o1l = sm[sO1lvl].value, o2l = sm[sO2lvl].value, subl = sm[sSub].value;
    const float nl = sm[sNoise].value, rl = sm[sRing].value, fmE = sm[sFm].value, driftE = sm[sDrift].value;
    const float fine = sm[sO2fine].value, oct1 = sm[sO1oct].value, semi2 = sm[sO2semi].value;
    const float cut = std::exp (sm[sCutoffLog].value), rs = sm[sReso].value, vw = clampf (sm[sVowel].value, 0, 4);
    const float lpE = sm[sLpitch].value, lfE = sm[sLfilt].value;
    const int w1 = (int) raw[o1wave], w2 = (int) raw[o2wave];
    const int ft = (int) raw[ftype];

    // formant table for the current vowel
    float vf[3];
    {
        const int i = std::min (3, (int) std::floor (vw));
        const float fr = vw - (float) i;
        for (int k = 0; k < 3; ++k) vf[k] = vowels[i][k] + (vowels[i + 1][k] - vowels[i][k]) * fr;
    }
    const float formScale = 0.5f + rs / 12.0f;

    for (auto& v : voices)
    {
        if (! v.active) continue;

        // pitch: dive envelope + drift + LFO + bend (cents), shared by all oscillators
        const float drift = std::sin (twoPi * (float) v.driftPh) * driftE;
        const float cents = v.pitchEnv + drift + lfoNow * lpE + bendCents;
        const float ratio = centsToRatio (cents);
        const float f1 = v.f * std::exp2 (oct1) * ratio;
        const float f2 = v.f * std::exp2 (semi2 / 12.0f) * ratio * centsToRatio (fine);
        const float fs = v.f * std::exp2 (oct1 - 1.0f) * ratio;
        const float fmAmt = fmE * fmE * v.f * 6.0f;
        const float inc2 = f2 * invSr, incS = fs * invSr;
        const float dt2 = std::min (0.5f, std::abs (inc2));

        // filter: envelope + LFO in cents
        const float fCents = v.fenvVal + lfoNow * lfE;
        const float fRatio = centsToRatio (fCents);
        if (v.formant)
            for (int k = 0; k < 3; ++k) v.form[k].set (vf[k] * fRatio, formantQ[k] * formScale, fsr);
        else
        {
            if (ft != Formant) v.lastFilterType = ft;
            const bool lphp = v.lastFilterType == LowPass || v.lastFilterType == HighPass;
            v.filt.set (cut * fRatio, lphp ? webQdB (rs) : std::max (0.4f, rs), fsr);
        }

        const auto& nb = noiseBuf[v.noiseColor];
        float* out = mono.data() + start;
        for (int i = 0; i < len; ++i)
        {
            const float p2 = (float) v.ph2;
            const float s2 = osc (w2, p2, dt2);
            v.ph2 = wrap (v.ph2 + inc2);

            const float inc1 = (f1 + s2 * fmAmt) * invSr;
            const float s1 = osc (w1, (float) v.ph1, std::min (0.5f, std::abs (inc1)));
            v.ph1 = wrap (v.ph1 + inc1);

            const float ss = std::sin (twoPi * (float) v.phSub);
            v.phSub = wrap (v.phSub + incS);

            const float n = nb[(size_t) v.noisePos];
            if (++v.noisePos >= noiseLen) v.noisePos = 0;

            const float mix = (s1 * o1l + s2 * o2l + ss * subl + s1 * s2 * rl + n * nl) * v.gain;

            float y;
            if (v.formant)
            {
                y = 0;
                for (int k = 0; k < 3; ++k)
                {
                    float lp, bp, hp;
                    v.form[k].tick (mix, lp, bp, hp);
                    y += bp * v.form[k].k * formantGain[k];
                }
            }
            else
            {
                float lp, bp, hp;
                v.filt.tick (mix, lp, bp, hp);
                switch (v.lastFilterType)
                {
                    case HighPass: y = hp; break;
                    case BandPass: y = bp * v.filt.k; break;
                    case Notch: y = mix - bp * v.filt.k; break;
                    default: y = lp; break;
                }
            }

            // amp envelope
            if (v.ampStage == 0)
            {
                v.env += v.atkInc;
                if (v.env >= 1.0f) { v.env = 1.0f; v.ampStage = 1; }
            }
            else if (v.ampStage == 1) v.env = v.sus + (v.env - v.sus) * v.decCoef;
            else
            {
                v.env *= v.relCoef;
                if (--v.remaining <= 0) { v.active = false; break; }
            }

            out[i] += y * v.env;
        }
        if (! v.active) continue;

        // advance chunk-rate modulators
        const float l = (float) len;
        v.pitchEnv *= std::exp (-l / (v.pitchTc * fsr));
        v.driftPh = wrap (v.driftPh + v.driftRate * l * invSr);
        v.fenvPos += len;
        if ((float) v.fenvPos < v.fatkSamples) v.fenvVal = v.fenvTarget * (float) v.fenvPos / v.fatkSamples;
        else
        {
            if ((float) (v.fenvPos - len) < v.fatkSamples) v.fenvVal = v.fenvTarget;
            v.fenvVal *= std::exp (-l / (v.fdecTc * fsr));
        }
    }
}

void Engine::process (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi)
{
    const int total = out.getNumSamples();
    auto midiIt = midi.begin();

    for (int blockStart = 0; blockStart < total; blockStart += maxBlock)
    {
        const int n = std::min (maxBlock, total - blockStart);
        const float fsr = (float) sr;

        // ---- LFO (free running, like the web app)
        {
            const int shape = (int) raw[lshape];
            const double inc = raw[lrate] / sr;
            for (int i = 0; i < n; ++i)
            {
                const float p = (float) lfoPh;
                float v;
                switch (shape)
                {
                    case LSine: v = std::sin (twoPi * p); break;
                    case LTriangle: v = p < 0.5f ? 4.0f * p - 1.0f : 3.0f - 4.0f * p; break;
                    case LSquare: v = p < 0.5f ? 1.0f : -1.0f; break;
                    case LSaw: v = 1.0f - 2.0f * p; break;
                    default: v = lfoRand; break;
                }
                lfoBuf[(size_t) i] = v;
                lfoPh += inc;
                if (lfoPh >= 1.0) { lfoPh -= std::floor (lfoPh); lfoRand = rng.nextFloat() * 2.0f - 1.0f; }
            }
        }

        // ---- voices, split at MIDI events and into 16-sample chunks
        std::fill (mono.begin(), mono.begin() + n, 0.0f);
        int pos = 0;
        while (pos < n)
        {
            while (midiIt != midi.end() && (*midiIt).samplePosition <= blockStart + pos)
            {
                handleMidi ((*midiIt).getMessage());
                ++midiIt;
            }
            int next = n;
            if (midiIt != midi.end()) next = juce::jlimit (pos + 1, n, (*midiIt).samplePosition - blockStart);
            const int len = std::min (chunkSize, next - pos);
            renderChunk (pos, len);
            pos += len;
        }

        // ---- tremolo (LFO to amp)
        const float lampV = raw[lamp];
        tremDepth.setTargetValue (lampV * 0.5f);
        for (int i = 0; i < n; ++i)
        {
            const float d = tremDepth.getNextValue();
            mono[(size_t) i] *= (1.0f - d) + lfoBuf[(size_t) i] * d;
        }

        // ---- drive (4x oversampled waveshaper; hard clip at +-1 like a WaveShaperNode)
        const float dr = std::round (eff (drive) * 100.0f) / 100.0f;
        const float k = dr * 60.0f;
        makeup.setTargetValue (1.0f / (1.0f + dr * 1.6f));
        {
            auto* mb = monoBlock.getWritePointer (0);
            std::copy (mono.begin(), mono.begin() + n, mb);
            juce::dsp::AudioBlock<float> block (monoBlock.getArrayOfWritePointers(), 1, (size_t) n);
            auto up = oversampling.processSamplesUp (block);
            const float norm = k < 0.01f ? 1.0f : 1.0f / std::tanh (1.0f + k);
            auto* u = up.getChannelPointer (0);
            for (size_t i = 0; i < up.getNumSamples(); ++i)
            {
                const float x = clampf (u[i], -1.0f, 1.0f);
                u[i] = k < 0.01f ? x : std::tanh (x * (1.0f + k)) * norm;
            }
            oversampling.processSamplesDown (block);
            std::copy (mb, mb + n, mono.begin());
        }

        // ---- bit crush, makeup
        const int bitsV = (int) std::round (eff (bits));
        const float steps = std::exp2 ((float) (bitsV - 1));
        for (int i = 0; i < n; ++i)
        {
            float x = mono[(size_t) i];
            if (bitsV < 16) x = std::round (clampf (x, -1.0f, 1.0f) * steps) / steps;
            mono[(size_t) i] = x * makeup.getNextValue();
        }

        // ---- warble (modulated short delay)
        const float wb = eff (warble);
        wbDry.setTargetValue (1.0f - wb * 0.6f);
        wbWet.setTargetValue (wb);
        wbDepth.setTargetValue (wb * 0.007f);
        {
            const int wl = (int) wBuf.size();
            const double wInc = raw[wrate] / sr;
            for (int i = 0; i < n; ++i)
            {
                const float x = mono[(size_t) i];
                wBuf[(size_t) wPos] = x;
                const float dSec = 0.014f + std::sin (twoPi * (float) wPh) * wbDepth.getNextValue();
                wPh = wrap (wPh + wInc);
                float rp = (float) wPos - dSec * fsr;
                while (rp < 0) rp += (float) wl;
                const int i0 = (int) rp; const float fr = rp - (float) i0;
                const float d = wBuf[(size_t) i0] + (wBuf[(size_t) ((i0 + 1) % wl)] - wBuf[(size_t) i0]) * fr;
                if (++wPos >= wl) wPos = 0;
                mono[(size_t) i] = x * wbDry.getNextValue() + d * wbWet.getNextValue();
            }
        }

        // ---- delay (feedback through a 3.2 kHz low pass)
        dFb.setTargetValue (raw[dfb]);
        dWet.setTargetValue (raw[dmix]);
        {
            const int dl = (int) dBuf.size();
            const float tcoef = std::exp (-1.0f / (0.06f * fsr));
            const float target = raw[dtime];
            for (int i = 0; i < n; ++i)
            {
                dTime = target + (dTime - target) * tcoef;
                float rp = (float) dPos - dTime * fsr;
                while (rp < 0) rp += (float) dl;
                const int i0 = (int) rp; const float fr = rp - (float) i0;
                const float d = dBuf[(size_t) i0] + (dBuf[(size_t) ((i0 + 1) % dl)] - dBuf[(size_t) i0]) * fr;
                const float tone = dTone.tick (d);
                const float x = mono[(size_t) i];
                dBuf[(size_t) dPos] = x + tone * dFb.getNextValue();
                if (++dPos >= dl) dPos = 0;
                mono[(size_t) i] = x + tone * dWet.getNextValue();
            }
        }

        // ---- reverb (stereo impulse), equal-power mix
        const float m = raw[rmix];
        rDry.setTargetValue (std::cos (m * juce::MathConstants<float>::halfPi));
        rWet.setTargetValue (std::sin (m * juce::MathConstants<float>::halfPi) * 1.15f);
        wet.setSize (2, n, false, false, true);
        for (int ch = 0; ch < 2; ++ch) wet.copyFrom (ch, 0, mono.data(), n);
        {
            juce::dsp::AudioBlock<float> wb2 (wet);
            reverb.process (juce::dsp::ProcessContextReplacing<float> (wb2));
        }

        // ---- master, compressor (-4 dB, 20:1, like the web app's limiter), final ceiling
        masterGain.setTargetValue (raw[master] * 1.15f);
        auto* L = wet.getWritePointer (0);
        auto* R = wet.getWritePointer (1);
        const float thr = -4.0f, ratio = 20.0f, makeupComp = 1.30f, ceiling = 0.912f;
        for (int i = 0; i < n; ++i)
        {
            const float dry = mono[(size_t) i] * rDry.getNextValue();
            const float wg = rWet.getNextValue();
            const float mg = masterGain.getNextValue();
            float l = (dry + L[i] * wg) * mg;
            float r = (dry + R[i] * wg) * mg;

            const float peak = std::max (std::abs (l), std::abs (r));
            const float lvl = juce::Decibels::gainToDecibels (peak, -120.0f);
            const float over = lvl - thr;
            const float grT = over > 0 ? over * (1.0f - 1.0f / ratio) : 0.0f;
            compGr = grT > compGr ? grT + (compGr - grT) * compAtk : grT + (compGr - grT) * compRel;
            const float g = juce::Decibels::decibelsToGain (-compGr) * makeupComp;
            l *= g; r *= g;

            const float pk = std::max (std::abs (l), std::abs (r));
            const float need = pk > ceiling ? ceiling / pk : 1.0f;
            limGain = need < limGain ? need : 1.0f + (limGain - 1.0f) * limRel;
            l *= limGain; r *= limGain;

            if (! std::isfinite (l) || ! std::isfinite (r)) { l = r = 0; }
            L[i] = l; R[i] = r;
        }

        if (out.getNumChannels() >= 2)
        {
            out.copyFrom (0, blockStart, wet, 0, 0, n);
            out.copyFrom (1, blockStart, wet, 1, 0, n);
            for (int ch = 2; ch < out.getNumChannels(); ++ch) out.clear (ch, blockStart, n);
        }
        else if (out.getNumChannels() == 1)
        {
            out.copyFrom (0, blockStart, wet, 0, 0, n);
            out.addFrom (0, blockStart, wet, 1, 0, n);
            out.applyGain (0, blockStart, n, 0.5f);
        }
    }

    // events at or past the end of the block
    for (; midiIt != midi.end(); ++midiIt) handleMidi ((*midiIt).getMessage());
}
} // namespace heresy
