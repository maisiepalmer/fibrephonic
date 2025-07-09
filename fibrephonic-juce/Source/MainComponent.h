#pragma once

#include <JuceHeader.h>
// #include <juce_serialport/juce_serialport.h>
#include "UI/LookandFeel.h"
#include "Data/OSCManager.h"
#include "imuExamples/BluetoothConnection.h"
#include "Data/BluetoothConnectionManager.h"
#include "Data/SerialPort.h"
//#include "imuExamples/Connection.h"
#include "Data/GestureManager.h"
#include "../Source/Data/MIDIDataHandler.h"

//==============================================================================

class MainComponent  : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() noexcept override;

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
    SliderLookAndFeel sliderlookandfeel;

    //Toggle Buttons and Functions
    bool isConnectionsToggled = true, isCalibrationToggled = false,
         isBlutoothToggled = false;

    TextButton connectionsbutton, calibrationbutton, BluetoothButton;

    TextButton* pConnectionsButton = &connectionsbutton;
    TextButton* pCalibrationButton = &calibrationbutton;
    TextButton* pBluetoothButton = &BluetoothButton;
 
    //std::vector<juce::Button> SwatchButtons;

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
    std::shared_ptr<BluetoothConnectionManager> bluetoothconnection;

    // Gestural and MIDI Control
    std::shared_ptr<GestureManager> gesturemanager;
    std::shared_ptr<MIDIHandler> midihandler;

private:

    int functioncount = 0;

public:

    // UI Paramaters
    Slider BPMSlider;
    Label BPMSliderLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
