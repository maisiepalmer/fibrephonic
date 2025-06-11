/*
  ==============================================================================

    BluetoothConnection.h
    Created: 11 Jun 2025 10:32:59am
    Author:  Joseph B

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "imuExamples/Connection.h"

class BluetoothConnectionManager : public Connection, public Thread, Timer
{
private:

    const int pollRate = 125;

public:

    BluetoothConnectionManager() : Thread("Bluetooth Connection Thread") 
    {
        startTimerHz(pollRate);
    }

    ~BluetoothConnectionManager() 
    {
        signalThreadShouldExit();
        stopTimer();
        stopThread(500);            
    }

    void run() override 
    {
        while (!threadShouldExit()) 
        {


            wait(pollRate);
        }
    }

private:

    void timerCallback() override {

        // Use for any additional function and management, ensure sync with bluetooth thread...

    }
};