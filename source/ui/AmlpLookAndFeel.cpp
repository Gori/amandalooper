#include "AmlpLookAndFeel.h"

AmlpLookAndFeel::AmlpLookAndFeel()
{
    // Dark colour scheme
    setColour (juce::ResizableWindow::backgroundColourId, getBackgroundColour());
    setColour (juce::TextButton::buttonColourId, getSurfaceColour());
    setColour (juce::TextButton::textColourOnId, getTextColour());
    setColour (juce::TextButton::textColourOffId, getTextColour());
    setColour (juce::Label::textColourId, getTextColour());
    setColour (juce::Slider::thumbColourId, juce::Colour (0xff4488cc));
    setColour (juce::Slider::trackColourId, getSurfaceColour());
    setColour (juce::Slider::backgroundColourId, juce::Colour (0xff222222));
    setColour (juce::ComboBox::backgroundColourId, getSurfaceColour());
    setColour (juce::ComboBox::textColourId, getTextColour());
    setColour (juce::PopupMenu::backgroundColourId, getPanelColour());
    setColour (juce::PopupMenu::textColourId, getTextColour());
    setColour (juce::PopupMenu::highlightedBackgroundColourId, getSurfaceColour());
}

juce::Colour AmlpLookAndFeel::getTrackColour (int trackIndex)
{
    static const juce::Colour colours[] = {
        juce::Colour (0xffff6b6b),  // Red
        juce::Colour (0xff4ecdc4),  // Teal
        juce::Colour (0xffffe66d),  // Yellow
        juce::Colour (0xffa29bfe),  // Purple
        juce::Colour (0xffff9ff3),  // Pink
        juce::Colour (0xff54a0ff),  // Blue
        juce::Colour (0xff5f27cd),  // Deep purple
        juce::Colour (0xff01a3a4),  // Dark teal
    };

    return colours[trackIndex % 8];
}

void AmlpLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    auto cornerSize = 6.0f;

    auto baseColour = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColour = baseColour.brighter (0.2f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter (0.1f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, cornerSize);

    g.setColour (baseColour.brighter (0.2f));
    g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
}
