/*
 ==============================================================================
 
 GestureManager.cpp
 Created: 24 Jun 2025 2:18:38pm
 Author:  Joseph B
 
 ==============================================================================
 */

#include "GestureManager.h"
#include "ConnectionManager.h"

//==============================================================================
GestureManager::GestureManager()
{
    // Initialize data streams to zero
    data.gx_raw = data.gy_raw = data.gz_raw =
    data.accX_raw = data.accY_raw = data.accZ_raw =
    data.jerkX = data.jerkY = data.jerkZ = 0;
    
    // Resize data vectors to the defined window size
    data.accelXData.resize(DATAWINDOW);
    data.accelYData.resize(DATAWINDOW);
    data.accelZData.resize(DATAWINDOW);
    
    data.gyroXData.resize(DATAWINDOW);
    data.gyroYData.resize(DATAWINDOW);
    data.gyroZData.resize(DATAWINDOW);
    
    data.accelXScaled.resize(DATAWINDOW);
    data.accelYScaled.resize(DATAWINDOW);
    data.accelZScaled.resize(DATAWINDOW);
    
    // Resize new gyroscope scaled vectors
    data.gyroXScaled.resize(DATAWINDOW);
    data.gyroYScaled.resize(DATAWINDOW);
    data.gyroZScaled.resize(DATAWINDOW);
    
    gesture = NO_GESTURE;
    
    // Connect the OSC sender to the local machine's port 5006.
    if (!oscSender.connect("192.169.1.2", 5006)) {
        DBG("Error: Could not connect to OSC port!");
    }
}

GestureManager::~GestureManager()
{
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { pollGestures(); }

void GestureManager::pollGestures()
{
    // Get raw data from the Connection manager
    getConnectionManagerValues();
    
    // Fill the data vectors for processing
    fillDataVectors(data.accelXData, data.accelYData, data.accelZData,
                    data.gyroXData, data.gyroYData, data.gyroZData,
                    data.gx_raw, data.gy_raw, data.gz_raw,
                    data.accX_raw, data.accY_raw, data.accZ_raw);
    
    // Scale the raw gyroscope data
    scaleAndCopy(data.gyroXData, data.gyroXScaled);
    scaleAndCopy(data.gyroYData, data.gyroYScaled);
    scaleAndCopy(data.gyroZData, data.gyroZScaled);
    
    // Perform the wavelet transform and filtering on accelerometer data
    perform1DWaveletTransform();
    
    // Scale the *processed* accelerometer data
    scaleAndCopy(data.accelXData, data.accelXScaled);
    scaleAndCopy(data.accelYData, data.accelYScaled);
    scaleAndCopy(data.accelZData, data.accelZScaled);
    
    // Send all the scaled data as an OSC bundle
    sendProcessedDataAsBundle(data.accelXScaled, data.accelYScaled, data.accelZScaled,
                              data.gyroXScaled, data.gyroYScaled, data.gyroZScaled);
}

void GestureManager::getConnectionManagerValues()
{
    // Correctly check if the weak_ptr is still valid by locking it.
    if (auto lockedManager = connectionManager.lock())
    {
        data.gx_raw = lockedManager->getGyroscopeX();
        data.gy_raw = lockedManager->getGyroscopeY();
        data.gz_raw = lockedManager->getGyroscopeZ();
        
        data.accX_raw = lockedManager->getAccelerationX();
        data.accY_raw = lockedManager->getAccelerationY();
        data.accZ_raw = lockedManager->getAccelerationZ();
    }
    else
    {
        DBG("ConnectionManager is no longer available!");
        return;
    }
}

void GestureManager::fillDataVectors(std::vector<double>& accelXData,
                                     std::vector<double>& accelYData,
                                     std::vector<double>& accelZData,
                                     std::vector<double>& gyroXData,
                                     std::vector<double>& gyroYData,
                                     std::vector<double>& gyroZData,
                                     double& gyroX_in,
                                     double& gyroY_in,
                                     double& gyroZ_in,
                                     double& accelX_in,
                                     double& accelY_in,
                                     double& accelZ_in)
{
    const int scaleVal = 1;
    
    if (accelXData.size() == DATAWINDOW) accelXData.erase(accelXData.begin());
    if (accelYData.size() == DATAWINDOW) accelYData.erase(accelYData.begin());
    if (accelZData.size() == DATAWINDOW) accelZData.erase(accelZData.begin());
    
    if (gyroXData.size() == DATAWINDOW) gyroXData.erase(gyroXData.begin());
    if (gyroYData.size() == DATAWINDOW) gyroYData.erase(gyroYData.begin());
    if (gyroZData.size() == DATAWINDOW) gyroZData.erase(gyroZData.begin());
    
    accelXData.push_back(accelX_in * scaleVal);
    accelYData.push_back(accelY_in * scaleVal);
    accelZData.push_back(accelZ_in * scaleVal);
    
    gyroXData.push_back(gyroX_in);
    gyroYData.push_back(gyroY_in);
    gyroZData.push_back(gyroZ_in);
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

void GestureManager::modifyWaveletDomain(std::vector<double>& accelXApprox, const std::vector<double>& accelXDetail,
                                         std::vector<double>& accelYApprox, const std::vector<double>& accelYDetail,
                                         std::vector<double>& accelZApprox, const std::vector<double>& accelZDetail)
{
    std::vector<double> accelXDetailCopy = accelXDetail;
    std::vector<double> accelYDetailCopy = accelYDetail;
    std::vector<double> accelZDetailCopy = accelZDetail;

    softThresholding(accelXDetailCopy);
    softThresholding(accelYDetailCopy);
    softThresholding(accelZDetailCopy);
    
    {
        double damping = 0.95;
        
        auto clampCoeff = [](double& val, double minVal, double maxVal) {
            if (val < minVal) val = minVal;
            else if (val > maxVal) val = maxVal;
        };
        
        for (auto& val : accelXApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : accelXDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        
        for (auto& val : accelYApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : accelYDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        
        for (auto& val : accelZApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : accelZDetailCopy) { val *= damping; clampCoeff(val, -1.0, 1.0); }
    }
}

GestureManager::Gesture GestureManager::identifyGesture(const std::vector<double>& accelXApprox, const std::vector<double>& accelXDetail,
                                                       const std::vector<double>& accelYApprox, const std::vector<double>& accelYDetail,
                                                       const std::vector<double>& accelZApprox, const std::vector<double>& accelZDetail)
{
    const double EPSILON = 1e-9;
    
    for (size_t i = 0; i < accelXApprox.size(); ++i)
    {
        if (std::abs(accelXApprox[i]) > EPSILON ||
            std::abs(accelXDetail[i]) > EPSILON ||
            std::abs(accelYApprox[i]) > EPSILON ||
            std::abs(accelYDetail[i]) > EPSILON ||
            std::abs(accelZApprox[i]) > EPSILON ||
            std::abs(accelZDetail[i]) > EPSILON)
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
    
    decomposeAxis(data.accelXData, wavelet, levels, data.accelXCoeff, data.accelXApprox, data.accelXDetail, data.accelXBookkeeping, data.accelXLengths);
    decomposeAxis(data.accelYData, wavelet, levels, data.accelYCoeff, data.accelYApprox, data.accelYDetail, data.accelYBookkeeping, data.accelYLengths);
    decomposeAxis(data.accelZData, wavelet, levels, data.accelZCoeff, data.accelZApprox, data.accelZDetail, data.accelZBookkeeping, data.accelZLengths);
    
    modifyWaveletDomain(data.accelXApprox, data.accelXDetail, data.accelYApprox, data.accelYDetail, data.accelZApprox, data.accelZDetail);
    
    gesture = identifyGesture(data.accelXApprox, data.accelXDetail, data.accelYApprox, data.accelYDetail, data.accelZApprox, data.accelZDetail);
    
    reconstructAxis(data.accelXCoeff, data.accelXApprox, data.accelXDetail, data.accelXBookkeeping, data.accelXLengths, wavelet, data.accelXData);
    reconstructAxis(data.accelYCoeff, data.accelYApprox, data.accelYDetail, data.accelYBookkeeping, data.accelYLengths, wavelet, data.accelYData);
    reconstructAxis(data.accelZCoeff, data.accelZApprox, data.accelZDetail, data.accelZBookkeeping, data.accelZLengths, wavelet, data.accelZData);
}

// Updated `scaleAndCopy` to be more generic, accepting any input and output vectors.
void GestureManager::scaleAndCopy(const std::vector<double>& input, std::vector<double>& output)
{
    if (input.empty()) {
        DBG("Input data vector empty!");
        return;
    }
    
    auto [minT, maxT] = std::minmax_element(input.begin(), input.end());
    double minVal = *minT;
    double maxVal = *maxT;
    
    output = normaliseData(minVal, maxVal, input);
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

// Updated function to send both accelerometer and gyroscope data
void GestureManager::sendProcessedDataAsBundle(const std::vector<double>& accelXData,
                                               const std::vector<double>& accelYData,
                                               const std::vector<double>& accelZData,
                                               const std::vector<double>& gyroXData,
                                               const std::vector<double>& gyroYData,
                                               const std::vector<double>& gyroZData)
{
    juce::OSCBundle bundle;

    // Add accelerometer data with clearer OSC addresses
    juce::OSCMessage accelXMessage("/imu/scaled/accelX");
    for (double value : accelXData) {
        accelXMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelXMessage);

    juce::OSCMessage accelYMessage("/imu/scaled/accelY");
    for (double value : accelYData) {
        accelYMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelYMessage);

    juce::OSCMessage accelZMessage("/imu/scaled/accelZ");
    for (double value : accelZData) {
        accelZMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(accelZMessage);
    
    // Add gyroscope data with clearer OSC addresses
    juce::OSCMessage gyroXMessage("/imu/scaled/gyroX");
    for (double value : gyroXData) {
        gyroXMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(gyroXMessage);

    juce::OSCMessage gyroYMessage("/imu/scaled/gyroY");
    for (double value : gyroYData) {
        gyroYMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(gyroYMessage);

    juce::OSCMessage gyroZMessage("/imu/scaled/gyroZ");
    for (double value : gyroZData) {
        gyroZMessage.addFloat32(static_cast<float>(value));
    }
    bundle.addElement(gyroZMessage);

    oscSender.send(bundle);
}
