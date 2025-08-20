/*
  ==============================================================================

    SerialConnectionManager.h
    Created: 19 Aug 2025 1:24:51pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SerialPort.h"

class SerialConnectionManager
{
    public:
    //==============================================================================
    SerialConnectionManager(std::shared_ptr<ValueTree> calibrationTree)
    : vt(valueTree)
    {
        
    }
    
    ~SerialConnectionManager()
    {
        
    }
    
    //==============================================================================
    juce::StringArray pollConnections()
    {
        
    }
    
    void selectDevice(int index)
    {
        selectedDevice = deviceList[index];
    }
    
    void run() override
    {
        
    }
    
    private:
    std::shared_ptr<ValueTree> vt;
    SerialPort port;
};
