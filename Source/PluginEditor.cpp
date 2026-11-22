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

// ================================================================ header
static void paintRivet (juce::Graphics& g, float x, float y, float r)
{
    juce::ColourGradient rv (juce::Colour (0xffd8c3a2), x - r * 0.4f, y - r * 0.4f, juce::Colour (0xff4c3828), x + r, y + r, true);
    g.setGradientFill (rv);
    g.fillEllipse (x - r, y - r, r * 2, r * 2);
}

static void paintBezel (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (0, 3), 5.0f);
    juce::ColourGradient metal (juce::Colour (0xff4a3a30), r.getX(), r.getY(), juce::Colour (0xff1a110c), r.getRight(), r.getBottom(), false);
    g.setGradientFill (metal);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (juce::Colour (0xff0d0806));
    g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
    const float i = 5.0f;
    for (auto p : { r.getTopLeft(), r.getTopRight(), r.getBottomLeft(), r.getBottomRight() })
        paintRivet (g, p.x + (p.x < r.getCentreX() ? i : -i), p.y + (p.y < r.getCentreY() ? i : -i), 3.2f);
}

Header::Header()
{
    setInterceptsMouseClicks (false, false);
    startTimerHz (30);
}

void Header::timerCallback()
{
    phase += 1.0f / 30.0f;
    repaint (screenR.toNearestInt().expanded (2));
}

void Header::resized()
{
    // title as a path, so it can be textured and dripped like the other Castle Danger plugins
    juce::GlyphArrangement ga;
    ga.addLineOfText (Fonts::get().title (100.0f), "HERESY", 0, 0);
    titlePath.clear();
    ga.createPath (titlePath);
    titlePath.applyTransform (titlePath.getTransformToScaleToFit (titleR, false));   // stretched like the other plugin titles

    drips.clear();
    juce::Random r (31);
    const auto tb = titlePath.getBounds();
    int made = 0;
    for (int tries = 0; tries < 400 && made < 11; ++tries)
    {
        const float x = tb.getX() + r.nextFloat() * tb.getWidth();
        const float y = tb.getBottom() - 2.0f;
        if (! titlePath.contains (x, y - 1.0f)) continue;
        bool clash = false;
        for (auto& d : drips) if (std::abs (d.getBounds().getCentreX() - x) < 14.0f) clash = true;
        if (clash) continue;
        const float w = 1.8f + r.nextFloat() * 2.0f, len = 4.0f + r.nextFloat() * r.nextFloat() * 13.0f;
        juce::Path d;
        d.startNewSubPath (x - w, y - 3.0f);
        d.cubicTo (x - w, y + len * 0.5f, x - w * 0.45f, y + len * 0.8f, x - w * 0.45f, y + len);
        d.addCentredArc (x, y + len, w * 0.45f + 0.6f, w * 0.45f + 0.6f, 0.0f, -juce::MathConstants<float>::halfPi,
                         juce::MathConstants<float>::halfPi, false);
        d.cubicTo (x + w * 0.45f, y + len * 0.8f, x + w, y + len * 0.5f, x + w, y - 3.0f);
        d.closeSubPath();
        if (r.nextBool()) d.addEllipse (x - w * 0.45f, y + len + 3.0f + r.nextFloat() * 3.0f, w * 0.9f, w * 1.2f);   // falling drop
        drips.push_back (d);
        ++made;
    }
}

void Header::paintScreen (juce::Graphics& g, juce::Rectangle<float> r)
{
    paintBezel (g, r);
    auto s = r.reduced (9.0f);
    g.setColour (juce::Colour (0xff030202));
    g.fillRoundedRectangle (s, 3.0f);

    g.saveState();
    g.reduceClipRegion (s.toNearestInt());
    const auto c = s.getCentre();
    const float pulse = 0.5f + 0.5f * std::sin (phase * 2.1f);

    juce::ColourGradient bg (juce::Colour (0xff5a0d08).withAlpha (0.75f + 0.2f * pulse), c.x, c.y,
                             juce::Colours::transparentBlack, c.x + s.getWidth() * 0.45f, c.y, true);
    g.setGradientFill (bg);
    g.fillRect (s);

    const float R = s.getHeight() * 0.44f;
    g.setColour (juce::Colour (0xffe0401f).withAlpha (0.85f));
    g.drawEllipse (c.x - R, c.y - R, R * 2, R * 2, 1.4f);
    g.drawEllipse (c.x - R * 0.7f, c.y - R * 0.7f, R * 1.4f, R * 1.4f, 1.0f);

    // runes turning slowly between the rings
    g.setColour (juce::Colour (0xffff6a2a));
    for (int i = 0; i < 14; ++i)
    {
        const float a = phase * 0.25f + (float) i * juce::MathConstants<float>::twoPi / 14.0f;
        const auto p = c.getPointOnCircumference (R * 0.85f, a);
        juce::Path rune;
        rune.startNewSubPath (0, -3.5f); rune.lineTo (0, 3.5f);
        rune.startNewSubPath (0, -3.5f); rune.lineTo ((i % 3 == 0 ? -2.5f : 2.5f), -1.0f);
        if (i % 2) { rune.startNewSubPath (0, 0.5f); rune.lineTo (2.5f, 3.0f); }
        g.strokePath (rune, juce::PathStrokeType (1.0f), juce::AffineTransform::rotation (a).translated (p));
    }

    // inverted cross and eye
    juce::Path cross;
    cross.startNewSubPath (c.x, c.y - R * 0.62f); cross.lineTo (c.x, c.y + R * 0.62f);
    cross.startNewSubPath (c.x - R * 0.32f, c.y + R * 0.3f); cross.lineTo (c.x + R * 0.32f, c.y + R * 0.3f);
    g.setColour (juce::Colour (0xffff4a24).withAlpha (0.9f));
    g.strokePath (cross, juce::PathStrokeType (2.2f));

    juce::ColourGradient core (juce::Colour (0xfffff1c4), c.x, c.y, juce::Colour (0x00660000), c.x + R * 0.45f, c.y, true);
    core.addColour (0.3, juce::Colour (0xffff9a3a));
    core.addColour (0.65, juce::Colour (0x8cc3200f));
    g.setGradientFill (core);
    const float cr = R * (0.36f + 0.05f * pulse);
    g.fillEllipse (c.x - cr, c.y - cr, cr * 2, cr * 2);
    juce::Path eye;
    const float ew = R * 0.3f, eh = R * 0.17f;
    eye.startNewSubPath (c.x - ew, c.y);
    eye.quadraticTo (c.x, c.y - eh, c.x + ew, c.y);
    eye.quadraticTo (c.x, c.y + eh, c.x - ew, c.y);
    eye.closeSubPath();
    g.setColour (juce::Colour (0xff1a0201));
    g.fillPath (eye);
    const float look = std::sin (phase * 0.7f) * ew * 0.35f;
    g.setColour (juce::Colour (0xffffcf6e));
    g.fillEllipse (c.x + look - 1.4f, c.y - eh * 0.8f, 2.8f, eh * 1.6f);

    // scanlines
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    for (float y = s.getY(); y < s.getBottom(); y += 3.0f) g.drawHorizontalLine ((int) y, s.getX(), s.getRight());
    g.restoreState();
}

void Header::paintTitle (juce::Graphics& g)
{
    const auto tb = titlePath.getBounds();
    g.setColour (juce::Colour (0xff9a2715));
    for (auto& d : drips) g.fillPath (d);

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillPath (titlePath, juce::AffineTransform::translation (2.0f, 3.0f));

    juce::ColourGradient grad (juce::Colour (0xffef6a45), 0, tb.getY(), juce::Colour (0xff7e1b0f), 0, tb.getBottom(), false);
    grad.addColour (0.5, juce::Colour (0xffc63c23));
    g.saveState();
    g.reduceClipRegion (titlePath);
    g.setGradientFill (grad);
    g.fillRect (tb);
    // grime and chips
    juce::Random r (7);
    for (int i = 0; i < 700; ++i)
    {
        const float x = tb.getX() + r.nextFloat() * tb.getWidth(), y = tb.getY() + r.nextFloat() * tb.getHeight();
        const float sz = 0.6f + r.nextFloat() * r.nextFloat() * 3.2f;
        g.setColour (juce::Colour (0xff1c0503).withAlpha (0.25f + r.nextFloat() * 0.55f));
        g.fillEllipse (x, y, sz, sz * (0.6f + r.nextFloat()));
    }
    for (int i = 0; i < 40; ++i)
    {
        const float x = tb.getX() + r.nextFloat() * tb.getWidth(), y = tb.getY() + r.nextFloat() * tb.getHeight();
        g.setColour (juce::Colour (0xffffb08a).withAlpha (0.12f));
        g.drawLine (x, y, x + 4.0f + r.nextFloat() * 10.0f, y + r.nextFloat() * 2.0f, 0.8f);
    }
    g.restoreState();
    g.setColour (juce::Colour (0xff1c0503));
    g.strokePath (titlePath, juce::PathStrokeType (1.4f));

    g.setColour (colours::ash);
    drawSpaced (g, juce::String ("HORROR SOUND DESIGN  //  V") + JucePlugin_VersionString, Fonts::get().label (12.5f),
                { (int) titleR.getX() + 2, 83, 330, 16 }, juce::Justification::centredLeft, 0.2f);
}

void Header::paintPlate (juce::Graphics& g, juce::Rectangle<float> r)
{
    paintBezel (g, r);
    auto in = r.reduced (7.0f);
    g.setColour (juce::Colour (0xcc120b08));
    g.fillRoundedRectangle (in, 3.0f);

    // crest: castle under a blood moon in a bronze ring
    const float cs = in.getHeight() - 6.0f;
    const auto crest = juce::Rectangle<float> (in.getX() + 6.0f, in.getY() + 3.0f, cs, cs);
    const auto cc = crest.getCentre();
    const float cr = cs * 0.5f;
    juce::ColourGradient bz (juce::Colour (0xff8e6b4c), crest.getX(), crest.getY(), juce::Colour (0xff20140e), crest.getRight(), crest.getBottom(), false);
    g.setGradientFill (bz);
    g.fillEllipse (crest);
    const float ir = cr * 0.8f;
    g.saveState();
    juce::Path disc; disc.addEllipse (cc.x - ir, cc.y - ir, ir * 2, ir * 2);
    g.reduceClipRegion (disc);
    g.setColour (juce::Colour (0xff120706));
    g.fillEllipse (cc.x - ir, cc.y - ir, ir * 2, ir * 2);
    const float u = ir / 38.0f;
    auto T = [&] (float x, float y) { return juce::Point<float> (cc.x + (x - 50.0f) * u, cc.y + (y - 50.0f) * u); };
    auto K = [&] (float x, float y) { return T (50.0f + (x - 50.0f) * 1.1f, 96.0f + (y - 96.0f) * 1.1f); };   // castle scale
    {
        const auto mc = T (52, 42);
        const float mr = 28.0f * u;
        juce::ColourGradient moon (juce::Colour (0xfff3794a), mc.x - mr * 0.1f, mc.y - mr * 0.25f, juce::Colour (0xff7a160c), mc.x + mr, mc.y, true);
        moon.addColour (0.55, juce::Colour (0xffd0391f));
        g.setGradientFill (moon);
        g.fillEllipse (mc.x - mr, mc.y - mr, mr * 2, mr * 2);
    }
    juce::Path castle;
    auto tower = [&] (float x0, float x1, float top, int merlons)
    {
        castle.startNewSubPath (K (x0, 102)); castle.lineTo (K (x0, top));
        const float w = (x1 - x0) / (float) (merlons * 2 - 1);
        for (int m = 0; m < merlons * 2 - 1; ++m)
        {
            const float xa = x0 + (float) m * w, xb = xa + w;
            if (m % 2 == 0) { castle.lineTo (K (xa, top - 4)); castle.lineTo (K (xb, top - 4)); castle.lineTo (K (xb, top)); }
            else castle.lineTo (K (xb, top));
        }
        castle.lineTo (K (x1, 102)); castle.closeSubPath();
    };
    tower (20, 35, 64, 3); tower (32, 68, 48, 5); tower (62, 77, 62, 3);
    g.setColour (juce::Colour (0xff050303));
    g.fillPath (castle);
    g.setColour (juce::Colour (0xffffa24a));
    for (auto w : { juce::Point<float> (47.6f, 58), juce::Point<float> (38.5f, 70), juce::Point<float> (58.5f, 70),
                    juce::Point<float> (25.0f, 76), juce::Point<float> (72.0f, 76) })
    {
        const auto p = K (w.x, w.y);
        g.fillRect (p.x, p.y, 2.2f * u * 1.1f, 3.8f * u * 1.1f);
    }
    g.restoreState();
    g.setColour (juce::Colour (0xff120b08));
    g.drawEllipse (crest.reduced (0.5f), 1.0f);

    auto txt = in.withTrimmedLeft (cs + 14.0f).toNearestInt();
    g.setColour (colours::bone);
    drawSpaced (g, "CASTLE DANGER", Fonts::get().title (21.0f), txt.withHeight (26).withY (txt.getY() + 6), juce::Justification::centredLeft, 0.05f);
    g.setColour (juce::Colour (0xffd9772f));
    drawSpaced (g, "MUSIC COMPANY", Fonts::get().label (15.0f), txt.withHeight (18).withY (txt.getY() + 32), juce::Justification::centredLeft, 0.22f);
    g.setColour (colours::ash);
    drawSpaced (g, "WE MAKE MUSIC", Fonts::get().label (10.0f), txt.withHeight (12).withY (txt.getY() + 50), juce::Justification::centredLeft, 0.3f);
}

void Header::paint (juce::Graphics& g)
{
    paintScreen (g, screenR);
    paintTitle (g);
    paintPlate (g, plateR);

    g.setColour (colours::ash);
    drawSpaced (g, "PRESET", Fonts::get().label (13.0f), presetArea().withHeight (16), juce::Justification::centredLeft, 0.25f);

    // hazard stripe
    const juce::Rectangle<float> hz (0.0f, 102.0f, (float) getWidth(), 8.0f);
    g.setColour (juce::Colour (0xff140c0a));
    g.fillRect (hz);
    g.saveState();
    g.reduceClipRegion (hz.toNearestInt());
    g.setColour (juce::Colour (0xff7a1f19));
    for (float x = -20.0f; x < hz.getRight() + 20.0f; x += 18.0f)
    {
        juce::Path st;
        st.addQuadrilateral (x, hz.getBottom(), x + 9.0f, hz.getBottom(), x + 17.0f, hz.getY(), x + 8.0f, hz.getY());
        g.fillPath (st);
    }
    g.restoreState();
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawHorizontalLine ((int) hz.getY(), 0.0f, (float) getWidth());
    g.drawHorizontalLine ((int) hz.getBottom(), 0.0f, (float) getWidth());
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
    content.addAndMakeVisible (header);
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
    setResizeLimits (840, 581, 1800, 1245);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize (1080, 747);
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
    header.setBounds (0, 0, baseW, heresy::ui::Header::height);
    const auto pa = header.presetArea();
    presetBox.setBounds (pa.getX(), pa.getY() + 20, pa.getWidth(), 28);
    const int by = pa.getY() + 54, bh = 26;
    prevBtn.setBounds (pa.getX(), by, 32, bh);
    nextBtn.setBounds (pa.getX() + 36, by, 32, bh);
    saveBtn.setBounds (pa.getX() + 72, by, 66, bh);
    deleteBtn.setBounds (pa.getX() + 142, by, 72, bh);
    panicBtn.setBounds (pa.getX() + 218, by, 66, bh);

    const int top = heresy::ui::Header::height + 8;
    macroPanel.setBounds (16, top, 300, 236);
    for (int i = 0; i < macros.size(); ++i) macros[i]->setBounds (22 + i * 96, top + 52, 96, 150);

    for (int i = 0; i < pads.size(); ++i) pads[i]->setBounds (328 + i * 289, top, 277, 236);

    for (int i = 0; i < sections.size(); ++i)
        sections[i]->setBounds (16 + (i % 4) * 295, i < 4 ? top + 248 : top + 430, 283, 170);

    keyboard.setBounds (16, top + 612, 1168, 84);
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
