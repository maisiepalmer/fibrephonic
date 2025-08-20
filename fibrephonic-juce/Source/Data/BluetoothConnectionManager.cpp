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
    
    bluetoothConnectionInfo = std::make_unique<ximu3::BluetoothConnectionInfo>("COM11");
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
    // Clear any previous devices and scan for new ones.
    deviceList.clear();
    deviceList = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);

    if (deviceList.empty())
    {
        DBG("No Bluetooth devices found.");
        isConnected = false;
    }
    else
    {
        DBG("Found " << deviceList.size() << " Bluetooth devices. Attempting to connect to the first one...");
        selectedDevice = deviceList[0];
        isConnected = false; // Set to false initially, will be set to true on successful connection
        startThread(); // Start the connection thread
    }
}

void BluetoothConnectionManager::stopConnection()
{
    signalThreadShouldExit();
    stopThread(500);
    isConnected = false;
    DBG("Connection stopped.");
}

//==============================================================================
void BluetoothConnectionManager::run()
{
    if (threadShouldExit() || deviceList.empty())
    {
        isConnected = false;
        return;
    }
    
    auto connectionInfoPtr = ximu3::connectionInfoFrom(selectedDevice);
    if (!connectionInfoPtr)
    {
        DBG("Failed to create connection info for the selected device.");
        isConnected = false;
        return;
    }
    
    // Run the connection in a non-blocking way
    connectionHandler->runConnection(*connectionInfoPtr, [this]() { return threadShouldExit(); });

    // Assuming a successful connection if runConnection returns, or if data starts flowing
    // For a real-world application, you might use a flag set by a callback
    // to confirm a connection is truly established.
    isConnected = true;
    DBG("Connection established with device: " << selectedDevice.device_name);

    // Keep the thread alive while the connection is active
    while (!threadShouldExit())
    {
        // Update values from the connection
        gyroscopeX = connectionHandler->getX();
        gyroscopeY = connectionHandler->getY();
        gyroscopeZ = connectionHandler->getZ();
        
        accelerationX = connectionHandler->getAccX();
        accelerationY = connectionHandler->getAccY();
        accelerationZ = connectionHandler->getAccZ();
        
        wait(pollRate); // Wait for the next poll
    }
    
    DBG("Thread exiting gracefully.");
}
