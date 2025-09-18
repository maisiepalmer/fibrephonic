#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Create the gesture detection system
    DBG("Initializing gesture detection system...");
    
    // Create both managers
    gestureManager = std::make_shared<GestureManager>();
    connectionManager = std::make_shared<ConnectionManager>(gestureManager);
    
    // Set up the circular reference (this resolves the dependency issue)
    gestureManager->setConnectionManager(connectionManager);
    
    DBG("Gesture detection system initialized");
    
    // Set up UI components
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Gesture Detection System", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(connectionLabel);
    connectionLabel.setText("Connection: Disconnected", juce::dontSendNotification);
    connectionLabel.setFont(juce::Font(16.0f));
    
    addAndMakeVisible(gestureLabel);
    gestureLabel.setText("Last Gesture: None", juce::dontSendNotification);
    gestureLabel.setFont(juce::Font(16.0f));
    
    addAndMakeVisible(sensorDataLabel);
    sensorDataLabel.setText("Sensor Data: Waiting...", juce::dontSendNotification);
    sensorDataLabel.setFont(juce::Font(14.0f));
    sensorDataLabel.setJustificationType(juce::Justification::topLeft);
    
    addAndMakeVisible(startButton);
    startButton.setButtonText("Start Detection");
    startButton.onClick = [this] { startButtonClicked(); };
    
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop Detection");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setEnabled(false);
    
    // Start UI update timer
    startTimerHz(10); // Update UI 10 times per second
    
    setSize(500, 400);
}

MainComponent::~MainComponent()
{
    DBG("MainComponent shutting down...");
    
    // Stop everything cleanly
    stopTimer();
    
    if (connectionManager)
    {
        connectionManager->stopConnection();
    }
    
    // Clear references in proper order
    if (gestureManager)
    {
        gestureManager->setConnectionManager(nullptr);
    }
    
    gestureManager.reset();
    connectionManager.reset();
    
    DBG("MainComponent shutdown complete");
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw a border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 2);
}

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
    startButton.setBounds(buttonArea.removeFromLeft(120));
    buttonArea.removeFromLeft(10);
    stopButton.setBounds(buttonArea.removeFromLeft(120));
}

void MainComponent::timerCallback()
{
    updateUI();
}

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

void MainComponent::startButtonClicked()
{
    DBG("Starting gesture detection...");
    
    if (connectionManager)
    {
        connectionManager->startConnection();
        startButton.setEnabled(false);
        stopButton.setEnabled(true);
    }
}

void MainComponent::stopButtonClicked()
{
    DBG("Stopping gesture detection...");
    
    if (connectionManager)
    {
        connectionManager->stopConnection();
        startButton.setEnabled(true);
        stopButton.setEnabled(false);
    }
}
