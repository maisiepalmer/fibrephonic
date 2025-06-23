/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B 

  ==============================================================================
*/

#pragma once

#define WAVE_EXPORT

#define IMUINERTIALREFRESH 400
#define DATAWINDOW 256 // Needs to be power of 2

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "../Wavelib/wavelet2s.h"

class BluetoothConnectionManager; // Forward Declare Bluetooth Manager Class, Avoids Header Clash in Main.

class GestureManager 
{
private:
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;
    
    //BluetoothConnectionManager bluetoothConnection;

    float gX, gY, gZ;
    float accX, accY, accZ;
    float jerkX, jerkY, jerkZ;

    float *paccX = &accX, 
          *paccY = &accY, 
          *paccZ = &accZ;

    enum Gesture {
        PITCH,
        ROLL,
        YAW,
        TAP,
        STROKE
    };

private:

    // Data Vectors for Wavelet Trasform
    std::vector<double> accXData, accYData, accZData;

    // Approximation Coefficients
    std::vector<double> cAx, cAy, cAz;

    // Detail Coefficients 
    std::vector<double> cDx, cDy, cDz;

    std::vector<double> *paccXData = &accXData,
                        *paccYData = &accYData,
                        *paccZData = &accZData;
                        
public:

    GestureManager(std::shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance)
        : bluetoothConnection(std::move(BluetoothConnectionManagerInstance)) 
    {
        gX = gY = gZ = accX = accY = accZ = 
        jerkX = jerkY = jerkZ = 0;

        accXData.resize(DATAWINDOW); accYData.resize(DATAWINDOW); accZData.resize(DATAWINDOW);

        cAx.resize(DATAWINDOW); cAy.resize(DATAWINDOW); cAz.resize(DATAWINDOW);
        cDx.resize(DATAWINDOW); cDy.resize(DATAWINDOW); cDz.resize(DATAWINDOW);
    }

    ~GestureManager() 
    {
        accXData.clear(); 
        accYData.clear();
        accZData.clear();

        paccX = nullptr;
        paccY = nullptr;
        paccZ = nullptr;

        paccXData = nullptr;
        paccYData = nullptr;
        paccZData = nullptr;
    }

    inline void getConnectionManagerValues()
    {
        if (!bluetoothConnection)
        {
            DBG("BluetoothConnectionManager is null!");
            return;
        }

        gX = bluetoothConnection->getGyroscopeX();
        gY = bluetoothConnection->getGyroscopeY();
        gZ = bluetoothConnection->getGyroscopeZ();

        accX = bluetoothConnection->getAccelerationX();
        accY = bluetoothConnection->getAccelerationY();
        accZ = bluetoothConnection->getAccelerationZ();
    }

    inline void fillDataVectors(std::vector<double>* xaccdata, 
                                std::vector<double>* yaccdata, 
                                std::vector<double>* zaccdata,
                                                  float* accx,
                                                  float* accy,
                                                  float* accz)
    {
        for (int i = 0; i < DATAWINDOW; i++) {
            
            xaccdata->push_back(*accx);
            yaccdata->push_back(*accy);
            zaccdata->push_back(*accz);
        }
    }

    inline void perform1DWaveletTransform(std::vector<double>* xaccdata,
        std::vector<double>* yaccdata,
        std::vector<double>* zaccdata)
    {
        
    }

    inline void PollGestures()
    {
        getConnectionManagerValues();
        fillDataVectors(paccXData, paccYData, paccZData, paccX, paccY, paccZ);

        DBG("In function...");
    }
};