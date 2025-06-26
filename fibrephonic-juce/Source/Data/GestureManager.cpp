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

    XData.resize(DATAWINDOW);
    YData.resize(DATAWINDOW);
    ZData.resize(DATAWINDOW);
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
    fillDataVectors(&accXData, &accYData, &accZData, &XData, &YData, &ZData, &gX, &gY, &gZ, &accX, &accY, &accZ);

    perform1DWaveletTransform(accXData, accYData, accZData);

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
    
    dwt(xaccdata, levels, wavelet, coeffs, lengths, bookkeeping);
    
    DBG("Wavelet Coefficients:\n");
    for (auto c : coeffs)
        DBG(c << " ");
}


