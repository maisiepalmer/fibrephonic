/*
  ==============================================================================

    BluetoothConnectionManager.h
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
        stopTimer();                  
        signalThreadShouldExit();     
        stopThread(500);                      
    }

    void run() override 
    {
        while (!threadShouldExit()) 
        {
            if (helpers::yesOrNo("Search for connections?"))
            {
                const auto devices = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);

                if (devices.empty())
                {
                    DBG("No Bluetooth connections available");
                }

                DBG("Found " << devices[0].device_name << " " << devices[0].serial_number);

                const auto connectionInfo = ximu3::connectionInfoFrom(devices[0]);

                runconnection(*connectionInfo);
            }
            else
            {
                const ximu3::BluetoothConnectionInfo connectionInfo("COM1"); // replace with actual connection info

                runconnection(connectionInfo);
            }

            wait(pollRate);
        }
    }

private:

    void timerCallback() override {

        // Use for any additional function and management, ensure sync with bluetooth thread...

    }
};