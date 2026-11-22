// Heresy — factory presets, matching Heresy web app v1.3.0.
// Each preset starts from the defaults and overrides the listed parameters.
#pragma once

#include "ParamDefs.h"

namespace heresy
{
struct FactoryPreset
{
    const char* name;
    const char* description;
    const char* values;   // "id:value id:value ..." (choice params use their ids)
};

inline const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> f {
        { "Init", "Clean starting point.", "" },
        { "Abyssal Drone", "A slow, bottomless drone that opens up over time.",
          "voicing:abyss o1wave:sawtooth o1oct:0 o1lvl:0.6 o2wave:square o2semi:0 o2fine:9 o2lvl:0.35 sub:0.35 ring:0.08 noise:0.07 noiseColor:brown drift:18 ftype:lowpass cutoff:340 reso:7 fenv:1.2 fatk:4 fdec:8 lshape:sine lrate:0.07 lpitch:6 lfilt:700 lamp:0 atk:4 dec:3 sus:0.95 rel:9 drive:0.15 warble:0.25 wrate:0.12 dtime:0.9 dfb:0.45 dmix:0.18 rsize:10 rmix:0.6 torment:0.15 crush:0" },
        { "Jump Scare", "A dissonant stinger that hits hard and tears off.",
          "voicing:sting o1wave:sawtooth o2wave:square o2semi:1 o2fine:25 o2lvl:0.5 fm:0.25 ring:0.2 sub:0.2 noise:0.6 noiseColor:white penv:24 ptime:0.25 ftype:lowpass cutoff:9000 reso:4 fenv:0 atk:0.001 dec:0.9 sus:0 rel:1.6 drive:0.65 warble:0 dtime:0.12 dfb:0.3 dmix:0.15 rsize:6 rmix:0.45 torment:0.3 crush:0.25" },
        { "Ghost Whisper", "Breathy, wandering voices.",
          "o1lvl:0 o2lvl:0 sub:0 noise:0.95 noiseColor:pink ftype:formant vowel:1.2 reso:9 fenv:0 lshape:random lrate:2.2 lpitch:0 lfilt:900 lamp:0.25 atk:1.2 dec:1 sus:0.85 rel:4.5 drive:0.12 warble:0.5 wrate:0.4 dtime:0.6 dfb:0.35 dmix:0.2 rsize:8 rmix:0.65 torment:0 crush:0" },
        { "Hell Siren", "A slow air-raid wail from below.",
          "o1wave:sawtooth o2wave:sawtooth o2semi:12 o2fine:-10 o2lvl:0.45 sub:0.2 noise:0.04 ftype:bandpass cutoff:1400 reso:7 fenv:0 lshape:triangle lrate:0.18 lpitch:900 lfilt:900 lamp:0 atk:0.8 dec:0.5 sus:1 rel:3.5 drive:0.45 warble:0.15 dtime:0.38 dfb:0.55 dmix:0.28 rsize:7 rmix:0.45 torment:0.2 crush:0" },
        { "Haunted Music Box", "A warped, detuned lullaby.",
          "o1wave:triangle o1oct:1 o1lvl:0.7 o2wave:sine o2semi:24 o2fine:8 o2lvl:0.25 fm:0.08 sub:0 noise:0 drift:30 ftype:lowpass cutoff:6000 reso:1 fenv:0 atk:0.001 dec:1.6 sus:0 rel:3 lshape:sine lrate:4.5 lpitch:12 lfilt:0 lamp:0 drive:0.04 bits:9 warble:0.65 wrate:0.35 dtime:0.42 dfb:0.5 dmix:0.32 rsize:7 rmix:0.55 torment:0 crush:0" },
        { "Metal Scrape", "Grinding, inharmonic metal.",
          "o1wave:sine o1lvl:0.7 o2wave:sine o2semi:13 o2fine:37 o2lvl:0.05 fm:0.85 ring:0.5 sub:0 noise:0.05 noiseColor:white ftype:bandpass cutoff:2600 reso:14 fenv:0 lshape:random lrate:14 lpitch:30 lfilt:1500 lamp:0 atk:0.25 dec:1.2 sus:0.6 rel:2.5 drive:0.35 warble:0.3 wrate:0.8 rsize:6 rmix:0.5 torment:0.1 crush:0" },
        { "Possession Choir", "A diminished choir slowly sagging out of tune.",
          "voicing:dim o1wave:sawtooth o2wave:sawtooth o2semi:0 o2fine:14 o2lvl:0.5 sub:0 noise:0.06 noiseColor:pink ftype:formant vowel:3.2 reso:10 fenv:0 penv:-3 ptime:5 drift:30 lshape:sine lrate:0.18 lpitch:25 lfilt:400 lamp:0 atk:2.5 dec:1 sus:0.9 rel:6 drive:0.1 warble:0.35 wrate:5 dtime:0.5 dfb:0.3 dmix:0.1 rsize:9 rmix:0.65 torment:0.1 crush:0" },
        { "Reverse Spirits", "Plucks that swell backwards out of the dark.",
          "o1wave:triangle o2wave:sine o2semi:7 o2lvl:0.4 sub:0.1 noise:0.15 noiseColor:white drift:15 ftype:lowpass cutoff:3000 reso:2 fenv:1.5 fatk:0.005 fdec:0.4 atk:0.002 dec:0.7 sus:0 rel:1.2 drive:0.05 warble:0.2 rrev:reverse rsize:4 rmix:0.85 dtime:0.55 dfb:0.25 dmix:0.15 torment:0 crush:0" },
        { "Terror Riser", "A long, pulsing build of dread.",
          "voicing:tritone o1wave:sawtooth o2wave:sawtooth o2semi:1 o2fine:15 o2lvl:0.5 noise:0.45 noiseColor:white penv:-24 ptime:9 ftype:lowpass cutoff:250 reso:9 fenv:5.5 fatk:9 fdec:12 atk:7 dec:1 sus:1 rel:2.5 lshape:square lrate:7 lpitch:0 lfilt:0 lamp:0.35 drive:0.5 warble:0.1 rsize:6 rmix:0.5 torment:0.25 crush:0" },
        { "Heartbeat Below", "A deep, thumping pulse. Play two quick notes.",
          "o1wave:sine o1oct:-2 o1lvl:0.8 sub:0.9 o2lvl:0 noise:0 penv:14 ptime:0.06 ftype:lowpass cutoff:180 reso:1 fenv:1 fatk:0.001 fdec:0.08 atk:0.002 dec:0.22 sus:0 rel:0.35 lfilt:0 lpitch:0 lamp:0 drive:0.55 warble:0 dmix:0 rsize:2 rmix:0.15 torment:0 crush:0" },
        { "Bit Rot Static", "Broken, crackling electronics.",
          "o1wave:square o2wave:square o2semi:6 o2lvl:0.4 noise:0.55 noiseColor:brown drift:40 bits:5 ftype:lowpass cutoff:2600 reso:11 fenv:0 lshape:random lrate:11 lpitch:0 lfilt:2400 lamp:0.4 atk:0.05 dec:0.5 sus:0.8 rel:1.2 drive:0.3 warble:0.2 rsize:4 rmix:0.3 torment:0 crush:0.35" },
        { "Wraith Wind", "Howling, resonant wind.",
          "o1lvl:0 o2lvl:0 sub:0 noise:1 noiseColor:pink ftype:bandpass cutoff:650 reso:15 fenv:0 lshape:sine lrate:0.1 lpitch:0 lfilt:2000 lamp:0.25 atk:2.5 dec:1 sus:1 rel:7 drive:0.16 warble:0.3 wrate:0.3 dtime:0.7 dfb:0.3 dmix:0.12 rsize:11 rmix:0.6 torment:0 crush:0" },
        { "Cellar Organ", "A wheezing pipe organ in a damp cellar.",
          "voicing:octave o1wave:square o2wave:sawtooth o2semi:7 o2fine:3 o2lvl:0.35 sub:0.35 noise:0.03 ftype:lowpass cutoff:2000 reso:2 fenv:0.3 atk:0.06 dec:0.4 sus:0.9 rel:2.5 lshape:sine lrate:6 lpitch:10 lfilt:0 lamp:0.15 drive:0.25 warble:0.4 wrate:0.25 rsize:10 rmix:0.55 torment:0.08 crush:0" },
        { "Drowned Bell", "A sunken bell tolling under water.",
          "o1wave:sine o2wave:sine o2semi:19 o2fine:-25 o2lvl:0.3 fm:0.5 ring:0.2 sub:0 noise:0 drift:25 ftype:lowpass cutoff:3500 reso:3 fenv:-2 fatk:0.001 fdec:3 atk:0.002 dec:5 sus:0 rel:7 lshape:sine lrate:0.3 lpitch:0 lfilt:600 lamp:0 drive:0 warble:0.55 wrate:0.2 dtime:0.8 dfb:0.55 dmix:0.3 rsize:11 rmix:0.7 torment:0 crush:0" },
    };
    return f;
}

// Locale-independent number parsing (hosts may set a locale that uses decimal commas).
inline float parseNumber (const std::string& s)
{
    double sign = 1, val = 0, scale = 0;
    for (char c : s)
    {
        if (c == '-') sign = -1;
        else if (c == '.') scale = 1;
        else if (c >= '0' && c <= '9')
        {
            if (scale > 0) { scale *= 0.1; val += (c - '0') * scale; }
            else val = val * 10 + (c - '0');
        }
    }
    return (float) (sign * val);
}

// Full parameter values (real units, choices as indices) for a factory preset.
inline std::array<float, NumParams> factoryValues (size_t presetIndex)
{
    std::array<float, NumParams> v {};
    auto& p = params();
    for (size_t i = 0; i < (size_t) NumParams; ++i) v[i] = p[i].def;

    std::string s = factoryPresets()[presetIndex].values;
    size_t pos = 0;
    while (pos < s.size())
    {
        auto end = s.find (' ', pos);
        if (end == std::string::npos) end = s.size();
        auto tok = s.substr (pos, end - pos);
        pos = end + 1;
        auto colon = tok.find (':');
        if (colon == std::string::npos) continue;
        auto id = tok.substr (0, colon), val = tok.substr (colon + 1);
        int idx = indexOf (id);
        if (idx < 0) continue;
        auto& d = p[(size_t) idx];
        if (! d.choiceIds.empty())
        {
            for (size_t c = 0; c < d.choiceIds.size(); ++c)
                if (d.choiceIds[c] == val) v[(size_t) idx] = (float) c;
        }
        else
        {
            v[(size_t) idx] = std::fmin (d.max, std::fmax (d.min, parseNumber (val)));
        }
    }
    return v;
}
} // namespace heresy
