#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : presetTree("Presets")
{
    // Handler Objects initialisation
    bluetoothconnection = std::make_shared<BluetoothConnectionManager>();
    gesturemanager = std::make_shared<GestureManager>(bluetoothconnection);
    midihandler = std::make_shared<MIDIHandler>(gesturemanager);

    {
        //Setup for XML file directory Ensures it exists and file path is valid....
        juce::File directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("fibrephonic-juce");

        directory.createDirectory();

        xmlFile = directory.getChildFile("Presets.xml");
    }
    
    //Resize Main Window 
    setSize (1200, 700);

    {
        //Buttons and Toggles
        connectionsbutton.setLookAndFeel(&buttonlookandfeel);
        connectionsbutton.setButtonText("Connections");
        
        BluetoothButton.setLookAndFeel(&roundedbuttonlookandfeel);
        BluetoothButton.setButtonText("Bluetooth \n Connection");

        pConnectionsButton->onClick = [this] {
            isConnectionsToggled = !isConnectionsToggled;
            DBG("isConnections = " << (isConnectionsToggled ? "true" : "false"));

            if (isConnectionsToggled)  
            {
                isCalibrationToggled = false;

                BluetoothButton.setVisible(false);
            }
         };

        calibrationbutton.setLookAndFeel(&buttonlookandfeel);
        calibrationbutton.setButtonText("Calibration");

        pCalibrationButton->onClick = [this] {
            isCalibrationToggled = !isCalibrationToggled;
            DBG("isCalibration = " << (isCalibrationToggled ? "true" : "false"));

            if (isCalibrationToggled) 
            {
                isConnectionsToggled = false;

                addAndMakeVisible(BluetoothButton);
            }
         };

        addAndMakeVisible(connectionsbutton);
        addAndMakeVisible(calibrationbutton);

        {
            // Bluetooth Connection Handling Via Button.
            // Thread Closing and Handling Included within button logic.

            pBluetoothButton->onClick = [this] {
                isBlutoothToggled = !isBlutoothToggled;

                bluetoothconnection->setConnectionbool(isBlutoothToggled); // Passes to Bluetooth Connection Manager instance

                DBG("Bluetooth = " << (isBlutoothToggled ? "true" : "false"));

                if (isBlutoothToggled)
                {
                    bluetoothconnection->startThread(); // Triggers thread run function
                    gesturemanager->startPolling();
                    midihandler->startThread();
                }
                else {

                    bluetoothconnection->wait(100);
                    midihandler->wait(100);

                    //bluetoothconnection->signalThreadShouldExit();
                    //bluetoothconnection->stopThread(1000);
                }
             };
        }        
    }

    {
        //Parameters
        
        juce::ValueTree mainTree("MainTreeRoot");

        SwatchTree.clear();
        SwatchTree.resize(6);

        presetTree = juce::ValueTree("PresetTree"); 
        presetTree.removeAllChildren(nullptr);

        // Clear previous children to avoid duplicates
        presetTree.removeAllChildren(nullptr); 

        for (int i = 0; i < SwatchTree.size(); ++i)
        {
            SwatchTree[i] = juce::ValueTree("Swatch");

            SwatchTree[i].setProperty("index", i, nullptr);
            SwatchTree[i].setProperty("enabled", true, nullptr);
            SwatchTree[i].setProperty("VID_NFC", 1111, nullptr);
            SwatchTree[i].setProperty("MIDI_Channel", 0, nullptr);
            SwatchTree[i].setProperty("Input_Max", 1111, nullptr);
            SwatchTree[i].setProperty("Input_Min", 0, nullptr);
            SwatchTree[i].setProperty("Transform", (int)0, nullptr);
            SwatchTree[i].setProperty("Output_Max", 1111, nullptr);
            SwatchTree[i].setProperty("Output_Min", 0, nullptr);
            SwatchTree[i].setProperty("Reset", (bool)0, nullptr);

            // Add to the parameterTree as child
            presetTree.addChild(SwatchTree[i], -1, nullptr);
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

    {
//        // Serial Connection  Setup 
//        SerialPortConfig config(115200, 8,
//            SerialPortConfig::SERIALPORT_PARITY_NONE,
//            SerialPortConfig::STOPBITS_1,
//            SerialPortConfig::FLOWCONTROL_NONE);
//
//        serialPort = std::make_unique<SerialPort>("COM6", config, nullptr);
//
//        if (serialPort && serialPort->exists() && serialPort->open("COM6"))
//        {
//            inputStream = std::make_unique<SerialPortInputStream>(serialPort.get());
//            serialConnected = true;
//            DBG("Serial connected!");
//        }
//        else
//        {
//            DBG("Failed to open serial port");
//        }
    }

}

MainComponent::~MainComponent()
{
    if (bluetoothconnection)
    {
        bluetoothconnection->signalThreadShouldExit();

        if (!bluetoothconnection->stopThread(1000))
            DBG("Thread did not stop cleanly");

        // Reset shared_ptr to release ownership and allow destruction if no other refs
        bluetoothconnection.reset();
    }

    stopTimer();
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

    // Draw title text centered above the line 
    float textY = lineY - 40.0f;  

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Fibrephonic Mapper", 0, (int)textY, getWidth(), 30, juce::Justification::centred);

    g.setFont(juce::Font(15.0f));

    // Extra UI Element control for different toggle elements....

    //MUST CALL REPAINT
    if (isConnectionsToggled == true) {
        



        repaint(); 
    }
    else if (isCalibrationToggled == true){





        repaint();
    }
}

void MainComponent::resized()
{
    //Buttons and Toggles
    connectionsbutton.setBounds(0, 50, getWidth() / 2, 60);
    calibrationbutton.setBounds(getWidth() / 2, 50, getWidth() / 2, 60);
    BluetoothButton.setBounds(100, 150, 100, 100);
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
void MainComponent::timerCallback(){
    startTimerHz(30);  
    repaint();

    {
   

//        // Serial Connection
//        if (!serialConnected || !inputStream)
//            return;
//
//        char buffer[128] = { 0 };
//        int bytesRead = inputStream->read(buffer, sizeof(buffer) - 1);
//
//        if (bytesRead > 0)
//        {
//            buffer[bytesRead] = '\0'; // Null terminate buffer safely
//            juce::String rawData(buffer);
//            parseIMUData(rawData.trim());
//        }
    }
}
