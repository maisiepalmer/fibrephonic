#pragma once

#include <JuceHeader.h>
#include "UI/LookandFeel.h"
#include "Data/BluetoothConnectionManager.h"
#include "Data/SerialPort.h"
#include "Data/GestureManager.h"
#include "Data/MIDIDataHandler.h"

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
    
    //==============================================================================
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;
    std::shared_ptr<GestureManager> gestureManager;
    std::shared_ptr<MIDIHandler> midiHandler;

private:
    //==============================================================================
    //Custom Look and Feel Instances
    ButtonLookandFeel buttonlookandfeel;
    RoundedButtonLookandFeel roundedbuttonlookandfeel;
    SliderLookAndFeel sliderlookandfeel;

    //Toggle Buttons and Functions
    int windowSelected = 0;
    bool isBluetoothToggled = false;

    TextButton connections, calibration, bluetooth;

    TextButton* pConnectionsButton = &connections;
    TextButton* pCalibrationButton = &calibration;
    TextButton* pBluetoothButton = &bluetooth;
 
    //std::vector<juce::Button> SwatchButtons;

    //Parameters and XML
    ValueTree presetTree, parameterTree;

    std::vector<ValueTree> swatchTree;

    File xmlFile;

    // Serial 
    // std::unique_ptr<SerialPort> serialPort;
    // std::unique_ptr<SerialPortInputStream> inputStream;
    bool serialConnected = false;
    
    int functionCount = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
