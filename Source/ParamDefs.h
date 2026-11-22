// Heresy — parameter definitions shared by the engine, the plugin and the UI.
// IDs, ranges and defaults match the Heresy web app (castledangermusic.com/project/heresy).
#pragma once

#include <array>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <vector>

namespace heresy
{
enum class Fmt { pct, hz, hzs, s, st, ct, oct, f1, intg, bits, vowel, choice };

struct ParamDef
{
    const char* id;
    const char* label;
    float min, max, def;
    float step;     // 0 = continuous
    bool log;
    Fmt fmt;
    std::vector<std::string> choiceIds;    // non-empty => choice parameter (def is an index)
    std::vector<std::string> choiceNames;
};

// Indices into the parameter table. Order is stable: never reorder (saved state uses IDs, but code uses these).
enum P : int
{
    o1wave, o1oct, o1lvl, sub, o2wave, o2semi, o2fine, o2lvl,
    fm, ring, drift, noise, noiseColor,
    voicing, penv, ptime,
    ftype, cutoff, reso, vowel, fenv, fatk, fdec,
    atk, dec, sus, rel,
    lshape, lrate, lpitch, lfilt, lamp,
    drive, bits, warble, wrate,
    dtime, dfb, dmix, rsize, rmix, rrev,
    torment, crush, master,
    NumParams
};

enum Wave { Sine, Triangle, Saw, Square };
enum NoiseColor { White, Pink, Brown };
enum FilterType { LowPass, HighPass, BandPass, Notch, Formant };
enum LfoShape { LSine, LTriangle, LSquare, LSaw, LRandom };

struct Voicing { const char* id; const char* name; std::vector<int> iv; };

inline const std::vector<Voicing>& voicings()
{
    static const std::vector<Voicing> v {
        { "single",  "Single",      { 0 } },
        { "octave",  "Octave Doom", { -12, 0 } },
        { "tritone", "Tritone",     { 0, 6 } },
        { "minor2",  "Minor 2nd",   { 0, 1 } },
        { "dim",     "Diminished",  { 0, 3, 6, 9 } },
        { "cluster", "Cluster",     { 0, 1, 2, 3 } },
        { "abyss",   "Abyss",       { -24, -12, -11, 0 } },
        { "sting",   "Stinger",     { 0, 1, 6, 11, 13 } },
    };
    return v;
}

inline const std::array<ParamDef, NumParams>& params()
{
    static const std::vector<std::string> waveIds { "sine", "triangle", "sawtooth", "square" };
    static const std::vector<std::string> waveNames { "Sine", "Tri", "Saw", "Square" };

    static const std::array<ParamDef, NumParams> p = [] {
        std::vector<std::string> vIds, vNames;
        for (auto& v : voicings()) { vIds.push_back (v.id); vNames.push_back (v.name); }

        auto C = [] (const char* id, const char* label, int def, std::vector<std::string> ids, std::vector<std::string> names)
        {
            return ParamDef { id, label, 0.0f, (float) ids.size() - 1.0f, (float) def, 1.0f, false, Fmt::choice, ids, names };
        };
        auto N = [] (const char* id, const char* label, float mn, float mx, float def, Fmt f, float step = 0.0f, bool lg = false)
        {
            return ParamDef { id, label, mn, mx, def, step, lg, f, {}, {} };
        };

        return std::array<ParamDef, NumParams> {
            C ("o1wave", "Osc 1", 2, waveIds, waveNames),
            N ("o1oct", "Oct", -3, 3, 0, Fmt::intg, 1),
            N ("o1lvl", "Level", 0, 1, 0.7f, Fmt::pct),
            N ("sub", "Sub", 0, 1, 0.3f, Fmt::pct),
            C ("o2wave", "Osc 2", 2, waveIds, waveNames),
            N ("o2semi", "Semi", -24, 24, 6, Fmt::st, 1),
            N ("o2fine", "Fine", -100, 100, 7, Fmt::ct),
            N ("o2lvl", "Level", 0, 1, 0.4f, Fmt::pct),

            N ("fm", "FM", 0, 1, 0, Fmt::pct),
            N ("ring", "Ring", 0, 1, 0, Fmt::pct),
            N ("drift", "Drift", 0, 100, 10, Fmt::ct),
            N ("noise", "Noise", 0, 1, 0.08f, Fmt::pct),
            C ("noiseColor", "Color", 1, { "white", "pink", "brown" }, { "White", "Pink", "Brown" }),

            C ("voicing", "Voicing", 0, vIds, vNames),
            N ("penv", "Dive", -48, 48, 0, Fmt::st, 1),
            N ("ptime", "Dive Time", 0.01f, 10, 0.5f, Fmt::s, 0, true),

            C ("ftype", "Filter", 0, { "lowpass", "highpass", "bandpass", "notch", "formant" },
                                     { "Low Pass", "High Pass", "Band Pass", "Notch", "Formant (Voice)" }),
            N ("cutoff", "Cutoff", 20, 18000, 1400, Fmt::hz, 0, true),
            N ("reso", "Reso", 0, 25, 4, Fmt::f1),
            N ("vowel", "Vowel", 0, 4, 0, Fmt::vowel),
            N ("fenv", "Env Amt", -6, 6, 2, Fmt::oct),
            N ("fatk", "Env Atk", 0.001f, 10, 0.01f, Fmt::s, 0, true),
            N ("fdec", "Env Dec", 0.01f, 12, 1.5f, Fmt::s, 0, true),

            N ("atk", "Attack", 0.001f, 12, 0.04f, Fmt::s, 0, true),
            N ("dec", "Decay", 0.01f, 12, 0.8f, Fmt::s, 0, true),
            N ("sus", "Sustain", 0, 1, 0.7f, Fmt::pct),
            N ("rel", "Release", 0.01f, 16, 2, Fmt::s, 0, true),

            C ("lshape", "Shape", 0, { "sine", "triangle", "square", "saw", "random" },
                                     { "Sine", "Triangle", "Square", "Saw", "Random S&H" }),
            N ("lrate", "Rate", 0.02f, 30, 3, Fmt::hzs, 0, true),
            N ("lpitch", "To Pitch", 0, 1200, 0, Fmt::ct),
            N ("lfilt", "To Filter", 0, 4800, 0, Fmt::ct),
            N ("lamp", "To Amp", 0, 1, 0, Fmt::pct),

            N ("drive", "Drive", 0, 1, 0.12f, Fmt::pct),
            N ("bits", "Bits", 2, 16, 16, Fmt::bits, 1),
            N ("warble", "Warble", 0, 1, 0.2f, Fmt::pct),
            N ("wrate", "Wbl Rate", 0.05f, 8, 0.6f, Fmt::hzs, 0, true),

            N ("dtime", "Delay", 0.01f, 2, 0.45f, Fmt::s, 0, true),
            N ("dfb", "Feedback", 0, 0.95f, 0.4f, Fmt::pct),
            N ("dmix", "Dly Mix", 0, 1, 0.22f, Fmt::pct),
            N ("rsize", "Size", 0.5f, 12, 4, Fmt::s, 0, true),
            N ("rmix", "Verb Mix", 0, 1, 0.4f, Fmt::pct),
            C ("rrev", "Reverb", 0, { "normal", "reverse" }, { "Normal", "Reverse" }),

            N ("torment", "Torment", 0, 1, 0, Fmt::pct),
            N ("crush", "Crush", 0, 1, 0, Fmt::pct),
            N ("master", "Volume", 0, 1, 0.7f, Fmt::pct),
        };
    }();
    return p;
}

inline int indexOf (const std::string& id)
{
    auto& p = params();
    for (int i = 0; i < NumParams; ++i)
        if (id == p[(size_t) i].id) return i;
    return -1;
}

// Macro routing, same as the web app: TORMENT and CRUSH push these parameters.
struct MacroRoute { int target; int macro; float amount; };
inline const std::vector<MacroRoute>& macroRoutes()
{
    static const std::vector<MacroRoute> m {
        { fm, torment, 0.6f }, { ring, torment, 0.45f }, { drift, torment, 90.0f }, { lpitch, torment, 120.0f },
        { warble, torment, 0.45f }, { o2fine, torment, 35.0f },
        { drive, crush, 0.85f }, { noise, crush, 0.3f }, { bits, crush, -13.0f },
    };
    return m;
}

inline std::string formatValue (int index, float v)
{
    auto& d = params()[(size_t) index];
    char buf[48];
    auto num = [&] (const char* f, double x) { std::snprintf (buf, sizeof buf, f, x); return std::string (buf); };
    switch (d.fmt)
    {
        case Fmt::choice: { auto i = (size_t) std::lround (v); return i < d.choiceNames.size() ? d.choiceNames[i] : std::string(); }
        case Fmt::pct:   return num ("%.0f%%", v * 100.0);
        case Fmt::hz:    return v >= 1000 ? num ("%.2f kHz", v / 1000.0) : num ("%.0f Hz", v);
        case Fmt::hzs:   return v < 1 ? num ("%.2f Hz", v) : num ("%.1f Hz", v);
        case Fmt::s:     return v < 1 ? num ("%.0f ms", v * 1000.0) : num ("%.2f s", v);
        case Fmt::st:    return num ("%+.0f st", v);
        case Fmt::ct:    return num ("%.0f ct", v);
        case Fmt::oct:   return num ("%+.1f oct", v);
        case Fmt::f1:    return num ("%.1f", v);
        case Fmt::intg:  return num ("%+.0f", v);
        case Fmt::bits:  return v >= 16 ? std::string ("Clean") : num ("%.0f-bit", v);
        case Fmt::vowel:
        {
            static const char* names[] = { "A", "E", "I", "O", "U" };
            auto i = (int) std::floor (v); float f = v - (float) i;
            if (i >= 4) return "U";
            if (f < 0.15f) return names[i];
            if (f > 0.85f) return names[i + 1];
            return std::string (names[i]) + ">" + names[i + 1];
        }
    }
    return {};
}
} // namespace heresy
