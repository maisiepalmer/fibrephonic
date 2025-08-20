/*
  ==============================================================================

    Calibration.h
    Created: 19 Aug 2025 9:45:55am
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Identifiers.h"
#include "SwatchConnector.h"
#include "LEDSignal.h"

//==============================================================================
/*
*/
class Calibration  : public juce::Component, public ComboBox::Listener, public ValueTree::Listener
{
public:
    Calibration(std::shared_ptr<ValueTree> calibrationTree, std::shared_ptr<ValueTree> swatchesTree)
    : ct(calibrationTree)
    , st(swatchesTree)
    {
        ct->addListener(this);
        
        bluetoothConnections.setName("Bluetooth");
        addAndMakeVisible(bluetoothConnections);
        bluetoothConnections.addListener(this);
        pollBluetooth.setButtonText("Poll");
        addAndMakeVisible(pollBluetooth);
        pollBluetooth.onClick = [this]{
            // led orange
            ct->setProperty(Identifiers::Calibration::BluetoothPoll, true, nullptr);
        };
        
        serialConnections.setName("Serial");
        addAndMakeVisible(serialConnections);
        serialConnections.addListener(this);
        pollSerial.setButtonText("Poll");
        addAndMakeVisible(pollSerial);
        pollSerial.onClick = [this]{
            // led orange
            juce::StringArray options;
            serialConnections.clear();
            serialConnections.addItemList(options, 0);
            
            if (options.isEmpty())
            {
                // led red
                DBG("No serial devices found.");
            }
            else
            {
                // led green
                
            }
        };
    }

    ~Calibration() override
    {
        ct->removeListener(this);
    }

    void paint (juce::Graphics& g) override
    {
        // text IMU, Glove, MIDI
    }

    void resized() override
    {
        bluetoothConnections.setBounds(50, 50, 500, 50);
        pollBluetooth.setBounds(600, 50, 50, 50);
    }
    
    //==============================================================================
    void parseIMUData(const juce::String& data)
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
    void comboBoxChanged (juce::ComboBox *comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged->getName() == "Bluetooth")
        {
            int index = comboBoxThatHasChanged->getSelectedId();
            ct->setProperty(Identifiers::Calibration::BluetoothSelection, index, nullptr);
            
#pragma message("add these into the gesture and midi classes as listener callbacks")
//            gm->startPolling();
//            mh->startThread();
        }
    }
    
    //==============================================================================
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override
    {
        if(property == Identifiers::Calibration::BluetoothOptions)
        {
            juce::StringArray options = static_cast<juce::StringArray>(treeWhosePropertyHasChanged.getProperty(property));
            bluetoothConnections.clear();
            bluetoothConnections.addItemList(options, 0);
            
            if (options.isEmpty())
            {
                // led red
                DBG("No bluetooth devices found.");
            }
            else
            {
                // led green
                
            }
        }
    }

private:
    //==============================================================================
    std::shared_ptr<ValueTree> ct;
    std::shared_ptr<ValueTree> st;
    
    // menu
    juce::ComboBox bluetoothConnections, serialConnections;
    juce::TextButton pollBluetooth, pollSerial;
    LEDSignal imuLED, gloveLED;
    
    // swatches
    std::array<SwatchConnector, 8> swatchConnectors;
        // when clicked, change colour, you scan an nfc (change colour when detected), rename box
        // update tree
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Calibration)
};
