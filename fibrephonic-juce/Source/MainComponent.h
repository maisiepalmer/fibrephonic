#pragma once

#include <JuceHeader.h>
#include "Data/BluetoothConnectionManager.h"
#include "Data/GestureManager.h"

//==============================================================================
/*
    This component manages the main application window and its GUI elements.
    It inherits from juce::Component for GUI functionality and juce::Timer
    to handle periodic updates.
*/
class MainComponent  : public juce::Component,
                       public juce::Button::Listener,
                       public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;

    // Class instances, shared pointer for managing lifetimes
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;
    std::shared_ptr<GestureManager> gestureManager;
    
    // GUI components
    juce::TextButton connectButton;
    juce::Label statusLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
