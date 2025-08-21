#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Create the gesture manager first.
    gestureManager = std::make_shared<GestureManager>();
    
    // Now, create the Bluetooth connection manager and pass the gesture manager to it.
    connectionManager = std::make_shared<ConnectionManager>(gestureManager);
    
    // Finally, set the connection manager back on the gesture manager to complete the circular dependency.
    gestureManager->setConnectionManager(connectionManager);
    
    // Configure and add the "Connect" button
    addAndMakeVisible(connectButton);
    connectButton.setButtonText("Connect");
    connectButton.addListener(this);

    // Configure and add the status label
    addAndMakeVisible(statusLabel);
    statusLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centred);

    // Resize Main Window
    setSize(1200, 700);

    // Start the timer to update the UI
    startTimerHz(10); // Update at 10 Hz
}

MainComponent::~MainComponent()
{
    // Stop the timer to prevent crashes during shutdown
    stopTimer();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Fill the background with a gradient
    juce::Colour colour1 = juce::Colour(0xff2a2a2a);
    juce::Colour colour2 = juce::Colour(0xff000000);
    g.setGradientFill(juce::ColourGradient(colour1, 0, 0, colour2, getWidth(), getHeight(), true));
    g.fillAll();
}

void MainComponent::resized()
{
    // Lay out the components in the window
    auto bounds = getLocalBounds();
    auto buttonHeight = 50;
    auto buttonWidth = 200;
    
    // Center the button at the top
    connectButton.setBounds(bounds.getCentreX() - buttonWidth / 2, 50, buttonWidth, buttonHeight);

    // Position the label below the button
    statusLabel.setBounds(bounds.getCentreX() - 300, 150, 600, 100);
}

//==============================================================================
void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &connectButton)
    {
        if (!connectionManager->getIsConnected())
        {
            DBG("Attempting to start connection...");
            connectionManager->startConnection();
        }
        else
        {
            DBG("Attempting to stop connection...");
            connectionManager->stopConnection();
        }
    }
}

void MainComponent::timerCallback()
{
    // Update the UI with the connection status and data
    if (connectionManager->getIsConnected())
    {
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        statusLabel.setText("Connected!", juce::dontSendNotification);
        connectButton.setButtonText("Disconnect");
    }
    else
    {
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        statusLabel.setText("Disconnected. Waiting for a device...", juce::dontSendNotification);
        connectButton.setButtonText("Connect");
    }
}
