#pragma once

#include <JuceHeader.h>
// #include <juce_serialport/juce_serialport.h>
#include "UI/LookandFeel.h"
#include "Data/OSCManager.h"
#include "imuExamples/BluetoothConnection.h"
#include "BluetoothConnectionManager.h"
#include "Data/SerialPort.h"
#include "imuExamples/Connection.h"

//==============================================================================

class MainComponent  : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void parseIMUData(const juce::String& data);

private:
    //==============================================================================
    //Instanciation Station

    //Custom Look and Feel Instances
    ButtonLookandFeel buttonlookandfeel;
    RoundedButtonLookandFeel roundedbuttonlookandfeel;

    //Toggle Buttons and Functions
    bool isConnectionsToggled = true, isCalibrationToggled = false,
         isBlutoothToggled = false;

    TextButton connectionsbutton, calibrationbutton, BluetoothButton;

    TextButton* pConnectionsButton = &connectionsbutton;
    TextButton* pCalibrationButton = &calibrationbutton;
    TextButton* pBluetoothButton = &BluetoothButton;
 
    //std::vector<juce::Button> SwatchButtons;

    inline void hideConnections()
    {
        /*
        instructionsOverlay->setVisible(false);
        dismissButton.setVisible(false);
        */
    }

    inline void showConnections()
    {

    }

    //OSC and Chip Communication
    OSCManager oscmanager;

    //Parameters and XML
    ValueTree presetTree, ParameterTree;

    std::vector<ValueTree> SwatchTree;

    File xmlFile;

    // Serial 
    // std::unique_ptr<SerialPort> serialPort;
    // std::unique_ptr<SerialPortInputStream> inputStream;
    bool serialConnected = false; 

public:
    // Bluetooth Connection and Thread 
    std::unique_ptr<BluetoothConnectionManager> bluetoothconnection;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
