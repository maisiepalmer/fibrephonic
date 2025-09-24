/**
 * @file MainComponent.cpp
 * @brief Main UI component implementation for gesture detection system
 * @author Joseph B, Maisie Palmer
 * @date Created: 2025
 */

#include "MainComponent.h"

MainComponent::MainComponent()
{
    gestureManager = std::make_shared<GestureManager>();
    connectionManager = std::make_shared<ConnectionManager>(gestureManager);
    
    // Set up the circular reference
    gestureManager->setConnectionManager(connectionManager);
    
    setupUI();
    
    // Start UI update timer
    startTimerHz(10); // Update UI 10 times per second
    
    setSize(600, 400);
}

MainComponent::~MainComponent()
{
    stopTimer();
    
    // Stop connection
    if (connectionManager && connectionManager->getIsConnected())
    {
        connectionManager->stopConnection();
    }
    
    // Clean up references
    if (gestureManager)
    {
        gestureManager->setConnectionManager(nullptr);
    }
    
    gestureManager.reset();
    connectionManager.reset();
}

void MainComponent::setupUI()
{
    // Title
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Textile Gesture Detection System", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::darkblue);
    
    // Connection controls
    addAndMakeVisible(toggleButton);
    toggleButton.setButtonText("Start Connection");
    toggleButton.setColour(juce::TextButton::buttonColourId, juce::Colours::forestgreen);
    toggleButton.onClick = [this] { toggleConnection(); };
    
    // Status labels
    addAndMakeVisible(connectionLabel);
    connectionLabel.setText("Connection: Disconnected", juce::dontSendNotification);
    connectionLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    connectionLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    
    addAndMakeVisible(gestureLabel);
    gestureLabel.setText("Last Gesture: None", juce::dontSendNotification);
    gestureLabel.setFont(juce::FontOptions(16.0f));
    gestureLabel.setColour(juce::Label::textColourId, juce::Colours::darkgreen);
    
    addAndMakeVisible(sensorDataLabel);
    sensorDataLabel.setText("Sensor Data: Waiting for connection...", juce::dontSendNotification);
    sensorDataLabel.setFont(juce::FontOptions(12.0f));
    sensorDataLabel.setJustificationType(juce::Justification::topLeft);
    sensorDataLabel.setColour(juce::Label::textColourId, juce::Colours::darkslategrey);
}

/**
 * @brief Paint callback for custom drawing
 * @param g Graphics context for drawing
 */
void MainComponent::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient gradient(juce::Colours::lightgrey.withAlpha(0.3f), 0, 0,
                                  juce::Colours::white, 0, getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Main border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 2);
    
    // Header separator
    g.setColour(juce::Colours::lightslategrey);
    g.drawLine(20, 70, getWidth() - 20, 70, 1.0f);
}

/**
 * @brief Layout components when window is resized
 */
void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Title section
    titleLabel.setBounds(bounds.removeFromTop(50));
    bounds.removeFromTop(10);
    
    // Control buttons
    auto buttonArea = bounds.removeFromTop(50);
    toggleButton.setBounds(buttonArea.removeFromLeft(180));
    buttonArea.removeFromLeft(20);
    bounds.removeFromTop(20);
    
    // Status section
    connectionLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(5);
    
    gestureLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    sensorDataLabel.setBounds(bounds.removeFromTop(120));
    bounds.removeFromTop(20);
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
    
    // Update toggle button
    toggleButton.setButtonText(isRunning ? "Stop Connection" : "Start Connection");
    toggleButton.setColour(juce::TextButton::buttonColourId,
                          isRunning ? juce::Colours::indianred : juce::Colours::forestgreen);
    
    // Update gesture info
    auto lastGesture = gestureManager->getLastGesture();
    juce::String gestureName = Gestures::getGestureName(lastGesture);
    gestureLabel.setText("Last Gesture: " + gestureName, juce::dontSendNotification);
    
    // Color-code gestures
    juce::Colour gestureColor = juce::Colours::darkgreen;
    if (lastGesture != Gestures::NO_GESTURE)
    {
        gestureColor = juce::Colours::orange;
    }
    gestureLabel.setColour(juce::Label::textColourId, gestureColor);
    
    // Update sensor data display
    if (connected)
    {
        juce::String sensorInfo;
        sensorInfo << "ACCELEROMETER (g):\n";
        sensorInfo << "   X: " << juce::String(connectionManager->getAccelerationX(), 3)
                   << "   Y: " << juce::String(connectionManager->getAccelerationY(), 3)
                   << "   Z: " << juce::String(connectionManager->getAccelerationZ(), 3) << "\n\n";
        
        sensorInfo << "GYROSCOPE (deg/s):\n";
        sensorInfo << "   X: " << juce::String(connectionManager->getGyroscopeX(), 2)
                   << "   Y: " << juce::String(connectionManager->getGyroscopeY(), 2)
                   << "   Z: " << juce::String(connectionManager->getGyroscopeZ(), 2) << "\n\n";
        
        sensorInfo << "MAGNETOMETER (uT):\n";
        sensorInfo << "   X: " << juce::String(connectionManager->getMagnetometerX(), 2)
                   << "   Y: " << juce::String(connectionManager->getMagnetometerY(), 2)
                   << "   Z: " << juce::String(connectionManager->getMagnetometerZ(), 2);
        
        sensorDataLabel.setText(sensorInfo, juce::dontSendNotification);
        sensorDataLabel.setColour(juce::Label::textColourId, juce::Colours::darkslategrey);
    }
    else
    {
        sensorDataLabel.setText("Sensor Data: No connection\n\nConnect to an x-IMU3 device to see live sensor readings.",
                               juce::dontSendNotification);
        sensorDataLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    }
}

/**
 * @brief Toggle connection state between running and stopped
 */
void MainComponent::toggleConnection()
{
    if (!isRunning)
    {
        DBG("Starting connection...");
        if (connectionManager)
        {
            connectionManager->startConnection();
            isRunning = true;
        }
    }
    else
    {
        DBG("Stopping connection...");
        if (connectionManager)
        {
            connectionManager->stopConnection();
            isRunning = false;
        }
    }
}
