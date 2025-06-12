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
#include "x-IMU3/Cpp/ConnectionInfo.hpp"

class BluetoothConnectionManager : public Connection, public Thread, public Timer
{
private:

    const int pollRate = 125;
    
    std::unique_ptr<ximu3::BluetoothConnectionInfo> bluetoothconnectioninfo;

public:

    BluetoothConnectionManager() : Thread("Bluetooth Connection Thread") 
    {
        startTimerHz(pollRate);

        bluetoothconnectioninfo = std::make_unique<ximu3::BluetoothConnectionInfo>("COM11");
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
            const auto devices = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);

            if (devices.empty())
            {

                DBG("No Bluetooth connections available");
            }
            else
            {
                // Ensure that IMU isn't connected in IMU GUI or it will will not be found by scanFilter...

                DBG("Found " << devices[0].device_name << " " << devices[0].serial_number);
                const auto connectionInfo = ximu3::connectionInfoFrom(devices[0]);
                
                runconnection(*connectionInfo);
            }

            wait(pollRate);
        }
    }

private:

    void timerCallback() override {

        // Use for any additional function and management, ensure sync with bluetooth thread...

    }
};