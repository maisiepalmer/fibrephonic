/*
  ==============================================================================

    BluetoothConnectionManager.h
    Created: 11 Jun 2025 10:32:59am
    Author:  Joseph B

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../imuExamples/Connection.h"
#include "x-IMU3/Cpp/ConnectionInfo.hpp"

class BluetoothConnectionManager : public Connection, public Thread
{
private:

    std::unique_ptr<ximu3::BluetoothConnectionInfo> bluetoothconnectioninfo;
    std::unique_ptr<Connection> ConnectionInstance;

private:

    const int pollRate = 125;

    float gX, gY, gZ;
    float accX, accY, accZ;

    bool isConnected = false;

public:

    inline void setGyroscopeValues(double x, double y, double z) { gX = x; gY = y; gZ = z; }
    inline void setAccelerometerValues(double x, double y, double z) { accX = x; accY = y; accZ = z; }

    inline void setConnectionbool(bool MainBool) { isConnected = MainBool; }

    inline double getGyroscopeX() { return gX; }
    inline double getGyroscopeY() { return gY; }
    inline double getGyroscopeZ() { return gZ; }

    inline double getAccelerationX() { return accX; }
    inline double getAccelerationY() { return accY; }
    inline double getAccelerationZ() { return accZ; }

public:

    BluetoothConnectionManager() : Thread("Bluetooth Connection Thread")
    {
        gX = gY = gZ = 0;
        accX = accY = accZ = 0;

        bluetoothconnectioninfo = std::make_unique<ximu3::BluetoothConnectionInfo>("COM11");
        ConnectionInstance = std::make_unique<Connection>(this);
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
            auto devices = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);
            if (devices.empty())
            {
                DBG("No Bluetooth connections available");
            }
            else
            {
                DBG("Found " << devices[0].device_name << " " << devices[0].serial_number);

                auto connectionInfoPtr = ximu3::connectionInfoFrom(devices[0]);
                if (!connectionInfoPtr)
                {
                    DBG("Failed to create connection info.");
                    return;
                }

                if (threadShouldExit()) break;

                ConnectionInstance->runconnection(*connectionInfoPtr);  

                gX = ConnectionInstance->getX(); 
                gY = ConnectionInstance->getX();
                gZ = ConnectionInstance->getX();

                accX = ConnectionInstance->getaccX(); 
                accY = ConnectionInstance->getaccY(); 
                accZ = ConnectionInstance->getaccZ();
            }

            wait(pollRate);
        }
    }
};
