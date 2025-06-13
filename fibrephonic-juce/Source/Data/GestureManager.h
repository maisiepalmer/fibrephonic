/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B 

  ==============================================================================
*/

#pragma once

class BluetoothConnectionManager; // Forward Declare Bluetooth Manager Class, Avoids Header Clash in Main.

class GestureManager 
{
private:
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;

public:
    GestureManager(std::shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance)
        : bluetoothConnection(std::move(BluetoothConnectionManagerInstance)) {}

    void PollGestures()
    {
        
    }
};