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

class BluetoothConnectionManager : public Connection, public Thread
{
private:

    const int pollRate = 125;
    
    std::unique_ptr<ximu3::BluetoothConnectionInfo> bluetoothconnectioninfo;

    float gX, gY, gZ;
    float accX, accY, accZ;

    bool isConnected = false;

public:

    inline void setGyroscopeValues(float x, float y, float z) { gX = x; gY = y; gZ = z; }
    inline void setConnectionbool(bool MainBool) { isConnected = MainBool; }

    inline float getGyroscopeX() { return gX; }
    inline float getGyroscopeY() { return gY; }
    inline float getGyroscopeZ() { return gZ; }

    inline float getAccelerationX() { return accX; }
    inline float getAccelerationY() { return accY; }
    inline float getAccelerationZ() { return accZ; }


public:

    BluetoothConnectionManager() : Thread("Bluetooth Connection Thread") 
    {
        gX = gY = gZ = 0;
        accX = accY = accZ = 0;

        bluetoothconnectioninfo = std::make_unique<ximu3::BluetoothConnectionInfo>("COM11");
    }

    ~BluetoothConnectionManager() 
    {                
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

                gX = getX(); gY = getY(); gZ = getZ();
                accX = getaccX(); accY = getaccY(); accZ = getaccZ();
            }

            wait(pollRate);
        }
    }
};