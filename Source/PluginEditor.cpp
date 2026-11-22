#include "PluginEditor.h"
#include "FactoryPresets.h"
#include "BinaryData.h"

using namespace heresy;
using namespace heresy::ui;

// ================================================================ fonts
Fonts& Fonts::get()
{
    static Fonts f = []
    {
        Fonts x;
        x.anton = juce::Typeface::createSystemTypefaceFor (BinaryData::AntonRegular_ttf, BinaryData::AntonRegular_ttfSize);
        x.oswald = juce::Typeface::createSystemTypefaceFor (BinaryData::OswaldVariable_ttf, BinaryData::OswaldVariable_ttfSize);
        x.courier = juce::Typeface::createSystemTypefaceFor (BinaryData::CourierPrimeBold_ttf, BinaryData::CourierPrimeBold_ttfSize);
        return x;
    }();
    return f;
}

static juce::Font fontFrom (const juce::Typeface::Ptr& tf, float h)
{
    if (tf != nullptr) return juce::Font (juce::FontOptions (tf).withHeight (h));
    return juce::Font (juce::FontOptions (h));
}
juce::Font Fonts::title (float h) const { return fontFrom (anton, h); }
juce::Font Fonts::label (float h) const { return fontFrom (oswald, h); }
juce::Font Fonts::mono (float h) const { return fontFrom (courier, h); }

static void drawSpaced (juce::Graphics& g, const juce::String& s, juce::Font f, juce::Rectangle<int> r,
                        juce::Justification j, float kerning = 0.12f)
{
    g.setFont (f.withExtraKerningFactor (kerning));
    g.drawFittedText (s, r, j, 1, 0.8f);
}

// ================================================================ look and feel
LookAndFeel::LookAndFeel()
{
    using namespace colours;
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff141010));
    setColour (juce::PopupMenu::textColourId, bone);
    setColour (juce::PopupMenu::headerTextColourId, red);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, redDim);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::ComboBox::textColourId, bone);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff181312));
    setColour (juce::ComboBox::outlineColourId, edge);
    setColour (juce::ComboBox::arrowColourId, red);
    setColour (juce::TextButton::textColourOffId, bone);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::TextButton::buttonColourId, iron);
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff141010));
    setColour (juce::AlertWindow::textColourId, bone);
    setColour (juce::AlertWindow::outlineColourId, edge);
    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0d0a0a));
    setColour (juce::TextEditor::textColourId, bone);
    setColour (juce::TextEditor::outlineColourId, edge);
    setColour (juce::TextEditor::focusedOutlineColourId, red);
    setColour (juce::TextEditor::highlightColourId, redDim);
    setColour (juce::CaretComponent::caretColourId, red);
    setColour (juce::Label::textColourId, bone);
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&)
{
    using namespace colours;
    const auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float r = std::min (area.getWidth(), area.getHeight()) * 0.5f;
    const auto c = area.getCentre();
    const float angle = start + pos * (end - start);
    const float track = std::max (2.0f, r * 0.11f);
    const float arcR = r - track * 0.5f;

    // track and value arc
    juce::Path bgArc;
    bgArc.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, start, end, true);
    g.setColour (juce::Colour (0xff221a18));
    g.strokePath (bgArc, juce::PathStrokeType (track, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path valArc;
    valArc.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, start, angle, true);
    g.setColour (red);
    g.strokePath (valArc, juce::PathStrokeType (track, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // body
    const float br = r - track * 1.9f;
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (c.x - br, c.y - br + br * 0.08f, br * 2, br * 2);
    juce::ColourGradient body (juce::Colour (0xff3d3330), c.x - br * 0.5f, c.y - br * 0.7f,
                               juce::Colour (0xff100c0b), c.x + br * 0.6f, c.y + br * 0.8f, false);
    g.setGradientFill (body);
    g.fillEllipse (c.x - br, c.y - br, br * 2, br * 2);
    g.setColour (juce::Colour (0xff4a3d38).withAlpha (0.7f));
    g.drawEllipse (c.x - br, c.y - br, br * 2, br * 2, 1.0f);

    // pointer
    const auto p1 = c.getPointOnCircumference (br * 0.25f, angle);
    const auto p2 = c.getPointOnCircumference (br * 0.85f, angle);
    g.setColour (bone);
    g.drawLine ({ p1, p2 }, std::max (1.6f, br * 0.12f));
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox& box)
{
    using namespace colours;
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (down || box.isMouseOver (true) ? red : edge);
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 12.0f, ay = (float) h * 0.5f;
    arrow.startNewSubPath (ax - 4, ay - 2);
    arrow.lineTo (ax, ay + 2);
    arrow.lineTo (ax + 4, ay - 2);
    g.setColour (red);
    g.strokePath (arrow, juce::PathStrokeType (1.6f));
}

juce::Font LookAndFeel::getComboBoxFont (juce::ComboBox& b) { return Fonts::get().label (std::min (15.0f, (float) b.getHeight() * 0.62f)); }

void LookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 1, box.getWidth() - 22, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

juce::Font LookAndFeel::getPopupMenuFont() { return Fonts::get().label (15.0f); }

void LookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
    g.setColour (colours::edge);
    g.drawRect (0, 0, w, h);
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    using namespace colours;
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    juce::ColourGradient grad (juce::Colour (0xff2e2522), 0, r.getY(), juce::Colour (0xff171211), 0, r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 3.0f);
    if (down) { g.setColour (redDim); g.fillRoundedRectangle (r, 3.0f); }
    g.setColour (over ? red : edge);
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
}

juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int h) { return Fonts::get().label (std::min (15.0f, (float) h * 0.55f)).withExtraKerningFactor (0.1f); }
juce::Font LookAndFeel::getAlertWindowTitleFont() { return Fonts::get().title (22.0f); }
juce::Font LookAndFeel::getAlertWindowMessageFont() { return Fonts::get().label (15.0f); }

// ================================================================ knob
Knob::Knob (juce::AudioProcessorValueTreeState& s, int i, bool b) : index (i), big (b)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setMouseDragSensitivity (big ? 260 : 200);
    slider.setVelocityBasedMode (false);
    slider.setScrollWheelEnabled (true);
    addAndMakeVisible (slider);
    attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, params()[(size_t) i].id, slider);
    slider.setDoubleClickReturnValue (true, params()[(size_t) i].def);
    slider.addMouseListener (this, false);
    slider.onDragStart = [this] { dragging = true; repaint(); };
    slider.onDragEnd = [this] { dragging = false; repaint(); };
    slider.onValueChange = [this] { if (hover || dragging) repaint(); };
    slider.setTooltip (HeresyProcessor::hostName (i));
}

void Knob::resized()
{
    auto r = getLocalBounds();
    const int labelH = big ? 22 : 15;
    auto k = r.removeFromTop (r.getHeight() - labelH);
    const int s = std::min (k.getWidth(), k.getHeight());
    slider.setBounds (k.withSizeKeepingCentre (s, s));
}

void Knob::paint (juce::Graphics& g)
{
    const int labelH = big ? 22 : 15;
    auto r = getLocalBounds().removeFromBottom (labelH);
    const bool showValue = hover || dragging;
    juce::String text;
    if (showValue) text = formatValue (index, (float) slider.getValue());
    else text = big ? juce::String (params()[(size_t) index].label).toUpperCase() : juce::String (params()[(size_t) index].label).toUpperCase();

    g.setColour (showValue ? colours::bone : (big ? colours::red : colours::ash));
    if (showValue) { g.setFont (Fonts::get().mono (big ? 15.0f : 12.0f)); g.drawFittedText (text, r, juce::Justification::centred, 1, 0.7f); }
    else drawSpaced (g, text, big ? Fonts::get().title (19.0f) : Fonts::get().label (12.5f), r, juce::Justification::centred, big ? 0.08f : 0.06f);
}

// ================================================================ choice
Choice::Choice (juce::AudioProcessorValueTreeState& s, int i) : index (i)
{
    juce::StringArray names;
    for (auto& n : params()[(size_t) i].choiceNames) names.add (n);
    box.addItemList (names, 1);
    box.setTooltip (HeresyProcessor::hostName (i));
    addAndMakeVisible (box);
    attach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (s, params()[(size_t) i].id, box);
}

void Choice::resized()
{
    auto r = getLocalBounds().reduced (3, 0);
    const int h = 24;
    box.setBounds (r.withSizeKeepingCentre (r.getWidth(), h).translated (0, -6));
}

void Choice::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().removeFromBottom (15);
    g.setColour (colours::ash);
    drawSpaced (g, juce::String (params()[(size_t) index].label).toUpperCase(), Fonts::get().label (12.5f), r, juce::Justification::centred, 0.06f);
}

// ================================================================ panels
static void paintPanel (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title)
{
    using namespace colours;
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (r.translated (0, 2), 5.0f);
    g.setColour (panel);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (edge);
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
    if (title.isNotEmpty())
    {
        g.setColour (red);
        drawSpaced (g, title.toUpperCase(), Fonts::get().label (15.0f), r.toNearestInt().reduced (12, 0).withHeight (28).withY ((int) r.getY() + 3),
                    juce::Justification::centredLeft, 0.22f);
        g.setColour (red.withAlpha (0.55f));
        g.fillRect (r.getX() + 12, r.getY() + 28, 26.0f, 1.5f);
    }
}

void Panel::paint (juce::Graphics& g) { paintPanel (g, getLocalBounds().toFloat(), title); }

Section::Section (juce::String t, juce::AudioProcessorValueTreeState& s, std::vector<std::vector<int>> r)
    : title (std::move (t)), rows (std::move (r))
{
    for (auto& row : rows)
    {
        std::vector<juce::Component*> line;
        for (int i : row)
        {
            juce::Component* c = params()[(size_t) i].choiceIds.empty() ? (juce::Component*) new Knob (s, i)
                                                                         : (juce::Component*) new Choice (s, i);
            items.add (c);
            addAndMakeVisible (c);
            line.push_back (c);
        }
        grid.push_back (line);
    }
}

void Section::paint (juce::Graphics& g) { paintPanel (g, getLocalBounds().toFloat(), title); }

void Section::resized()
{
    auto area = getLocalBounds().reduced (8, 0).withTrimmedTop (34).withTrimmedBottom (8);
    auto weight = [this] (size_t r)
    {
        float w = 0;
        for (size_t c = 0; c < rows[r].size(); ++c) w += params()[(size_t) rows[r][c]].choiceIds.empty() ? 1.0f : 1.45f;
        return w;
    };
    float maxW = 0;
    for (size_t r = 0; r < rows.size(); ++r) maxW = std::max (maxW, weight (r));
    const float unit = std::min (72.0f, (float) area.getWidth() / std::max (1.0f, maxW));
    const int rowH = std::min (68, area.getHeight() / (int) std::max<size_t> (1, rows.size()));
    int y = area.getY() + (area.getHeight() - rowH * (int) rows.size()) / 2;
    for (size_t r = 0; r < rows.size(); ++r, y += rowH)
    {
        float x = (float) area.getX() + ((float) area.getWidth() - weight (r) * unit) * 0.5f;
        for (size_t c = 0; c < rows[r].size(); ++c)
        {
            const float w = (params()[(size_t) rows[r][c]].choiceIds.empty() ? 1.0f : 1.45f) * unit;
            grid[r][c]->setBounds ((int) x, y, (int) w, rowH);
            x += w;
        }
    }
}

// ================================================================ pad
Pad::Pad (juce::AudioProcessorValueTreeState& s, Spec sp) : apvts (s), spec (std::move (sp))
{
    setMouseCursor (juce::MouseCursor::CrosshairCursor);
    refresh();
}

juce::Rectangle<float> Pad::field() const
{
    return getLocalBounds().toFloat().withTrimmedTop (36).withTrimmedBottom (28).reduced (12, 0);
}

void Pad::refresh()
{
    const float nx = juce::jlimit (0.0f, 1.0f, spec.getX()), ny = juce::jlimit (0.0f, 1.0f, spec.getY());
    if (std::abs (nx - x) > 0.0005f || std::abs (ny - y) > 0.0005f || ! trail.empty())
    {
        x = nx; y = ny;
        if (! active && ! trail.empty()) trail.erase (trail.begin());
        repaint();
    }
}

void Pad::set (juce::Point<float> p)
{
    auto f = field();
    const float nx = juce::jlimit (0.0f, 1.0f, (p.x - f.getX()) / f.getWidth());
    const float ny = juce::jlimit (0.0f, 1.0f, 1.0f - (p.y - f.getY()) / f.getHeight());
    spec.setX (nx);
    spec.setY (ny);
    trail.push_back ({ nx, ny });
    if (trail.size() > 18) trail.erase (trail.begin());
    x = nx; y = ny;
    repaint();
}

void Pad::mouseDown (const juce::MouseEvent& e)
{
    active = true;
    for (int i : spec.touches) if (auto* p = apvts.getParameter (params()[(size_t) i].id)) p->beginChangeGesture();
    set (e.position);
}
void Pad::mouseDrag (const juce::MouseEvent& e) { set (e.position); }
void Pad::mouseUp (const juce::MouseEvent&)
{
    active = false;
    for (int i : spec.touches) if (auto* p = apvts.getParameter (params()[(size_t) i].id)) p->endChangeGesture();
    repaint();
}

void Pad::paint (juce::Graphics& g)
{
    using namespace colours;
    paintPanel (g, getLocalBounds().toFloat(), spec.name);
    auto f = field();

    g.setColour (juce::Colour (0xff080606));
    g.fillRoundedRectangle (f, 4.0f);
    g.setColour (juce::Colour (0xff201816));
    for (int i = 1; i < 6; ++i)
    {
        const float gx = f.getX() + f.getWidth() * (float) i / 6.0f, gy = f.getY() + f.getHeight() * (float) i / 6.0f;
        g.drawVerticalLine ((int) gx, f.getY(), f.getBottom());
        g.drawHorizontalLine ((int) gy, f.getX(), f.getRight());
    }
    g.setColour (active ? red.withAlpha (0.8f) : edge);
    g.drawRoundedRectangle (f, 4.0f, 1.0f);

    auto toPt = [&f] (float nx, float ny) { return juce::Point<float> (f.getX() + nx * f.getWidth(), f.getBottom() - ny * f.getHeight()); };
    for (size_t i = 0; i < trail.size(); ++i)
    {
        const float a = (float) (i + 1) / (float) (trail.size() + 1);
        const auto p = toPt (trail[i].x, trail[i].y);
        g.setColour (red.withAlpha (0.25f * a));
        g.fillEllipse (p.x - 5 * a, p.y - 5 * a, 10 * a, 10 * a);
    }

    const auto p = toPt (x, y);
    g.setColour (red.withAlpha (0.18f));
    g.drawVerticalLine ((int) p.x, f.getY(), f.getBottom());
    g.drawHorizontalLine ((int) p.y, f.getX(), f.getRight());
    juce::ColourGradient glow (red.withAlpha (active ? 0.7f : 0.45f), p.x, p.y, red.withAlpha (0.0f), p.x + 26, p.y, true);
    g.setGradientFill (glow);
    g.fillEllipse (p.x - 26, p.y - 26, 52, 52);
    g.setColour (bone);
    g.fillEllipse (p.x - 5, p.y - 5, 10, 10);
    g.setColour (red);
    g.drawEllipse (p.x - 7, p.y - 7, 14, 14, 1.5f);

    auto lab = getLocalBounds().removeFromBottom (26).reduced (14, 0);
    g.setColour (ash);
    drawSpaced (g, juce::String::fromUTF8 ("X \xc2\xb7 ") + spec.xLabel().toUpperCase(), Fonts::get().label (12.5f), lab, juce::Justification::centredLeft, 0.1f);
    drawSpaced (g, juce::String::fromUTF8 ("Y \xc2\xb7 ") + spec.yLabel().toUpperCase(), Fonts::get().label (12.5f), lab, juce::Justification::centredRight, 0.1f);
}

// ================================================================ editor
static juce::Image makeTexture (int w, int h)
{
    juce::Image img (juce::Image::ARGB, w, h, true);
    juce::Graphics g (img);
    g.fillAll (colours::bg);
    juce::Random r (666);

    // smoke
    for (int i = 0; i < 26; ++i)
    {
        const float cx = r.nextFloat() * (float) w, cy = r.nextFloat() * (float) h, rad = 120.0f + r.nextFloat() * 260.0f;
        juce::ColourGradient smoke (juce::Colour (0xff2a2322).withAlpha (0.10f + r.nextFloat() * 0.10f), cx, cy,
                                    juce::Colour (0xff2a2322).withAlpha (0.0f), cx + rad, cy, true);
        g.setGradientFill (smoke);
        g.fillEllipse (cx - rad, cy - rad, rad * 2, rad * 2);
    }
    // rust streaks
    for (int i = 0; i < 900; ++i)
    {
        const float x = r.nextFloat() * (float) w, y = r.nextFloat() * (float) h, len = 10.0f + r.nextFloat() * 90.0f;
        g.setColour (juce::Colour (0xff8c2a1e).withAlpha (0.03f + r.nextFloat() * 0.08f));
        g.drawLine (x, y, x + len * 0.05f, y + len, 1.0f);
    }
    // grain
    for (int i = 0; i < w * h / 18; ++i)
    {
        g.setColour (juce::Colours::white.withAlpha (r.nextFloat() * 0.035f));
        g.fillRect (r.nextInt (w), r.nextInt (h), 1, 1);
    }
    // vignette
    juce::ColourGradient vig (juce::Colours::transparentBlack, (float) w * 0.5f, (float) h * 0.45f,
                              juce::Colours::black.withAlpha (0.75f), 0.0f, 0.0f, true);
    g.setGradientFill (vig);
    g.fillRect (0, 0, w, h);
    return img;
}

HeresyEditor::HeresyEditor (HeresyProcessor& p)
    : AudioProcessorEditor (p), proc (p), keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    texture = makeTexture (baseW, baseH);

    content.setInterceptsMouseClicks (false, true);
    addAndMakeVisible (content);
    auto& s = proc.apvts;

    // header: presets
    for (auto* b : { &prevBtn, &nextBtn, &saveBtn, &deleteBtn, &panicBtn }) content.addAndMakeVisible (*b);
    content.addAndMakeVisible (presetBox);
    presetBox.setTextWhenNothingSelected ("Init");
    presetBox.onChange = [this] { if (presetBox.getSelectedId() > 0) choosePreset (presetBox.getSelectedId()); };
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };
    saveBtn.onClick = [this] { savePreset(); };
    deleteBtn.onClick = [this] { deletePreset(); };
    panicBtn.onClick = [this] { proc.panic(); };
    panicBtn.setColour (juce::TextButton::textColourOffId, colours::red);
    prevBtn.setTooltip ("Previous preset");
    nextBtn.setTooltip ("Next preset");
    panicBtn.setTooltip ("Stop every sound");

    // macros
    content.addAndMakeVisible (macroPanel);
    for (int id : { (int) torment, (int) crush, (int) master })
        content.addAndMakeVisible (macros.add (new Knob (s, id, true)));

    // touch pads
    auto P = [&s] (int i) { return s.getParameter (params()[(size_t) i].id); };
    auto get = [P] (int i) { auto* q = P (i); return q->convertFrom0to1 (q->getValue()); };
    auto setV = [P] (int i, float v) { auto* q = P (i); q->setValueNotifyingHost (q->convertTo0to1 (v)); };
    auto norm = [P] (int i) { return P (i)->getValue(); };
    auto setN = [P] (int i, float v) { P (i)->setValueNotifyingHost (v); };
    auto isFormant = [get] { return (int) std::lround (get (ftype)) == Formant; };

    pads.add (new Pad (s, { "Fear",
        [isFormant] { return juce::String (isFormant() ? "Vowel" : "Cutoff"); }, [] { return juce::String ("Torment"); },
        [=] { return isFormant() ? get (vowel) / 4.0f : norm (cutoff); }, [=] { return get (torment); },
        [=] (float v) { if (isFormant()) setV (vowel, v * 4.0f); else setN (cutoff, v); }, [=] (float v) { setV (torment, v); },
        { cutoff, vowel, torment } }));
    pads.add (new Pad (s, { "Space",
        [] { return juce::String ("Reverb"); }, [] { return juce::String ("Echo"); },
        [=] { return get (rmix); }, [=] { return juce::jlimit (0.0f, 1.0f, get (dmix) / 0.55f); },
        [=] (float v) { setV (rmix, v); }, [=] (float v) { setV (dmix, v * 0.55f); setV (dfb, v * 0.82f); },
        { rmix, dmix, dfb } }));
    pads.add (new Pad (s, { "Motion",
        [] { return juce::String ("Speed"); }, [] { return juce::String ("Depth"); },
        [=] { return norm (lrate); }, [=] { return juce::jlimit (0.0f, 1.0f, get (lfilt) / 3600.0f); },
        [=] (float v) { setN (lrate, v); }, [=] (float v) { setV (lfilt, v * 3600.0f); setV (lpitch, v * v * 240.0f); setV (lamp, v * 0.3f); },
        { lrate, lfilt, lpitch, lamp } }));
    for (auto* pad : pads) content.addAndMakeVisible (pad);

    // sections (same groups as the web app's Advanced panel)
    sections.add (new Section ("Oscillators", s, { { o1wave, o1oct, o1lvl, sub }, { o2wave, o2semi, o2fine, o2lvl } }));
    sections.add (new Section ("Dissonance", s, { { noiseColor, noise, drift }, { fm, ring } }));
    sections.add (new Section ("Pitch", s, { { voicing }, { penv, ptime } }));
    sections.add (new Section ("Filter", s, { { ftype, cutoff, reso, vowel }, { fenv, fatk, fdec } }));
    sections.add (new Section ("Amp Envelope", s, { { atk, dec }, { sus, rel } }));
    sections.add (new Section ("LFO", s, { { lshape, lrate }, { lpitch, lfilt, lamp } }));
    sections.add (new Section ("Damage", s, { { drive, bits }, { warble, wrate } }));
    sections.add (new Section ("Space", s, { { rrev, dtime, dfb, dmix }, { rsize, rmix } }));
    for (auto* sec : sections) content.addAndMakeVisible (sec);

    // keyboard
    keyboard.setAvailableRange (24, 96);
    keyboard.setScrollButtonsVisible (false);
    keyboard.setWantsKeyboardFocus (false);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, colours::bone);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff161111));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0x66000000));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, colours::red.withAlpha (0.25f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, colours::red.withAlpha (0.8f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::transparentBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, juce::Colour (0xff5a4c46));
    content.addAndMakeVisible (keyboard);

    layoutContent();
    refreshPresetList();

    setResizable (true, true);
    setResizeLimits (840, 560, 1800, 1200);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize (1080, 720);
    startTimerHz (30);
}

HeresyEditor::~HeresyEditor()
{
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void HeresyEditor::layoutContent()
{
    content.setBounds (0, 0, baseW, baseH);
    prevBtn.setBounds (440, 24, 30, 30);
    presetBox.setBounds (476, 24, 250, 30);
    nextBtn.setBounds (732, 24, 30, 30);
    saveBtn.setBounds (772, 24, 62, 30);
    deleteBtn.setBounds (840, 24, 68, 30);
    panicBtn.setBounds (918, 24, 66, 30);

    macroPanel.setBounds (16, 88, 300, 236);
    for (int i = 0; i < macros.size(); ++i) macros[i]->setBounds (22 + i * 96, 140, 96, 150);

    for (int i = 0; i < pads.size(); ++i) pads[i]->setBounds (328 + i * 289, 88, 277, 236);

    for (int i = 0; i < sections.size(); ++i)
        sections[i]->setBounds (16 + (i % 4) * 295, i < 4 ? 336 : 518, 283, 170);

    keyboard.setBounds (16, 700, 1168, 84);
    keyboard.setKeyWidth (1168.0f / 43.0f);
}

void HeresyEditor::resized()
{
    const float scale = (float) getWidth() / (float) baseW;
    content.setTransform (juce::AffineTransform::scale (scale));
}

void HeresyEditor::paint (juce::Graphics& g)
{
    const float scale = (float) getWidth() / (float) baseW;
    g.addTransform (juce::AffineTransform::scale (scale));
    g.drawImageAt (texture, 0, 0);

    using namespace colours;
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.setFont (Fonts::get().title (54.0f).withExtraKerningFactor (0.04f));
    g.drawText ("HERESY", 26, 9, 300, 56, juce::Justification::centredLeft);
    g.setColour (red);
    g.drawText ("HERESY", 24, 7, 300, 56, juce::Justification::centredLeft);
    g.setColour (ash);
    drawSpaced (g, "HORROR SOUND DESIGN SYNTH", Fonts::get().label (12.5f), { 26, 60, 300, 18 }, juce::Justification::centredLeft, 0.25f);

    g.setColour (bone);
    drawSpaced (g, "CASTLE DANGER", Fonts::get().title (22.0f), { 990, 18, 194, 26 }, juce::Justification::centredRight, 0.06f);
    g.setColour (ash);
    drawSpaced (g, "MUSIC COMPANY", Fonts::get().label (12.5f), { 990, 44, 194, 16 }, juce::Justification::centredRight, 0.3f);

    g.setColour (red.withAlpha (0.35f));
    g.fillRect (16.0f, 80.0f, 1168.0f, 1.0f);
}

// ---------------------------------------------------------------- presets UI
void HeresyEditor::refreshPresetList()
{
    userNames = PresetManager::userPresetNames();
    presetBox.clear (juce::dontSendNotification);
    presetBox.addSectionHeading ("Factory");
    const auto& f = factoryPresets();
    for (size_t i = 0; i < f.size(); ++i) presetBox.addItem (f[i].name, (int) i + 1);
    if (! userNames.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading ("User");
        for (int i = 0; i < userNames.size(); ++i) presetBox.addItem (userNames[i], 1001 + i);
    }

    const auto name = proc.getPresetName();
    int id = 0;
    if (proc.isUserPreset()) { const int i = userNames.indexOf (name); if (i >= 0) id = 1001 + i; }
    else for (size_t i = 0; i < f.size(); ++i) if (name == f[i].name) id = (int) i + 1;
    if (id > 0) presetBox.setSelectedId (id, juce::dontSendNotification);
    else presetBox.setText (name, juce::dontSendNotification);
    deleteBtn.setEnabled (proc.isUserPreset());
    lastPresetVersion = proc.presetVersion();
}

void HeresyEditor::choosePreset (int id)
{
    if (id >= 1001) proc.loadUserPreset (userNames[id - 1001]);
    else proc.loadFactoryPreset (id - 1);
}

void HeresyEditor::stepPreset (int delta)
{
    std::vector<int> ids;
    for (size_t i = 0; i < factoryPresets().size(); ++i) ids.push_back ((int) i + 1);
    for (int i = 0; i < userNames.size(); ++i) ids.push_back (1001 + i);
    const int cur = presetBox.getSelectedId();
    auto it = std::find (ids.begin(), ids.end(), cur);
    int idx = it == ids.end() ? 0 : (int) (it - ids.begin());
    idx = (idx + delta + (int) ids.size()) % (int) ids.size();
    choosePreset (ids[(size_t) idx]);
}

void HeresyEditor::savePreset()
{
    auto* w = new juce::AlertWindow ("Save preset", "Name this preset. It is saved for every project.", juce::MessageBoxIconType::NoIcon, this);
    w->setLookAndFeel (&lnf);
    w->addTextEditor ("name", proc.isUserPreset() ? proc.getPresetName() : juce::String(), "Name");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<HeresyEditor> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int result)
    {
        if (result != 1 || safe == nullptr) return;
        const auto name = w->getTextEditorContents ("name").trim();
        if (name.isNotEmpty() && safe->proc.saveUserPreset (name)) safe->refreshPresetList();
    }), true);
}

void HeresyEditor::deletePreset()
{
    if (! proc.isUserPreset()) return;
    const auto name = proc.getPresetName();
    juce::Component::SafePointer<HeresyEditor> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::QuestionIcon)
                                      .withTitle ("Delete preset")
                                      .withMessage ("Delete \"" + name + "\"? This can't be undone.")
                                      .withButton ("Delete")
                                      .withButton ("Cancel")
                                      .withAssociatedComponent (this),
                                  [safe, name] (int result)
                                  {
                                      if (result == 1 && safe != nullptr && safe->proc.deleteUserPreset (name))
                                          safe->refreshPresetList();
                                  });
}

void HeresyEditor::timerCallback()
{
    for (auto* pad : pads) pad->refresh();
    if (proc.presetVersion() != lastPresetVersion) refreshPresetList();
}
