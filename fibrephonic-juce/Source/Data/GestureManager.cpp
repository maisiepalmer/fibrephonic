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

    DATA.XData.clear();
    DATA.YData.clear();
    DATA.ZData.clear();
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { PollGestures(); } // pollcount++; DBG(pollcount); }

void GestureManager::PollGestures()
{
    getConnectionManagerValues();
    fillDataVectors(&DATA.accXData, &DATA.accYData, &DATA.accZData, &DATA.XData, &DATA.YData, &DATA.ZData, &DATA.gX, &DATA.gY, &DATA.gZ, &DATA.accX, &DATA.accY, &DATA.accZ);

    perform1DWaveletTransform(DATA.xApprox, DATA.xDetail,
                              DATA.yApprox, DATA.yDetail,
                              DATA.zApprox, DATA.zDetail);

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
    // Vector runnoff if not resized in this function
    xaccdata->resize(DATAWINDOW);
    yaccdata->resize(DATAWINDOW);
    zaccdata->resize(DATAWINDOW);
    xdata->resize(DATAWINDOW);
    ydata->resize(DATAWINDOW);
    zdata->resize(DATAWINDOW);

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

void processAxis(std::vector<double>& input,
                        std::string wavelet,
                                 int levels,
std::vector<double>& reconstructed,
std::vector<double>& approx,
std::vector<double>& detail)

{
    std::vector<double> coeffs, bookkeeping, lengths;
    std::vector<int> idwt_lengths;

    // Perform DWT
    dwt(input, levels, wavelet, coeffs, bookkeeping, lengths);

    // Extract coefficients
    if (lengths.size() >= 2) {
        approx.assign(coeffs.begin(), coeffs.begin() + lengths[0]);
        detail.assign(coeffs.begin() + lengths[0], coeffs.begin() + lengths[0] + lengths[1]);
    }
    
    /* Modify DATA.XYZ Here....







    */

    // Reconstruct
    idwt_lengths.assign(lengths.begin(), lengths.end());
    reconstructed = input;  // Start from original size
    idwt(coeffs, bookkeeping, wavelet, reconstructed, idwt_lengths);
}

void GestureManager::perform1DWaveletTransform(std::vector<double>& xaccapprox,
                                               std::vector<double>& xaccdetail,
                                               std::vector<double>& yaccapprox,
                                               std::vector<double>& yaccdetail,
                                               std::vector<double>& zaccapprox,
                                               std::vector<double>& zaccdetail)
{
    int levels = 1;
    std::string wavelet = "haar"; // db1

    // Reconstructed versions will overwrite the original DATA vectors
    processAxis(DATA.accXData, wavelet, levels, DATA.accXData, xaccapprox, xaccdetail);
    processAxis(DATA.accYData, wavelet, levels, DATA.accYData, yaccapprox, yaccdetail);
    processAxis(DATA.accZData, wavelet, levels, DATA.accZData, zaccapprox, zaccdetail);

    for (int i = 0; i < DATA.accXData.size(); i++) {
        DBG(DATA.accXData[i]);
    }
}



