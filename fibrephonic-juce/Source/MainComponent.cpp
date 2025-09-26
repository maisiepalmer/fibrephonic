/**
 * @file MainComponent.cpp
 * @brief Main UI component with calibration support
 */

#include "MainComponent.h"
#include "CalibrationComponent.h"

MainComponent::MainComponent()
{
    gestureManager = std::make_shared<GestureManager>();
    connectionManager = std::make_shared<ConnectionManager>(gestureManager);
    
    // Set up the circular reference
    gestureManager->setConnectionManager(connectionManager);
    
    // Create calibration component
    if (gestureManager->getDetector())
    {
        calibrationComponent = std::make_unique<CalibrationComponent>(*gestureManager->getDetector());
        addAndMakeVisible(calibrationComponent.get());
    }
    
    setupUI();
    
    // Start UI update timer
    startTimerHz(10); // Update UI 10 times per second
    
    setSize(600, 450); // Increased width for calibration panel
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
    
    calibrationComponent.reset();
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
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
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

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll();
    
    // Main border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 2);
    
    // Header separator
    g.setColour(juce::Colours::lightslategrey);
    g.drawLine(20, 70, getWidth() - 20, 70, 1.0f);
    
    // Vertical separator for calibration panel
    if (calibrationComponent)
    {
        int separatorX = getWidth() - 320;
        g.setColour(juce::Colours::lightslategrey);
        g.drawLine(separatorX, 80, separatorX, getHeight() - 20, 1.0f);
    }
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Reserve space for calibration panel on the right
    int calibrationWidth = 300;
    juce::Rectangle<int> mainArea = bounds;
    juce::Rectangle<int> calibArea;
    
    // Layout main controls in left area
    auto mainBounds = mainArea;
    
    // Title section
    titleLabel.setBounds(mainBounds.removeFromTop(50));
    mainBounds.removeFromTop(10);
    
    if (calibrationComponent)
    {
        calibArea = mainBounds.removeFromRight(calibrationWidth);
        mainArea.removeFromRight(20); // spacing
    }
    
    // Control buttons
    auto buttonArea = mainBounds.removeFromTop(50);
    toggleButton.setBounds(buttonArea.removeFromLeft(180));
    mainBounds.removeFromTop(20);
    
    // Status section
    connectionLabel.setBounds(mainBounds.removeFromTop(30));
    mainBounds.removeFromTop(5);
    
    gestureLabel.setBounds(mainBounds.removeFromTop(30));
    mainBounds.removeFromTop(5);
    
    sensorDataLabel.setBounds(mainBounds.removeFromTop(150));
    
    // Calibration component on the right
    if (calibrationComponent)
    {
        calibrationComponent->setBounds(calibArea);
    }
}

void MainComponent::timerCallback()
{
    updateUI();
}

void MainComponent::updateUI()
{
    if (!connectionManager || !gestureManager)
        return;

    // Connection status
    bool connected = connectionManager->getIsConnected();
    connectionLabel.setText("Connection: " + juce::String(connected ? "Connected" : "Disconnected"),
                           juce::dontSendNotification);
    connectionLabel.setColour(juce::Label::textColourId,
                             connected ? juce::Colours::green : juce::Colours::red);

    // Toggle button
    toggleButton.setButtonText(isRunning ? "Stop Connection" : "Start Connection");
    toggleButton.setColour(juce::TextButton::buttonColourId,
                          isRunning ? juce::Colours::indianred : juce::Colours::forestgreen);

    // Gesture info
    float lastTapVelocity = gestureManager->getLastTapVelocity();
    if (lastTapVelocity > 0.0f)
    {
        gestureLabel.setText("Last Gesture: Tap (velocity: " + juce::String(lastTapVelocity, 1) + ")",
                            juce::dontSendNotification);
    }
    else
    {
        gestureLabel.setText("Last Gesture: None", juce::dontSendNotification);
    }

    // Calibration status popup
    if (gestureManager->isCalibrated())
    {
        if (!calibrationStatusShown)
        {
            calibrationStatusShown = true;
            juce::AlertWindow::showAsync(MessageBoxOptions()
                             .withIconType (MessageBoxIconType::InfoIcon)
                             .withTitle ("Calibration Status")
                             .withMessage ("System is calibrated and ready for gesture detection!")
                             .withButton("Close"),
                             nullptr);
        }
    }

    // Sensor data
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
                   << "   Z: " << juce::String(connectionManager->getMagnetometerZ(), 2) << "\n\n";

        sensorInfo << "Calibration: " << (gestureManager->isCalibrated() ? "YES" : "NO");

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
