#pragma once

#include <JuceHeader.h>
#include "Data/GestureManager.h"
#include "Data/ConnectionManager.h"
#include <memory>

/**
 * @brief Main UI component for the gesture detection application
 *
 * Manages the user interface and coordinates between the ConnectionManager,
 * GestureManager, and GestureRecorder components.
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
    
    // UI Components - Main Controls
    juce::Label titleLabel;
    juce::TextButton toggleButton;
    
    // Status Display
    juce::Label connectionLabel;
    juce::Label gestureLabel;
    juce::Label sensorDataLabel;
    
    // Application state
    bool isRunning = false;
    
    // Methods
    void timerCallback() override;
    void updateUI();
    void toggleConnection();

    void setupUI();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
