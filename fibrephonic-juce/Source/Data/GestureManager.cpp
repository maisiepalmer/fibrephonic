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
    DATA.gX = DATA.gY = DATA.gZ = 
    DATA.accX = DATA.accY = DATA.accZ = 
    DATA.jerkX = DATA.jerkY = DATA.jerkZ = 0;

    DATA.accXData.resize(DATAWINDOW);
    DATA.accYData.resize(DATAWINDOW);
    DATA.accZData.resize(DATAWINDOW);

    DATA.XData.resize(DATAWINDOW);
    DATA.YData.resize(DATAWINDOW);
    DATA.ZData.resize(DATAWINDOW);
}

GestureManager::~GestureManager()
{
    DATA.accXData.clear();
    DATA.accYData.clear();
    DATA.accZData.clear();
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { PollGestures(); } // pollcount++; DBG(pollcount); }

void GestureManager::PollGestures()
{
    getConnectionManagerValues();
    fillDataVectors(&DATA.accXData, &DATA.accYData, &DATA.accZData, &DATA.XData, &DATA.YData, &DATA.ZData, &DATA.gX, &DATA.gY, &DATA.gZ, &DATA.accX, &DATA.accY, &DATA.accZ);

    perform1DWaveletTransform(DATA.accXData, DATA.accYData, DATA.accZData);
   
    /*
    for (int i = 0; i < DATAWINDOW; i++) {
        DBG(XData[i]);
    }
    */
}

void GestureManager::getConnectionManagerValues()
{
    if (!bluetoothConnection)
    {
        DBG("BluetoothConnectionManager is null!");
        return;
    }

    DATA.gX = bluetoothConnection->getGyroscopeX();
    DATA.gY = bluetoothConnection->getGyroscopeY();
    DATA.gZ = bluetoothConnection->getGyroscopeZ();

    DATA.accX = bluetoothConnection->getAccelerationX();
    DATA.accY = bluetoothConnection->getAccelerationY();
    DATA.accZ = bluetoothConnection->getAccelerationZ();
}

void GestureManager::fillDataVectors(std::vector<double>* xaccdata,
                                     std::vector<double>* yaccdata,
                                     std::vector<double>* zaccdata,
                                     std::vector<double>* xdata,
                                     std::vector<double>* ydata,
                                     std::vector<double>* zdata,
                                                      double* x,
                                                      double* y,
                                                      double* z,
                                                      double* accx,
                                                      double* accy,
                                                      double* accz)
{
    for (int i = 0; i < DATAWINDOW; i++)
    {
        // Populate Acceleration 
        (*xaccdata)[i] = static_cast<double>(*accx);
        (*yaccdata)[i] = static_cast<double>(*accy);
        (*zaccdata)[i] = static_cast<double>(*accz);

        (*xdata)[i] = static_cast<double>(*x);
        (*ydata)[i] = static_cast<double>(*y);
        (*zdata)[i] = static_cast<double>(*z);
    }
}

void GestureManager::perform1DWaveletTransform(std::vector<double>& xaccdata,
                                               std::vector<double>& xaccapprox,
                                               std::vector<double>& xaccdetail)
{
    int levels = 1;
    std::string wavelet = "Haar";
    std::vector<double> coeffs, lengths;
    std::vector<int> bookkeeping;

    //coeffs.resize(DATAWINDOW);
    
   // dwt(xaccdata, levels, wavelet, coeffs, lengths, bookkeeping);
    
    /*
    for (int i = 0; i < coeffs.size(); i++) {
        DBG(coeffs[i]);
    }
    */

    /*
    for (int i = 0; i < DATA.accXData.size(); i++) {
        DBG(DATA.accXData[i]);
    }
    */
}


