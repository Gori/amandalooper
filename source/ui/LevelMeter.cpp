#include "LevelMeter.h"
#include <cmath>

LevelMeter::LevelMeter (Orientation orient)
    : orientation (orient)
{
    startTimerHz (30);
}

void LevelMeter::setLevel (float left, float right)
{
    leftPeak = left;
    rightPeak = right;
}

void LevelMeter::setInputMeter (bool isInput)
{
    inputMeter = isInput;
}

void LevelMeter::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (juce::Colour (0xff111111));
    g.fillRoundedRectangle (bounds, 2.0f);

    auto meterColour = inputMeter ? AmlpLookAndFeel::getArmedColour()
                                  : AmlpLookAndFeel::getPlayingColour();

    auto drawBar = [&] (juce::Rectangle<float> area, float level)
    {
        if (level <= 0.0f)
            return;

        // Clamp
        level = std::min (level, 1.0f);

        // Color gradient: green -> yellow -> red
        juce::Colour barColour;
        if (level < 0.6f)
            barColour = meterColour;
        else if (level < 0.85f)
            barColour = AmlpLookAndFeel::getOverdubbingColour();
        else
            barColour = AmlpLookAndFeel::getRecordingColour();

        if (orientation == Orientation::vertical)
        {
            float h = area.getHeight() * level;
            g.setColour (barColour);
            g.fillRoundedRectangle (area.getX(), area.getBottom() - h,
                                     area.getWidth(), h, 1.0f);
        }
        else
        {
            float w = area.getWidth() * level;
            g.setColour (barColour);
            g.fillRoundedRectangle (area.getX(), area.getY(),
                                     w, area.getHeight(), 1.0f);
        }
    };

    bounds = bounds.reduced (1.0f);

    if (orientation == Orientation::vertical)
    {
        auto leftArea = bounds.removeFromLeft (bounds.getWidth() * 0.45f);
        bounds.removeFromLeft (bounds.getWidth() * 0.1f);
        auto rightArea = bounds;

        drawBar (leftArea, leftDisplay);
        drawBar (rightArea, rightDisplay);
    }
    else
    {
        auto leftArea = bounds.removeFromTop (bounds.getHeight() * 0.45f);
        bounds.removeFromTop (bounds.getHeight() * 0.1f);
        auto rightArea = bounds;

        drawBar (leftArea, leftDisplay);
        drawBar (rightArea, rightDisplay);
    }
}

void LevelMeter::timerCallback()
{
    // Smooth decay
    const float decay = 0.85f;
    leftDisplay = std::max (leftPeak, leftDisplay * decay);
    rightDisplay = std::max (rightPeak, rightDisplay * decay);

    // Reset peaks after reading
    leftPeak = 0.0f;
    rightPeak = 0.0f;

    repaint();
}
