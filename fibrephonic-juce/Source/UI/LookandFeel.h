/*
  ==============================================================================

    LookandFeel.h
    Created: 3 Jun 2025 12:06:24pm
    Author:  josep

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ButtonLookandFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.5f);

        // Fill green background if pressed
        if (isButtonDown)
            g.setColour(juce::Colours::green);
        else
            g.setColour(juce::Colours::transparentBlack);

        g.fillRect(bounds);

        // Overlay transparent grey if hovered
        if (isMouseOverButton)
        {
            g.setColour(juce::Colours::grey.withAlpha(0.3f));
            g.fillRect(bounds);
        }

        // White border
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 3.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        auto bounds = button.getLocalBounds();
        g.setColour(juce::Colours::white);
        g.setFont(15.0f);
        g.drawFittedText(button.getButtonText(), bounds, juce::Justification::centred, 1);
    }
};

class RoundedButtonLookandFeel : public ButtonLookandFeel
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.5f);
        float cornerRadius = 10.0f; // You can adjust this for more or less roundness

        // Fill green background if pressed
        if (isButtonDown)
            g.setColour(juce::Colours::green);
        else
            g.setColour(juce::Colours::transparentBlack);

        g.fillRoundedRectangle(bounds, cornerRadius);

        // Overlay transparent grey if hovered
        if (isMouseOverButton)
        {
            g.setColour(juce::Colours::grey.withAlpha(0.3f));
            g.fillRoundedRectangle(bounds, cornerRadius);
        }

        // White border
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds, cornerRadius, 3.0f);
    }
};

class SliderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        const float radius = juce::jmin(width, height) / 2.0f - 2.0f;
        const float centreX = x + width / 2.0f;
        const float centreY = y + height / 2.0f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw base ring
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawEllipse(rx, ry, rw, rw, 1.5f);

        // Draw arc (value ring)
        juce::Path valueArc;
        valueArc.addArc(rx, ry, rw, rw, rotaryStartAngle, angle, true);

        g.setColour(juce::Colours::white);
        g.strokePath(valueArc, juce::PathStrokeType(2.0f)); // thinner ring
    }

    Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;

        auto bounds = slider.getLocalBounds();
        const int textBoxHeight = 20;
        const int textBoxOffset = 6; // move it down a bit

        // Reserve space at the bottom for the text box
        auto sliderArea = bounds.reduced(2);
        sliderArea.removeFromBottom(textBoxHeight + textBoxOffset);

        layout.sliderBounds = sliderArea;

        // Position the text box
        layout.textBoxBounds = {
            (bounds.getWidth() - 60) / 2,
            sliderArea.getBottom() + textBoxOffset,
            60,
            textBoxHeight
        };

        return layout;
    }
};


