#pragma once

#include <JuceHeader.h>
#include "Data/GestureManager.h"
#include "Data/ConnectionManager.h"
#include <memory>

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
    
    // UI Components for debugging/display
    juce::Label statusLabel;
    juce::Label gestureLabel;
    juce::Label connectionLabel;
    juce::Label sensorDataLabel;
    
    juce::TextButton startButton;
    juce::TextButton stopButton;
    
    void timerCallback() override;
    void updateUI();
    void startButtonClicked();
    void stopButtonClicked();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
