/**
 * @file MainComponent.cpp
 * @brief Main UI component implementation for gesture detection system
 * @author Joseph B, Maisie Palmer
 * @date Created: 2025
 */

#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Create the gesture detection system
    gestureManager = std::make_shared<GestureManager>();
    connectionManager = std::make_shared<ConnectionManager>(gestureManager);
    
    // Set up the circular reference
    gestureManager->setConnectionManager(connectionManager);
    
    // Set up UI components with new FontOptions
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Gesture Detection System", juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(20.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(connectionLabel);
    connectionLabel.setText("Connection: Disconnected", juce::dontSendNotification);
    connectionLabel.setFont(juce::FontOptions(16.0f));
    
    addAndMakeVisible(gestureLabel);
    gestureLabel.setText("Last Gesture: None", juce::dontSendNotification);
    gestureLabel.setFont(juce::FontOptions(16.0f));
    
    addAndMakeVisible(sensorDataLabel);
    sensorDataLabel.setText("Sensor Data: Waiting...", juce::dontSendNotification);
    sensorDataLabel.setFont(juce::FontOptions(14.0f));
    sensorDataLabel.setJustificationType(juce::Justification::topLeft);
    
    addAndMakeVisible(toggleButton);
    toggleButton.setButtonText("Start");
    toggleButton.onClick = [this] { toggleConnection(); };
    
    // Start UI update timer
    startTimerHz(10); // Update UI 10 times per second
    
    setSize(500, 400);
}

MainComponent::~MainComponent()
{
    stopTimer();
    
    if (connectionManager && connectionManager->getIsConnected())
    {
        connectionManager->stopConnection();
    }
    
    if (gestureManager)
    {
        gestureManager->setConnectionManager(nullptr);
    }
    
    gestureManager.reset();
    connectionManager.reset();
}

/**
 * @brief Paint callback for custom drawing
 * @param g Graphics context for drawing
 */
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw a border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);
}

/**
 * @brief Layout components when window is resized
 */
void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    statusLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(10);
    
    connectionLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(5);
    
    gestureLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    sensorDataLabel.setBounds(bounds.removeFromTop(120));
    bounds.removeFromTop(20);
    
    auto buttonArea = bounds.removeFromTop(40);
    toggleButton.setBounds(buttonArea.removeFromLeft(120));
}

/**
 * @brief Timer callback for UI updates
 */
void MainComponent::timerCallback()
{
    updateUI();
}

/**
 * @brief Update UI elements with current system state
 */
void MainComponent::updateUI()
{
    if (!connectionManager || !gestureManager)
        return;
    
    // Update connection status
    bool connected = connectionManager->getIsConnected();
    connectionLabel.setText("Connection: " + juce::String(connected ? "Connected" : "Disconnected"),
                           juce::dontSendNotification);
    connectionLabel.setColour(juce::Label::textColourId,
                             connected ? juce::Colours::green : juce::Colours::red);
    
    // Update toggle button state
    toggleButton.setButtonText(isRunning ? "Stop" : "Start");
    toggleButton.setColour(juce::TextButton::buttonColourId,
                          isRunning ? juce::Colours::indianred : juce::Colours::forestgreen);
    
    // Update gesture info
    auto lastGesture = gestureManager->getLastGestureName();
    gestureLabel.setText("Last Gesture: " + juce::String(lastGesture), juce::dontSendNotification);
    
    // Update sensor data display
    if (connected)
    {
        juce::String sensorInfo;
        sensorInfo << "Accelerometer: "
                   << juce::String(connectionManager->getAccelerationX(), 2) << ", "
                   << juce::String(connectionManager->getAccelerationY(), 2) << ", "
                   << juce::String(connectionManager->getAccelerationZ(), 2) << "\n";
        sensorInfo << "Gyroscope: "
                   << juce::String(connectionManager->getGyroscopeX(), 2) << ", "
                   << juce::String(connectionManager->getGyroscopeY(), 2) << ", "
                   << juce::String(connectionManager->getGyroscopeZ(), 2) << "\n";
        sensorInfo << "Magnetometer: "
                   << juce::String(connectionManager->getMagnetometerX(), 2) << ", "
                   << juce::String(connectionManager->getMagnetometerY(), 2) << ", "
                   << juce::String(connectionManager->getMagnetometerZ(), 2);
        
        sensorDataLabel.setText(sensorInfo, juce::dontSendNotification);
    }
    else
    {
        sensorDataLabel.setText("Sensor Data: No connection", juce::dontSendNotification);
    }
}

/**
 * @brief Toggle connection state between running and stopped
 */
void MainComponent::toggleConnection()
{
    if (!isRunning)
    {
        if (connectionManager)
        {
            connectionManager->startConnection();
            isRunning = true;
        }
    }
    else
    {
        if (connectionManager)
        {
            connectionManager->stopConnection();
            isRunning = false;
        }
    }
}
