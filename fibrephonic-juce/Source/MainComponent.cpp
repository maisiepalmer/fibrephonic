#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: presetTree("Presets")
{
    // Handler Objects initialisation
    bluetoothConnection = std::make_shared<BluetoothConnectionManager>();
    gestureManager = std::make_shared<GestureManager>(bluetoothConnection);
    midiHandler = std::make_shared<MIDIHandler>(gestureManager);
    
    //Setup for XML file directory Ensures it exists and file path is valid....
    juce::File directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("fibrephonic-juce");
    
    directory.createDirectory();
    
    xmlFile = directory.getChildFile("Presets.xml");
    
    //Resize Main Window
    setSize (1200, 700);
    
    // Menu
    connections.setLookAndFeel(&buttonlookandfeel);
    connections.setButtonText("Connections");

    pConnectionsButton->onClick = [this] {
        DBG("Window = Connections");
        updateWindows(Windows::CONNECTIONS_WINDOW);
    };
    
    calibration.setLookAndFeel(&buttonlookandfeel);
    calibration.setButtonText("Calibration");
    
    pCalibrationButton->onClick = [this] {
        DBG("Window = Calibration");
        updateWindows(Windows::CALIBRATION_WINDOW);
    };
    
    addAndMakeVisible(connections);
    addAndMakeVisible(calibration);
    addAndMakeVisible(connectionsWindow);
    addAndMakeVisible(calibrationWindow);
    
    // Bluetooth Connection Handling Via Button.
    // Thread Closing and Handling Included within button logic.
    bluetooth.setLookAndFeel(&roundedbuttonlookandfeel);
    bluetooth.setButtonText("Bluetooth \n Connection");
    
    pBluetoothButton->onClick = [this] {
        isBluetoothToggled = !isBluetoothToggled;
        
        bluetoothConnection->setConnectionBool(isBluetoothToggled); // Passes to Bluetooth Connection Manager instance
        
        DBG("Bluetooth = " << (isBluetoothToggled ? "true" : "false"));
        
        if (isBluetoothToggled)
        {
            bluetoothConnection->startThread(); // Triggers thread run function
            gestureManager->startPolling();
            midiHandler->startThread();
        }
        else {
            
            bluetoothConnection->wait(100);
            midiHandler->wait(100);
        }
    };
    
    //Parameters
    
    juce::ValueTree mainTree("MainTreeRoot");
    
    swatchTree.clear();
    swatchTree.resize(6);
    
    presetTree = juce::ValueTree("PresetTree");
    presetTree.removeAllChildren(nullptr);
    
    // Clear previous children to avoid duplicates
    presetTree.removeAllChildren(nullptr);
    
    for (int i = 0; i < swatchTree.size(); ++i)
    {
        swatchTree[i] = juce::ValueTree("Swatch");
        
        swatchTree[i].setProperty("index", i, nullptr);
        swatchTree[i].setProperty("enabled", true, nullptr);
        swatchTree[i].setProperty("VID_NFC", 1111, nullptr);
        swatchTree[i].setProperty("MIDI_Channel", 0, nullptr);
        swatchTree[i].setProperty("Input_Max", 1111, nullptr);
        swatchTree[i].setProperty("Input_Min", 0, nullptr);
        swatchTree[i].setProperty("Transform", (int)0, nullptr);
        swatchTree[i].setProperty("Output_Max", 1111, nullptr);
        swatchTree[i].setProperty("Output_Min", 0, nullptr);
        swatchTree[i].setProperty("Reset", (bool)0, nullptr);
        
        // Add to the parameterTree as child
        presetTree.addChild(swatchTree[i], -1, nullptr);
    }
    
    juce::ValueTree parameterTree("ParameterTree");
    parameterTree.setProperty("someParameter", 42, nullptr);
    
    mainTree.addChild(presetTree, 0, nullptr);
    mainTree.addChild(parameterTree, -1, nullptr);
    
    // Save parameters to XML file
    if (auto xml = mainTree.createXml())
    {
        if (xml->writeTo(xmlFile))
            DBG("Saved XML successfully to: " << xmlFile.getFullPathName());
        else
            DBG("Failed to save XML to: " << xmlFile.getFullPathName());
    }
    else
    {
        DBG("Failed to create XML from presetTree");
    }
}

MainComponent::~MainComponent()
{
    stopTimer();
    
    if (bluetoothConnection)
    {
        bluetoothConnection->signalThreadShouldExit();
        
        // Important: call stopThread only from a different thread
        // Make sure this destructor runs on the GUI thread, NOT the Bluetooth thread
        if (Thread::getCurrentThreadId() != bluetoothConnection->getThreadId())
        {
            if (!bluetoothConnection->stopThread(1000))
                DBG("Thread did not stop cleanly");
            
            bluetoothConnection.reset();
        }
        else
        {
            // If destructor somehow called from inside thread,
            // defer cleanup to GUI thread asynchronously
            auto threadPtr = std::move(bluetoothConnection);
            MessageManager::callAsync([threadPtr = std::move(threadPtr)]() mutable
                                      {
                threadPtr->signalThreadShouldExit();
                if (!threadPtr->stopThread(1000))
                    DBG("Thread did not stop cleanly (deferred)");
                // threadPtr destroyed here
            });
        }
    }
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Fill the background with a gradient
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Define the gradient
    juce::ColourGradient gradient(
                                  juce::Colours::black, 0.0f, 0.0f,
                                  juce::Colours::darkgrey, (float)getWidth(), (float)getHeight(), false);
    
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
    
    // Parameters for outlines and corners
    float outlinethickness = 3.0f;
    float cornerRadius = 50.0f;
    
    // Draw main rounded rectangle outline
    juce::Rectangle<float> bounds = getLocalBounds().toFloat().reduced(outlinethickness * 0.5f);
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds, cornerRadius, outlinethickness);
    
    float lineY = cornerRadius + outlinethickness * 0.5f;
    
    g.fillRect(0.0f, lineY - (outlinethickness / 2.0f), (float)getWidth(), outlinethickness);
}

void MainComponent::resized()
{
    auto height = getHeight();
    auto width = getWidth();
    
    auto menuHeight = 50;
    
    // menu
    connections.setBounds(0, 0, width/2, menuHeight);
    calibration.setBounds(width/2, 0, width/ 2, menuHeight);
    
    // windows
    connectionsWindow.setBounds(0, menuHeight, width, height-menuHeight);
    calibrationWindow.setBounds(0, menuHeight, width, height-menuHeight);
    
    bluetooth.setBounds(100, 150, 100, 100);
}

//==============================================================================
void MainComponent::parseIMUData(const juce::String& data)
{
    // Expected format: "ACC:0.01,0.02,9.8;GYRO:0.01,0.00,0.1;"
    DBG("IMU Raw Data: " << data);
    
    juce::StringArray sections;
    sections.addTokens(data, ";", "");
    
    for (auto& section : sections)
    {
        if (section.startsWith("ACC:"))
        {
            auto accValues = juce::StringArray::fromTokens(section.fromFirstOccurrenceOf("ACC:", false, false), ",", "");
            if (accValues.size() == 3)
            {
                float ax = accValues[0].getFloatValue();
                float ay = accValues[1].getFloatValue();
                float az = accValues[2].getFloatValue();
                DBG("Accel: " << ax << ", " << ay << ", " << az);
            }
        }
        else if (section.startsWith("GYRO:"))
        {
            auto gyroValues = juce::StringArray::fromTokens(section.fromFirstOccurrenceOf("GYRO:", false, false), ",", "");
            if (gyroValues.size() == 3)
            {
                float gx = gyroValues[0].getFloatValue();
                float gy = gyroValues[1].getFloatValue();
                float gz = gyroValues[2].getFloatValue();
                DBG("Gyro: " << gx << ", " << gy << ", " << gz);
            }
        }
    }
}

//==============================================================================
void MainComponent::timerCallback()
{
    startTimerHz(30);
    repaint();
}
