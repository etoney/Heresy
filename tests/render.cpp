// Headless test: renders every factory preset through the engine and checks the audio.
// Usage: HeresyRender <outputFolder>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Engine.h"
#include "FactoryPresets.h"

using namespace heresy;

int main (int argc, char** argv)
{
    const double sr = 48000.0;
    const int block = 256;
    juce::File outDir (argc > 1 ? juce::String (argv[1]) : juce::File::getCurrentWorkingDirectory().getFullPathName());
    outDir.createDirectory();
    int failures = 0;

    for (size_t p = 0; p < factoryPresets().size(); ++p)
    {
        Engine e;
        auto v = factoryValues (p);
        e.setParameters (v);
        e.prepare (sr, block);
        juce::Thread::sleep (400);          // let the convolution load its impulse

        // a tiny warm-up so the reverb engine swaps in
        {
            juce::AudioBuffer<float> warm (2, block); juce::MidiBuffer none2;
            for (int i = 0; i < 4; ++i) { warm.clear(); e.process (warm, none2); }
            juce::Thread::sleep (100);
            for (int i = 0; i < 4; ++i) { warm.clear(); e.process (warm, none2); }
        }

        // play: a chord-ish phrase, note length depends on preset (2.5 s), then 4 s tail
        const int total = (int) (sr * 7.0);
        juce::AudioBuffer<float> out (2, total);
        out.clear();
        const int notes[] = { 48, 51 };
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            juce::MidiBuffer midi;
            auto at = [&] (int sample, const juce::MidiMessage& m) { if (sample >= pos && sample < pos + n) midi.addEvent (m, sample - pos); };
            at (0, juce::MidiMessage::noteOn (1, notes[0], 0.8f));
            at ((int) (sr * 0.6), juce::MidiMessage::noteOn (1, notes[1], 0.8f));
            at ((int) (sr * 3.0), juce::MidiMessage::noteOff (1, notes[0]));
            at ((int) (sr * 3.0), juce::MidiMessage::noteOff (1, notes[1]));
            juce::AudioBuffer<float> chunk (2, n);
            chunk.clear();
            e.process (chunk, midi);
            for (int ch = 0; ch < 2; ++ch) out.copyFrom (ch, pos, chunk, ch, 0, n);
        }

        float peak = 0; double sum = 0; bool bad = false;
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < total; ++i)
            {
                const float s = out.getSample (ch, i);
                if (! std::isfinite (s)) bad = true;
                peak = std::max (peak, std::abs (s));
                sum += (double) s * s;
            }
        const double rms = std::sqrt (sum / (2.0 * total));
        const float rmsDb = juce::Decibels::gainToDecibels ((float) rms, -120.0f);
        const float pkDb = juce::Decibels::gainToDecibels (peak, -120.0f);
        const bool silent = rmsDb < -50.0f, clipped = peak > 1.0f;
        if (bad || silent || clipped) ++failures;
        std::printf ("%-20s peak %6.1f dB  rms %6.1f dB %s%s%s\n", factoryPresets()[p].name, pkDb, rmsDb,
                     bad ? " NaN!" : "", silent ? " SILENT!" : "", clipped ? " CLIP!" : "");

        juce::WavAudioFormat wav;
        auto file = outDir.getChildFile (juce::String (factoryPresets()[p].name).replaceCharacter (' ', '_') + ".wav");
        file.deleteFile();
        if (auto stream = std::unique_ptr<juce::OutputStream> (file.createOutputStream()))
            if (auto w = std::unique_ptr<juce::AudioFormatWriter> (wav.createWriterFor (stream.get(), sr, 2, 24, {}, 0)))
            {
                stream.release();
                w->writeFromAudioSampleBuffer (out, 0, total);
            }
    }
    std::printf ("%d problem(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
