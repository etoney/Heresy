#pragma once

#include "PluginProcessor.h"

namespace heresy::ui
{
struct Fonts
{
    juce::Typeface::Ptr anton, oswald, courier;
    static Fonts& get();
    juce::Font title (float h) const;
    juce::Font label (float h) const;
    juce::Font mono (float h) const;
};

namespace colours
{
    const juce::Colour bg { 0xff0b0909 };
    const juce::Colour panel { 0xe6130f0e };
    const juce::Colour edge { 0xff3b2622 };
    const juce::Colour red { 0xffd23a2e };
    const juce::Colour redDim { 0xff7a1f19 };
    const juce::Colour bone { 0xffe8dfd0 };
    const juce::Colour ash { 0xff9a8f86 };
    const juce::Colour iron { 0xff2a2220 };
}

class LookAndFeel : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int h) override;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
};

// A knob with its name underneath; the name turns into the value while hovered or dragged.
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState&, int paramIndex, bool big = false);
    void resized() override;
    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }

private:
    int index; bool big; bool hover = false, dragging = false;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

class Choice : public juce::Component
{
public:
    Choice (juce::AudioProcessorValueTreeState&, int paramIndex);
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    int index;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attach;
};

class Section : public juce::Component
{
public:
    Section (juce::String title, juce::AudioProcessorValueTreeState&, std::vector<std::vector<int>> rows);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    juce::String title;
    std::vector<std::vector<int>> rows;
    juce::OwnedArray<juce::Component> items;
    std::vector<std::vector<juce::Component*>> grid;
};

// XY touch pad, as in the web app (Fear, Space, Motion).
class Pad : public juce::Component
{
public:
    struct Spec
    {
        juce::String name;
        std::function<juce::String()> xLabel, yLabel;
        std::function<float()> getX, getY;
        std::function<void (float)> setX, setY;
        std::vector<int> touches;   // parameters a drag changes (for host gestures)
    };
    Pad (juce::AudioProcessorValueTreeState&, Spec);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void refresh();
private:
    juce::Rectangle<float> field() const;
    void set (juce::Point<float>);
    juce::AudioProcessorValueTreeState& apvts;
    Spec spec;
    float x = 0, y = 0;
    bool active = false;
    std::vector<juce::Point<float>> trail;
};

// Plugin header in the shared Castle Danger layout: screen, dripping title, crest plate, hazard stripe.
class Header : public juce::Component, private juce::Timer
{
public:
    Header();
    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr int height = 112;
    juce::Rectangle<int> presetArea() const { return { 900, 14, 284, 76 }; }
private:
    void timerCallback() override;
    void paintScreen (juce::Graphics&, juce::Rectangle<float>);
    void paintTitle (juce::Graphics&);
    void paintPlate (juce::Graphics&, juce::Rectangle<float>);
    juce::Rectangle<float> screenR { 16, 12, 250, 84 }, titleR { 288, 12, 330, 62 }, plateR { 636, 14, 248, 78 };
    juce::Path titlePath;
    std::vector<juce::Path> drips;
    float phase = 0;
};

class Panel : public juce::Component
{
public:
    explicit Panel (juce::String t) : title (std::move (t)) { setInterceptsMouseClicks (false, true); }
    void paint (juce::Graphics&) override;
    juce::String title;
};
} // namespace heresy::ui

class HeresyEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit HeresyEditor (HeresyProcessor&);
    ~HeresyEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutContent();
    void refreshPresetList();
    void choosePreset (int itemId);
    void stepPreset (int delta);
    void savePreset();
    void deletePreset();

    HeresyProcessor& proc;
    heresy::ui::LookAndFeel lnf;
    juce::Component content;
    juce::Image texture;
    heresy::ui::Header header;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "Save" }, deleteBtn { "Delete" }, panicBtn { "Panic" };
    juce::StringArray userNames;
    int lastPresetVersion = -1;

    heresy::ui::Panel macroPanel { "Possession" };
    juce::OwnedArray<heresy::ui::Knob> macros;
    juce::OwnedArray<heresy::ui::Pad> pads;
    juce::OwnedArray<heresy::ui::Section> sections;
    juce::MidiKeyboardComponent keyboard;

    static constexpr int baseW = 1200, baseH = 830;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeresyEditor)
};
