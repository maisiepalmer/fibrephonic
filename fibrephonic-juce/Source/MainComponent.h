#pragma once

#include <JuceHeader.h>
#include "Data/GestureManager.h"
#include "Data/ConnectionManager.h"
#include <memory>

/**
 * @brief Main UI component for the gesture detection application
 *
 * Manages the user interface and coordinates between the ConnectionManager
 * and GestureManager components.
 */
class MainComponent : public juce::Component, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Core gesture detection system
    std::shared_ptr<GestureManager> gestureManager;
    std::shared_ptr<ConnectionManager> connectionManager;
    
    // UI Components
    juce::Label statusLabel;
    juce::Label gestureLabel;
    juce::Label connectionLabel;
    juce::Label sensorDataLabel;
    
    juce::TextButton toggleButton; // Single button for start/stop
    
    bool isRunning = false;
    
    void timerCallback() override;
    void updateUI();
    void toggleConnection();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
