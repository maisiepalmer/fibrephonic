/*
  ==============================================================================

    GestureManager.cpp
    Created: 24 Jun 2025 2:18:38pm
    Author:  Joseph B

  ==============================================================================
*/

#include "GestureManager.h"
#include "BluetoothConnectionManager.h"

GestureManager::GestureManager(std::shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance)
    : bluetoothConnection(std::move(BluetoothConnectionManagerInstance))
{
    gX = gY = gZ = accX = accY = accZ = jerkX = jerkY = jerkZ = 0;

    accXData.resize(DATAWINDOW);
    accYData.resize(DATAWINDOW);
    accZData.resize(DATAWINDOW);

    cAx.resize(DATAWINDOW);
    cAy.resize(DATAWINDOW);
    cAz.resize(DATAWINDOW);

    cDx.resize(DATAWINDOW);
    cDy.resize(DATAWINDOW);
    cDz.resize(DATAWINDOW);
}

GestureManager::~GestureManager()
{
    accXData.clear();
    accYData.clear();
    accZData.clear();
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { PollGestures(); }

void GestureManager::PollGestures()
{
    getConnectionManagerValues();
    fillDataVectors(&accXData, &accYData, &accZData, &gX, &gY, &gZ);

    for (int i = 0; i < DATAWINDOW; i++) {
        DBG(accXData[i]);
    }
}

void GestureManager::getConnectionManagerValues()
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

void GestureManager::fillDataVectors(std::vector<double>* xaccdata,
                                     std::vector<double>* yaccdata,
                                     std::vector<double>* zaccdata,
                                                      double* accx,
                                                      double* accy,
                                                      double* accz)
{
    for (int i = 0; i < DATAWINDOW; i++)
    {
        (*xaccdata)[i] = static_cast<double>(*accx);
        (*yaccdata)[i] = static_cast<double>(*accy);
        (*zaccdata)[i] = static_cast<double>(*accz);
    }
}

void GestureManager::perform1DWaveletTransform(std::vector<double>* xaccdata,
                                               std::vector<double>* yaccdata,
                                               std::vector<double>* zaccdata)
{
    
}

