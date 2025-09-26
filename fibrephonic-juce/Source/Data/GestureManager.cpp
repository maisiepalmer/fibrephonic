/**
 * @file GestureManager.cpp
 * @brief Gesture manager using calibrated detection system with Mi.mu-based tap detection
 * @author Joseph B
 * @date Created: 24 Jun 2025
 */

#include "GestureManager.h"
#include "ConnectionManager.h"

GestureManager::GestureManager()
{
    gestureDetector = std::make_unique<GestureDetector>();
    ensureOSCConnection();
}

GestureManager::~GestureManager()
{
    stopPolling();
}

void GestureManager::startPolling()
{
    pollCount = 0;
    isPolling = true;
    startTimerHz(POLLING_RATE_HZ);
}

void GestureManager::stopPolling()
{
    isPolling = false;
    stopTimer();
}

void GestureManager::startCalibration()
{
    if (gestureDetector)
    {
        gestureDetector->startCalibration();
        
        // Send calibration start message to Max
        if (ensureOSCConnection())
        {
            juce::OSCMessage msg("/calibration/start");
            oscSender.send(msg);
        }
        
        DBG("Started textile gesture calibration...");
    }
}

void GestureManager::stopCalibration()
{
    if (gestureDetector)
    {
        gestureDetector->stopCalibration();
        
        // Send calibration data to Max
        if (gestureDetector->isCalibrated() && ensureOSCConnection())
        {
            auto calib = gestureDetector->getCalibration();
            
            juce::OSCMessage msg("/calibration/complete");
            msg.addFloat32(calib.baselineMagnitude);
            msg.addFloat32(calib.baselineStd);
            msg.addFloat32(calib.baselineX);
            msg.addFloat32(calib.baselineY);
            msg.addFloat32(calib.baselineZ);
            msg.addFloat32(calib.stdX);
            msg.addFloat32(calib.stdY);
            msg.addFloat32(calib.stdZ);
            
            oscSender.send(msg);
        }
        
        DBG("Stopped textile gesture calibration");
    }
}

bool GestureManager::isCalibrated() const
{
    return gestureDetector ? gestureDetector->isCalibrated() : false;
}

void GestureManager::timerCallback()
{
    if (isPolling)
    {
        pollGestures();
    }
}

void GestureManager::pollGestures()
{
    pollCount++;
    
    // Get latest sensor data from connection manager
    if (!getSensorDataFromConnection())
        return; // No valid data available
    
    IMUData imuData(
        sensorData.accelX, sensorData.accelY, sensorData.accelZ,
        sensorData.gyroX, sensorData.gyroY, sensorData.gyroZ,
        sensorData.magX, sensorData.magY, sensorData.magZ
    );
    
    // Update detector and check for tap (Mi.mu drum-detector based)
    gestureDetector->pushSample(imuData);
    lastTapVelocity = gestureDetector->detectTap(); // Returns velocity or 0
    
    // Send ALL data via OSC at refresh rate
    sendDataViaOSC();
}

bool GestureManager::getSensorDataFromConnection()
{
    // Check if connection manager is still valid
    auto lockedManager = connectionManager.lock();
    if (!lockedManager)
    {
        if (isPolling)
        {
            stopPolling();
        }
        return false;
    }
    
    // Check if we have a valid connection
    if (!lockedManager->getIsConnected())
    {
        return false;
    }
    
    try
    {
        // Get latest sensor readings - these are atomic operations so thread-safe
        sensorData.accelX = static_cast<float>(lockedManager->getAccelerationX());
        sensorData.accelY = static_cast<float>(lockedManager->getAccelerationY());
        sensorData.accelZ = static_cast<float>(lockedManager->getAccelerationZ());
        
        sensorData.gyroX = static_cast<float>(lockedManager->getGyroscopeX());
        sensorData.gyroY = static_cast<float>(lockedManager->getGyroscopeY());
        sensorData.gyroZ = static_cast<float>(lockedManager->getGyroscopeZ());
        
        sensorData.magX = static_cast<float>(lockedManager->getMagnetometerX());
        sensorData.magY = static_cast<float>(lockedManager->getMagnetometerY());
        sensorData.magZ = static_cast<float>(lockedManager->getMagnetometerZ());
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool GestureManager::ensureOSCConnection()
{
    if (oscConnected)
    {
        return true;
    }
    
    if (oscSender.connect(oscHost, oscPort))
    {
        oscConnected = true;
        oscReconnectAttempts = 0;
        return true;
    }
    else
    {
        oscConnected = false;
        oscReconnectAttempts++;
        return false;
    }
}

void GestureManager::sendDataViaOSC()
{
    if (!ensureOSCConnection())
    {
        return;
    }
    
    try
    {
        // Raw sensor data (existing streams for compatibility)
        juce::OSCMessage accMessage("/sensor/acc");
        accMessage.addFloat32(sensorData.accelX);
        accMessage.addFloat32(sensorData.accelY);
        accMessage.addFloat32(sensorData.accelZ);
        
        juce::OSCMessage gyroMessage("/sensor/gyro");
        gyroMessage.addFloat32(sensorData.gyroX);
        gyroMessage.addFloat32(sensorData.gyroY);
        gyroMessage.addFloat32(sensorData.gyroZ);
        
        juce::OSCMessage magMessage("/sensor/mag");
        magMessage.addFloat32(sensorData.magX);
        magMessage.addFloat32(sensorData.magY);
        magMessage.addFloat32(sensorData.magZ);
        
        // Enhanced data for Max/MSP analysis (only if calibrated)
        if (gestureDetector->isCalibrated())
        {
            juce::OSCMessage calibratedMessage("/sensor/calibrated");
            calibratedMessage.addFloat32(gestureDetector->getCalibratedMagnitude());
            calibratedMessage.addFloat32(gestureDetector->getCalibratedX());
            calibratedMessage.addFloat32(gestureDetector->getCalibratedY());
            calibratedMessage.addFloat32(gestureDetector->getCalibratedZ());
            
            juce::OSCMessage magnitudeMessage("/sensor/magnitude");
            magnitudeMessage.addFloat32(gestureDetector->getMagnitude());
            
            // Continuous directional information (adapted from Mi.mu DirectionProcessor)
            auto directionalInfo = gestureDetector->getDirectionalInfo();
            juce::OSCMessage directionMessage("/sensor/direction");
            directionMessage.addFloat32(directionalInfo.tiltX);      // -1 to 1, normalised tilt
            directionMessage.addFloat32(directionalInfo.tiltY);      // -1 to 1, normalised tilt
            directionMessage.addFloat32(directionalInfo.tiltZ);      // -1 to 1, normalised tilt
            directionMessage.addFloat32(directionalInfo.magnitude);  // Overall movement magnitude
            directionMessage.addInt32(directionalInfo.isMoving ? 1 : 0); // Movement flag
            
            if (!oscSender.send(calibratedMessage) || !oscSender.send(magnitudeMessage) ||
                !oscSender.send(directionMessage))
                oscConnected = false;
        }
        
        // Tap detection with velocity (Mi.mu drum-detector style)
        if (lastTapVelocity > 0.0f)
        {
            juce::OSCMessage tapMessage("/gesture/tap");
            tapMessage.addFloat32(lastTapVelocity);
            tapMessage.addInt32(1); // Binary flag for Max trigger
            
            if (!oscSender.send(tapMessage))
                oscConnected = false;
        }
        
        // Send all messages
        if (!oscSender.send(accMessage) || !oscSender.send(gyroMessage) ||
            !oscSender.send(magMessage))
        {
            oscConnected = false;
        }
        else
        {
            oscReconnectAttempts = 0;
        }
    }
    catch (...)
    {
        oscConnected = false;
    }
}
