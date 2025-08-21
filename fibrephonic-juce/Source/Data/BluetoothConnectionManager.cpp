/*
 ==============================================================================
 
 BluetoothConnectionManager.cpp
 Created: 20 Aug 2025 5:34:45pm
 Author:  Maisie Palmer
 
 ==============================================================================
 */

#include "BluetoothConnectionManager.h"

//==============================================================================
BluetoothConnectionManager::BluetoothConnectionManager() : juce::Thread("Bluetooth Connection Thread")
{
    gyroscopeX = gyroscopeY = gyroscopeZ = 0;
    accelerationX = accelerationY = accelerationZ = 0;
    
    // The connection handler is created here with a pointer to this manager.
    connectionHandler = std::make_unique<Connection>(this);
}

BluetoothConnectionManager::~BluetoothConnectionManager()
{
    signalThreadShouldExit();
    stopThread(500);
}

//==============================================================================
void BluetoothConnectionManager::startConnection()
{
    startThread();
}

void BluetoothConnectionManager::stopConnection()
{
    signalThreadShouldExit();
    stopThread(500);
    isConnected = false;
    DBG("Connection stopped.");
}

void BluetoothConnectionManager::run()
{
    deviceList.clear();
    deviceList = ximu3::PortScanner::scan();
    
    if (threadShouldExit())
    {
        isConnected = false;
        return;
    }
    
    if (deviceList.empty())
    {
        DBG("No devices found.");
        isConnected = false;
        return;
    }
    
    DBG("Found " << deviceList.size() << " devices. Attempting to connect to the first one...");
    selectedDevice = deviceList[0];
    
    auto connectionInfoPtr = ximu3::connectionInfoFrom(selectedDevice);
    if (!connectionInfoPtr)
    {
        DBG("Failed to create connection info for the selected device.");
        isConnected = false;
        return;
    }
    
    // This callback will be called by the Connection class when the connection is open.
    auto onConnectionSuccess = [this]() {
        isConnected = true;
        DBG("Connection established with device.");
    };
    
    connectionHandler->runConnection(*connectionInfoPtr, [this]() { return threadShouldExit(); }, onConnectionSuccess);
    
    // Keep the thread alive while the connection is active.
    while (!threadShouldExit())
    {
        wait(pollRate);
    }
    
    DBG("Thread exiting gracefully.");
}
