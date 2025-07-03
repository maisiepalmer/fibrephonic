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

    perform1DWaveletTransform();

    /*
    for (int i = 0; i < DATA.yDetail.size(); i++) {
        DBG(DATA.accYData[i]);
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

void GestureManager::decomposeAxis(std::vector<double>& input,
                                          std::string wavelet,
                                                   int levels,

    std::vector<double>& coeffs,
    std::vector<double>& approx,
    std::vector<double>& detail,
    std::vector<double>& bookkeeping,
    std::vector<double>& lengths)
{
    dwt(input, levels, wavelet, coeffs, bookkeeping, lengths);

    if (lengths.size() >= 2) {
        approx.assign(coeffs.begin(), coeffs.begin() + lengths[0]);
        detail.assign(coeffs.begin() + lengths[0], coeffs.begin() + lengths[0] + lengths[1]);
    }
}

void GestureManager::reconstructAxis(std::vector<double>& coeffs,
                     std::vector<double>& approx,
                     std::vector<double>& detail,
                     std::vector<double>& bookkeeping,
                     std::vector<double>& lengths,
                     std::string wavelet,
                     std::vector<double>& reconstructed)
{
    // Update coeffs with modified approx and detail
    std::copy(approx.begin(), approx.end(), coeffs.begin());
    std::copy(detail.begin(), detail.end(), coeffs.begin() + lengths[0]);

    std::vector<int> idwt_lengths(lengths.begin(), lengths.end());

    idwt(coeffs, bookkeeping, wavelet, reconstructed, idwt_lengths);
}

void GestureManager::perform1DWaveletTransform()
{
    std::string wavelet = "haar";
    int levels = 1;

    // Lambda for printing data
    auto vecToString = [](const std::vector<double>& vec) -> std::string {
        if (vec.empty())
            return "N/A";
        else
            return std::to_string(vec.back());
        };

    // --- X axis ---
    decomposeAxis(DATA.accXData, wavelet, levels, DATA.xCoeff, DATA.xApprox, DATA.xDetail, DATA.xBookkeeping, DATA.xLengths);

    // Modify DATA.xApprox and/or DATA.xDetail 

    reconstructAxis(DATA.xCoeff, DATA.xApprox, DATA.xDetail, DATA.xBookkeeping, DATA.xLengths, wavelet, DATA.accXData);

    std::string line = "X Approx: " + vecToString(DATA.xApprox) +
        ", X Detail: " + vecToString(DATA.xDetail) +
        ", X Reconstructed: " + vecToString(DATA.accXData);
    DBG(line);

    // --- Y axis ---
    decomposeAxis(DATA.accYData, wavelet, levels, DATA.yCoeff, DATA.yApprox, DATA.yDetail, DATA.yBookkeeping, DATA.yLengths);

    // Modify DATA.yApprox and/or DATA.yDetail 

    reconstructAxis(DATA.yCoeff, DATA.yApprox, DATA.yDetail, DATA.yBookkeeping, DATA.yLengths, wavelet, DATA.accYData);

    line = "Y Approx: " + vecToString(DATA.yApprox) +
        ", Y Detail: " + vecToString(DATA.yDetail) +
        ", Y Reconstructed: " + vecToString(DATA.accYData);
    DBG(line);

    // --- Z axis ---
    decomposeAxis(DATA.accZData, wavelet, levels, DATA.zCoeff, DATA.zApprox, DATA.zDetail, DATA.zBookkeeping, DATA.zLengths);

    // Modify DATA.zApprox and/or DATA.zDetail 

    reconstructAxis(DATA.zCoeff, DATA.zApprox, DATA.zDetail, DATA.zBookkeeping, DATA.zLengths, wavelet, DATA.accZData);

    line = "Z Approx: " + vecToString(DATA.zApprox) +
        ", Z Detail: " + vecToString(DATA.zDetail) +
        ", Z Reconstructed: " + vecToString(DATA.accZData);
    DBG(line);
}





