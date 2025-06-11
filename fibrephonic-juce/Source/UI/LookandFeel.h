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



