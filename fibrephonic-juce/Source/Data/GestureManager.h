/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B 

  ==============================================================================
*/

#define IMUINERTIALREFRESH 400

#pragma once

#define WAVE_EXPORT
#include "Wavelib/wavelet2d.h"

class BluetoothConnectionManager; // Forward Declare Bluetooth Manager Class, Avoids Header Clash in Main.

class GestureManager 
{
private:
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;

    float gX, gY, gZ;
    float accX, accY, accZ;
    float jerkX, jerkY, jerkZ;

    enum Gesture {
        PITCH,
        ROLL,
        YAW,
        TAP,
        STROKE
    };

private:

    

public:

    GestureManager(std::shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance)
        : bluetoothConnection(std::move(BluetoothConnectionManagerInstance)) 
    {
        gX = gY = gZ = accX = accY = accZ = 
        jerkX = jerkY = jerkZ = 0;
    }

    inline float CalculateJerk(float changingValue) {

        float Derivative = changingValue;

        // Get Derivative of input value eg acceleration...


        return Derivative;
    }

    inline void getConnectionManagerValues() 
    {
        gX = bluetoothConnection->getGyroscopeX();
        gY = bluetoothConnection->getGyroscopeY();
        gZ = bluetoothConnection->getGyroscopeZ();

        accX = bluetoothConnection->getAccelerationX();
        accY = bluetoothConnection->getAccelerationY();
        accZ = bluetoothConnection->getAccelerationZ();
    }

    inline void PollGestures()
    {
        getConnectionManagerValues();

        
    }
};