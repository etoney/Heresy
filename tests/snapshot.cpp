// Renders the plugin window to a PNG so the UI can be checked on CI without a DAW.
// Usage: HeresySnapshot <out.png> [presetIndex]
#include "PluginProcessor.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    juce::File out (argc > 1 ? juce::String (argv[1]) : juce::String ("heresy-ui.png"));
    const int preset = argc > 2 ? juce::String (argv[2]).getIntValue() : 1;

    auto proc = std::make_unique<HeresyProcessor>();
    proc->prepareToPlay (48000.0, 512);
    proc->setCurrentProgram (preset);

    std::unique_ptr<juce::AudioProcessorEditor> editor (proc->createEditor());
    editor->setSize (1200, 862);
    auto img = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);

    out.deleteFile();
    juce::FileOutputStream stream (out);
    juce::PNGImageFormat png;
    const bool ok = stream.openedOk() && png.writeImageToStream (img, stream);
    editor.reset();
    proc.reset();
    std::printf ("%s %s\n", ok ? "wrote" : "FAILED", out.getFullPathName().toRawUTF8());
    return ok ? 0 : 1;
}
