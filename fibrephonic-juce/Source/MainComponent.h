#pragma once

#include <JuceHeader.h>
#include "Data/GestureManager.h"
#include "Data/ConnectionManager.h"
#include <memory>

// Forward declaration
class CalibrationComponent;

/**
 * @brief Main UI component for the gesture detection application with calibration
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
    
    // Calibration UI
    std::unique_ptr<CalibrationComponent> calibrationComponent;
    
    // UI Components - Main Controls
    juce::Label titleLabel;
    juce::TextButton toggleButton;
    
    // Status Display
    juce::Label connectionLabel;
    juce::Label gestureLabel;
    juce::Label sensorDataLabel;
    
    // Application state
    bool isRunning = false;
    bool calibrationStatusShown = false;
    
    // Methods
    void timerCallback() override;
    void updateUI();
    void toggleConnection();
    void setupUI();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
