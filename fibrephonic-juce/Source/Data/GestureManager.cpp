/*
 ==============================================================================
 
 GestureManager.cpp
 Created: 24 Jun 2025 2:18:38pm
 Author:  Joseph B
 
 ==============================================================================
 */

#include "GestureManager.h"
#include "ConnectionManager.h"

//==============================================================================
GestureManager::GestureManager()
{
    // Initialize data streams to zero
    data.magX_raw = data.magY_raw = data.magZ_raw =
    data.gx_raw = data.gy_raw = data.gz_raw =
    data.accX_raw = data.accY_raw = data.accZ_raw =
    data.jerkX = data.jerkY = data.jerkZ = 0;
    
    // Initialize scaled data vectors
    data.accelXScaled.resize(DATAWINDOW);
    data.accelYScaled.resize(DATAWINDOW);
    data.accelZScaled.resize(DATAWINDOW);
    
    data.gyroXScaled.resize(DATAWINDOW);
    data.gyroYScaled.resize(DATAWINDOW);
    data.gyroZScaled.resize(DATAWINDOW);
    
    data.magXScaled.resize(DATAWINDOW);
    data.magYScaled.resize(DATAWINDOW);
    data.magZScaled.resize(DATAWINDOW);
    
    
    gesture = NO_GESTURE;
    
    // Initialize OSC connection
    ensureOSCConnection();
    
    orientationProcessor = std::make_unique<OrientationProcessor>(
        500.f,   // tap threshold
        700.f,   // stroke threshold
        600.f    // peak detector threshold
    );
    orientationProcessor->addListener(this);

}

GestureManager::~GestureManager()
{
    stopPolling();
}

void GestureManager::startPolling()
{
    DBG("Starting IMU polling at " << IMUINERTIALREFRESH << "Hz");
    pollCount = 0;
    startTimerHz(IMUINERTIALREFRESH);
}

void GestureManager::stopPolling()
{
    DBG("Stopping IMU polling");
    stopTimer();
}

void GestureManager::timerCallback()
{
    pollGestures();
}

bool GestureManager::ensureOSCConnection()
{
    if (oscConnected && oscSender.connect(oscHost, oscPort)) {
        return true; // Already connected
    }
    
    if (oscSender.connect(oscHost, oscPort)) {
        oscConnected = true;
        oscReconnectAttempts = 0;
        DBG("OSC connected successfully to " << oscHost << ":" << oscPort);
        return true;
    } else {
        oscConnected = false;
        oscReconnectAttempts++;
        
        if (oscReconnectAttempts <= MAX_RECONNECT_ATTEMPTS) {
            DBG("OSC connection failed, attempt " << oscReconnectAttempts << "/" << MAX_RECONNECT_ATTEMPTS);
        }
        return false;
    }
}

void GestureManager::pollGestures()
{
    pollCount++;
    
    // Get raw data from the Connection manager
    getConnectionManagerValues();
    
    // Fill the circular buffers
    fillDataBuffers(data.magX_raw, data.magY_raw, data.magZ_raw,
                    data.gx_raw, data.gy_raw, data.gz_raw,
                    data.accX_raw, data.accY_raw, data.accZ_raw);
    
    // Only process if we have enough data
    if (data.accelXBuffer.size() < DATAWINDOW)
        return; // Not enough data yet
    
    // Get data from circular buffers
    data.accelXData = data.accelXBuffer.getData();
    data.accelYData = data.accelYBuffer.getData();
    data.accelZData = data.accelZBuffer.getData();
    data.gyroXData = data.gyroXBuffer.getData();
    data.gyroYData = data.gyroYBuffer.getData();
    data.gyroZData = data.gyroZBuffer.getData();
    
    // Scale the raw gyroscope data
    scaleAndCopy(data.gyroXData, data.gyroXScaled);
    scaleAndCopy(data.gyroYData, data.gyroYScaled);
    scaleAndCopy(data.gyroZData, data.gyroZScaled);
    
    // Scale the raw magnetometer data
    scaleAndCopy(data.magXData, data.magXScaled);
    scaleAndCopy(data.magYData, data.magYScaled);
    scaleAndCopy(data.magZData, data.magZScaled);
    
    // Perform the wavelet transform and filtering on accelerometer data
    perform1DWaveletTransform();
    
    // Scale the *processed* accelerometer data
    scaleAndCopy(data.accelXData, data.accelXScaled);
    scaleAndCopy(data.accelYData, data.accelYScaled);
    scaleAndCopy(data.accelZData, data.accelZScaled);
    
    // Send all the scaled data as an OSC bundle
    sendProcessedDataAsBundle(data.accelXScaled, data.accelYScaled, data.accelZScaled,
                              data.gyroXScaled, data.gyroYScaled, data.gyroZScaled,
                              data.magXScaled, data.magYScaled, data.magZScaled);
}

void GestureManager::getConnectionManagerValues()
{
    // Check if the weak_ptr is still valid by locking it
    auto lockedManager = connectionManager.lock();
    if (!lockedManager)
    {
        DBG("ConnectionManager is no longer available! Stopping polling.");
        stopPolling();
        return;
    }
    
    try {
        data.magX_raw = lockedManager->getMagnetometerX();
        data.magY_raw = lockedManager->getMagnetometerY();
        data.magZ_raw = lockedManager->getMagnetometerZ();
        
        data.gx_raw = lockedManager->getGyroscopeX();
        data.gy_raw = lockedManager->getGyroscopeY();
        data.gz_raw = lockedManager->getGyroscopeZ();
        
        data.accX_raw = lockedManager->getAccelerationX();
        data.accY_raw = lockedManager->getAccelerationY();
        data.accZ_raw = lockedManager->getAccelerationZ();
    } catch (const std::exception& e) {
        DBG("Exception getting connection manager values: " << e.what());
        return;
    }
}

void GestureManager::fillDataBuffers(double magX_in, double magY_in, double magZ_in,
                                     double gyroX_in, double gyroY_in, double gyroZ_in,
                                     double accelX_in, double accelY_in, double accelZ_in)
{
    const double scaleVal = 1.0;
    
    // Push new data to circular buffers
    data.accelXBuffer.push(accelX_in * scaleVal);
    data.accelYBuffer.push(accelY_in * scaleVal);
    data.accelZBuffer.push(accelZ_in * scaleVal);
    
    data.gyroXBuffer.push(gyroX_in);
    data.gyroYBuffer.push(gyroY_in);
    data.gyroZBuffer.push(gyroZ_in);
    
    data.magXBuffer.push(magX_in);
    data.magYBuffer.push(magY_in);
    data.magZBuffer.push(magZ_in);
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
    
    try {
        dwt(input, levels, wavelet, coeffs, bookkeeping, lengths);
        
        if (lengths.size() >= 2) {
            approx.assign(coeffs.begin(), coeffs.begin() + lengths[0]);
            detail.assign(coeffs.begin() + lengths[0], coeffs.begin() + lengths[0] + lengths[1]);
        }
    } catch (const std::exception& e) {
        DBG("Exception in decomposeAxis: " << e.what());
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
    try {
        size_t totalLength = 0;
        for (auto len : lengths) totalLength += static_cast<size_t>(len);
        coeffs.resize(totalLength);
        
        std::copy(approx.begin(), approx.end(), coeffs.begin());
        std::copy(detail.begin(), detail.end(), coeffs.begin() + lengths[0]);
        
        std::vector<int> idwt_lengths(lengths.begin(), lengths.end());
        
        idwt(coeffs, bookkeeping, wavelet, reconstructed, idwt_lengths);
    } catch (const std::exception& e) {
        DBG("Exception in reconstructAxis: " << e.what());
    }
}

void GestureManager::softThresholding(std::vector<double>& detailCoeffs)
{
    if (detailCoeffs.empty()) return;
    
    try {
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
    } catch (const std::exception& e) {
        DBG("Exception in softThresholding: " << e.what());
    }
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
    if (accelXApprox.empty() || accelYApprox.empty() || accelZApprox.empty())
        return NO_GESTURE;


    return NO_GESTURE;
}

void GestureManager::perform1DWaveletTransform()
{
    std::string wavelet = "db4";
    int levels = 1;
    
    try {
        decomposeAxis(data.accelXData, wavelet, levels, data.accelXCoeff, data.accelXApprox, data.accelXDetail, data.accelXBookkeeping, data.accelXLengths);
        decomposeAxis(data.accelYData, wavelet, levels, data.accelYCoeff, data.accelYApprox, data.accelYDetail, data.accelYBookkeeping, data.accelYLengths);
        decomposeAxis(data.accelZData, wavelet, levels, data.accelZCoeff, data.accelZApprox, data.accelZDetail, data.accelZBookkeeping, data.accelZLengths);
        
        modifyWaveletDomain(data.accelXApprox, data.accelXDetail, data.accelYApprox, data.accelYDetail, data.accelZApprox, data.accelZDetail);
        
        gesture = identifyGesture(data.accelXApprox, data.accelXDetail, data.accelYApprox, data.accelYDetail, data.accelZApprox, data.accelZDetail);
        
        reconstructAxis(data.accelXCoeff, data.accelXApprox, data.accelXDetail, data.accelXBookkeeping, data.accelXLengths, wavelet, data.accelXData);
        reconstructAxis(data.accelYCoeff, data.accelYApprox, data.accelYDetail, data.accelYBookkeeping, data.accelYLengths, wavelet, data.accelYData);
        reconstructAxis(data.accelZCoeff, data.accelZApprox, data.accelZDetail, data.accelZBookkeeping, data.accelZLengths, wavelet, data.accelZData);
    } catch (const std::exception& e) {
        DBG("Exception in perform1DWaveletTransform: " << e.what());
    }
}

void GestureManager::scaleAndCopy(const std::vector<double>& input, std::vector<double>& output)
{
    if (input.empty()) {
        DBG("Input data vector empty!");
        return;
    }
    
    try {
        auto [minT, maxT] = std::minmax_element(input.begin(), input.end());
        double minVal = *minT;
        double maxVal = *maxT;
        
        output = normaliseData(minVal, maxVal, input);
    } catch (const std::exception& e) {
        DBG("Exception in scaleAndCopy: " << e.what());
    }
}

std::vector<double> GestureManager::normaliseData(double min, double max, const std::vector<double>& input)
{
    std::vector<double> rescaled;
    rescaled.reserve(input.size());
    
    if (std::abs(min - max) < 1e-10) { // Avoid division by zero
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

void GestureManager::sendProcessedDataAsBundle(const std::vector<double>& accelXData,
                                               const std::vector<double>& accelYData,
                                               const std::vector<double>& accelZData,
                                               const std::vector<double>& gyroXData,
                                               const std::vector<double>& gyroYData,
                                               const std::vector<double>& gyroZData,
                                               const std::vector<double>& magXData,
                                               const std::vector<double>& magYData,
                                               const std::vector<double>& magZData)
{
    // Ensure OSC connection is active
    if (!ensureOSCConnection()) {
        if (oscReconnectAttempts > MAX_RECONNECT_ATTEMPTS) {
            if (pollCount % 1000 == 0) { // Only log every 10 seconds to avoid spam
                DBG("OSC connection failed after " << MAX_RECONNECT_ATTEMPTS << " attempts. Data not sent.");
            }
        }
        return;
    }
    
    try {
        juce::OSCBundle bundle;

        juce::OSCMessage accelXMessage("/imu/accelX");
        for (double value : accelXData) {
            accelXMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(accelXMessage);

        juce::OSCMessage accelYMessage("/imu/accelY");
        for (double value : accelYData) {
            accelYMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(accelYMessage);

        juce::OSCMessage accelZMessage("/imu/accelZ");
        for (double value : accelZData) {
            accelZMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(accelZMessage);
        
        juce::OSCMessage gyroXMessage("/imu/gyroX");
        for (double value : gyroXData) {
            gyroXMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(gyroXMessage);

        juce::OSCMessage gyroYMessage("/imu/gyroY");
        for (double value : gyroYData) {
            gyroYMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(gyroYMessage);

        juce::OSCMessage gyroZMessage("/imu/gyroZ");
        for (double value : gyroZData) {
            gyroZMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(gyroZMessage);
        
        juce::OSCMessage magXMessage("/imu/magX");
        for (double value : magXData) {
            magXMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(magXMessage);

        juce::OSCMessage magYMessage("/imu/magY");
        for (double value : magYData) {
            magYMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(magYMessage);
        
        juce::OSCMessage magZMessage("/imu/magZ");
        for (double value : magZData) {
            magZMessage.addFloat32(static_cast<float>(value));
        }
        bundle.addElement(magZMessage);
        
        juce::OSCMessage gestureMessage("/imu/gesture");
        gestureMessage.addInt32((int)gesture);
        bundle.addElement(gestureMessage);

        if (!oscSender.send(bundle)) {
            DBG("Failed to send OSC bundle!");
            oscConnected = false; // Mark as disconnected so we retry connection next time
        } else {
            // Success - reset reconnect attempts
            oscReconnectAttempts = 0;
        }
    } catch (const std::exception& e) {
        DBG("Exception sending OSC data: " << e.what());
        oscConnected = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------
void GestureManager::newDirection (const OrientationProcessor* source, const Direction direction)
{
    
}
void GestureManager::newSegment (const OrientationProcessor* source, const Segment segment)
{
    
}
void GestureManager::orientationEvent (const OrientationProcessor* source, OrientationProcessor::Axis axis, float magnitude)
{
    
}
void GestureManager::gyroscopeDisplacement(const OrientationProcessor* source, const float gyroDelta[OrientationProcessor::NumAxes])
{
    
}
