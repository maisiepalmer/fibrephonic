/*
  ==============================================================================

    OSCManager.h
    Created: 12 Mar 2025 3:38:57pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_osc/juce_osc.h>
//#include "Helpers.h"

using namespace juce;

class OSCManager : private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>
{
public:
    //==============================================================================
    OSCManager()
    {
        sender.connect("127.0.0.1", 7400);
        receiver.addListener(this);
    }
  
    ~OSCManager()
    {
        sender.disconnect();
        receiver.removeListener(this);
    }

    //==============================================================================
    void sendMessage(std::array<float, 3> acc, std::array<float, 3> gyro)
    {
        OSCAddressPattern location{ "/wrist" }; // getLocationTag()
        OSCMessage message{ location, acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2] };
        sender.send(message);
    }

    //==============================================================================
    void oscMessageReceived(const OSCMessage& message) override
    {

    }

private:
    //==============================================================================
    OSCSender sender;
    OSCReceiver receiver;

    OSCBundle makeBundle(OSCAddressPattern pattern, std::array<float, 3> values)
    {
        OSCBundle tempBundle;
        for (int i = 0; i < 3; i++)
        {
            OSCMessage message{ pattern, values[i] };
            tempBundle.addElement(OSCBundle::Element(message));
        }
        return tempBundle;
    }
};