/*
 ==============================================================================
 
 GestureManager.cpp
 Created: 24 Jun 2025 2:18:38pm
 Author:  Joseph B
 
 ==============================================================================
 */

#include "GestureManager.h"
#include "BluetoothConnectionManager.h"

// JUCE and standard library includes
#include <juce_osc/juce_osc.h>

GestureManager::GestureManager(std::shared_ptr<BluetoothConnectionManager> bluetoothConnectionManagerInstance)
: bluetoothConnection(bluetoothConnectionManagerInstance)
{
    // Initialize data streams to zero
    data.gx = data.gy = data.gz =
    data.accX = data.accY = data.accZ =
    data.jerkX = data.jerkY = data.jerkZ = 0;
    
    // Resize data vectors to the defined window size
    data.accXData.resize(DATAWINDOW);
    data.accYData.resize(DATAWINDOW);
    data.accZData.resize(DATAWINDOW);
    
    data.xData.resize(DATAWINDOW);
    data.yData.resize(DATAWINDOW);
    data.zData.resize(DATAWINDOW);
    
    data.xScaled.resize(DATAWINDOW);
    data.yScaled.resize(DATAWINDOW);
    data.zScaled.resize(DATAWINDOW);
    
    gesture = NO_GESTURE;
    
    // Connect the OSC sender to the local machine's port 5006.
    if (!oscSender.connect("127.0.0.1", 5006)) {
        DBG("Error: Could not connect to OSC port!");
    }
}

GestureManager::~GestureManager()
{
    // The C++ standard library handles the cleanup of vectors and other
    // members automatically, so explicit clear() calls are not needed.
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { pollGestures(); }

void GestureManager::pollGestures()
{
    // 1. Get raw data from the Bluetooth manager
    getConnectionManagerValues();
    
    // 2. Fill the data vectors for processing
    fillDataVectors(data.accXData, data.accYData, data.accZData,
                    data.xData, data.yData, data.zData,
                    data.gx, data.gy, data.gz,
                    data.accX, data.accY, data.accZ);
    
    // 3. Perform the wavelet transform and filtering
    perform1DWaveletTransform();
    
    // 4. Scale the *processed* data for OSC/MIDI
    scaleAndCopy(data.accXData, data.accYData, data.accZData, data.xScaled, data.yScaled, data.zScaled);
    
    // 5. Send the scaled, processed data as an OSC bundle
    sendProcessedDataAsBundle(data.xScaled, data.yScaled, data.zScaled);
}

void GestureManager::getConnectionManagerValues()
{
    if (!bluetoothConnection)
    {
        DBG("BluetoothConnectionManager is null!");
        return;
    }
    
    data.gx = bluetoothConnection->getGyroscopeX();
    data.gy = bluetoothConnection->getGyroscopeY();
    data.gz = bluetoothConnection->getGyroscopeZ();
    
    data.accX = bluetoothConnection->getAccelerationX();
    data.accY = bluetoothConnection->getAccelerationY();
    data.accZ = bluetoothConnection->getAccelerationZ();
}

void GestureManager::fillDataVectors(std::vector<double>& xaccdata,
                                     std::vector<double>& yaccdata,
                                     std::vector<double>& zaccdata,
                                     std::vector<double>& xdata,
                                     std::vector<double>& ydata,
                                     std::vector<double>& zdata,
                                     double& x,
                                     double& y,
                                     double& z,
                                     double& accx,
                                     double& accy,
                                     double& accz)
{
    const int scaleVal = 1;
    
    if (xaccdata.size() == DATAWINDOW) xaccdata.erase(xaccdata.begin());
    if (yaccdata.size() == DATAWINDOW) yaccdata.erase(yaccdata.begin());
    if (zaccdata.size() == DATAWINDOW) zaccdata.erase(zaccdata.begin());
    
    if (xdata.size() == DATAWINDOW) xdata.erase(xdata.begin());
    if (ydata.size() == DATAWINDOW) ydata.erase(ydata.begin());
    if (zdata.size() == DATAWINDOW) zdata.erase(zdata.begin());
    
    xaccdata.push_back(accx * scaleVal);
    yaccdata.push_back(accy * scaleVal);
    zaccdata.push_back(accz * scaleVal);
    
    xdata.push_back(x);
    ydata.push_back(y);
    zdata.push_back(z);
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
    coeffs.clear();
    approx.clear();
    detail.clear();
    bookkeeping.clear();
    lengths.clear();
    
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
    size_t totalLength = 0;
    for (auto len : lengths) totalLength += static_cast<size_t>(len);
    coeffs.resize(totalLength);
    
    std::copy(approx.begin(), approx.end(), coeffs.begin());
    std::copy(detail.begin(), detail.end(), coeffs.begin() + lengths[0]);
    
    std::vector<int> idwt_lengths(lengths.begin(), lengths.end());
    
    idwt(coeffs, bookkeeping, wavelet, reconstructed, idwt_lengths);
}

void GestureManager::softThresholding(std::vector<double>& detailCoeffs)
{
    std::vector<double> absCoeffs(detailCoeffs.size());
    
    for (size_t i = 0; i < detailCoeffs.size(); ++i)
        absCoeffs[i] = std::abs(detailCoeffs[i]);
    
    std::sort(absCoeffs.begin(), absCoeffs.end());
    double median;
    size_t n = absCoeffs.size();
    if (n % 2 == 0)
        median = (absCoeffs[n / 2 - 1] + absCoeffs[n / 2]) / 2.0;
    else
        median = absCoeffs[n / 2];
    
    double nsd = median / 0.6745;
    double threshold = nsd * std::sqrt(2.0 * std::log(n));
    
    for (auto& x : detailCoeffs)
        x = std::copysign(std::max(std::abs(x) - threshold, 0.0), x);
}

void GestureManager::modifyWaveletDomain(std::vector<double>& xApprox, const std::vector<double>& xDetail,
                                         std::vector<double>& yApprox, const std::vector<double>& yDetail,
                                         std::vector<double>& zApprox, const std::vector<double>& zDetail)
{
    std::vector<double> xDetailCopy = xDetail;
    std::vector<double> yDetailCopy = yDetail;
    std::vector<double> zDetailCopy = zDetail;

    softThresholding(xDetailCopy);
    softThresholding(yDetailCopy);
    softThresholding(zDetailCopy);
    
    {
        double damping = 0.95;
        
        auto clampCoeff = [](double& val, double minVal, double maxVal) {
            if (val < minVal) val = minVal;
            else if (val > maxVal) val = maxVal;
        };
        
        for (auto& val : xApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : xDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        
        for (auto& val : yApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : yDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        
        for (auto& val : zApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : zDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
    }
}

GestureManager::Gesture GestureManager::identifyGesture(const std::vector<double>& xApprox, const std::vector<double>& xDetail,
                                                       const std::vector<double>& yApprox, const std::vector<double>& yDetail,
                                                       const std::vector<double>& zApprox, const std::vector<double>& zDetail)
{
    const double EPSILON = 1e-9;
    
    for (size_t i = 0; i < xApprox.size(); ++i)
    {
        if (std::abs(xApprox[i]) > EPSILON ||
            std::abs(xDetail[i]) > EPSILON ||
            std::abs(yApprox[i]) > EPSILON ||
            std::abs(yDetail[i]) > EPSILON ||
            std::abs(zApprox[i]) > EPSILON ||
            std::abs(zDetail[i]) > EPSILON)
        {
            return NO_GESTURE;
        }
    }
    
    return NO_GESTURE;
}

void GestureManager::perform1DWaveletTransform()
{
    std::string wavelet = "db4";
    int levels = 1;
    
    decomposeAxis(data.accXData, wavelet, levels, data.xCoeff, data.xApprox, data.xDetail, data.xBookkeeping, data.xLengths);
    decomposeAxis(data.accYData, wavelet, levels, data.yCoeff, data.yApprox, data.yDetail, data.yBookkeeping, data.yLengths);
    decomposeAxis(data.accZData, wavelet, levels, data.zCoeff, data.zApprox, data.zDetail, data.zBookkeeping, data.zLengths);
    
    modifyWaveletDomain(data.xApprox, data.xDetail, data.yApprox, data.yDetail, data.zApprox, data.zDetail);
    
    gesture = identifyGesture(data.xApprox, data.xDetail, data.yApprox, data.yDetail, data.zApprox, data.zDetail);
    
    reconstructAxis(data.xCoeff, data.xApprox, data.xDetail, data.xBookkeeping, data.xLengths, wavelet, data.accXData);
    reconstructAxis(data.yCoeff, data.yApprox, data.yDetail, data.yBookkeeping, data.yLengths, wavelet, data.accYData);
    reconstructAxis(data.zCoeff, data.zApprox, data.zDetail, data.zBookkeeping, data.zLengths, wavelet, data.accZData);
}

std::vector<double> GestureManager::normaliseData(double min, double max, const std::vector<double>& input)
{
    std::vector<double> rescaled;
    rescaled.reserve(input.size());
    
    if (min == max) {
        rescaled.assign(input.size(), 64.0);
        return rescaled;
    }
    
    for (double x : input) {
        double normalised = (x - min) / (max - min);
        double scaled = normalised * 126.0 + 1.0;
        rescaled.push_back(scaled);
    }
    
    return rescaled;
}

void GestureManager::scaleAndCopy(std::vector<double>& xaccdata, std::vector<double>& yaccdata, std::vector<double>& zaccdata,
                                  std::vector<double>& xscale, std::vector<double>& yscale, std::vector<double>& zscale)
{
    if (xaccdata.empty() || yaccdata.empty() || zaccdata.empty()) {
        DBG("Data vectors empty!");
        return;
    }
    
    auto [minxT, maxxT] = std::minmax_element(xaccdata.begin(), xaccdata.end());
    auto [minyT, maxyT] = std::minmax_element(yaccdata.begin(), yaccdata.end());
    auto [minzT, maxzT] = std::minmax_element(zaccdata.begin(), zaccdata.end());
    
    double minxVal = *minxT, maxxVal = *maxxT;
    double minyVal = *minyT, maxyVal = *maxyT;
    double minzVal = *minzT, maxzVal = *maxzT;
    
    xscale = normaliseData(minxVal, maxxVal, xaccdata);
    yscale = normaliseData(minyVal, maxyVal, yaccdata);
    zscale = normaliseData(minzVal, maxzVal, zaccdata);
}

void GestureManager::sendProcessedDataAsBundle(const std::vector<double>& accelXData,
                                               const std::vector<double>& accelYData,
                                               const std::vector<double>& accelZData)
{
    juce::OSCBundle bundle;

    juce::OSCMessage accelXMessage("/imu/clean/accelX");
    for (double value : accelXData) {
        accelXMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelXMessage);

    juce::OSCMessage accelYMessage("/imu/clean/accelY");
    for (double value : accelYData) {
        accelYMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelYMessage);

    juce::OSCMessage accelZMessage("/imu/clean/accelZ");
    for (double value : accelZData) {
        accelZMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelZMessage);

    oscSender.send(bundle);
}
