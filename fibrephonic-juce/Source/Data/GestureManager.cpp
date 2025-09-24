/**
 * @file GestureManager.cpp
 * @brief Gesture manager using calibrated detection system
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
        DBG("Started gesture calibration...");
    }
}

void GestureManager::stopCalibration()
{
    if (gestureDetector)
    {
        gestureDetector->stopCalibration();
        DBG("Stopped gesture calibration");
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
    
    // Create IMU data structure
    IMUData imuData(
        sensorData.accelX, sensorData.accelY, sensorData.accelZ,
        sensorData.gyroX, sensorData.gyroY, sensorData.gyroZ,
        sensorData.magX, sensorData.magY, sensorData.magZ
    );
    
    // Add data to detector
    gestureDetector->addSensorData(imuData);
    
    // Detect gesture
    auto result = gestureDetector->detect();
    
    if (result.valid)
    {
        DBG("Detected: " << result.toString() << " (confidence: " << result.confidence << ")");
        
        // Convert to legacy gesture type for compatibility
        if (result.type == GestureDetector::GestureType::TAP)
        {
            switch(result.tapStrength)
            {
                case GestureDetector::TapStrength::SOFT:
                    lastDetectedGesture = Gestures::TAP_SOFT;
                    break;
                case GestureDetector::TapStrength::MEDIUM:
                    lastDetectedGesture = Gestures::TAP_SOFT; // Map medium to soft for now
                    break;
                case GestureDetector::TapStrength::HARD:
                    lastDetectedGesture = Gestures::TAP_HARD;
                    break;
            }
        }
        else if (result.type == GestureDetector::GestureType::STROKE)
        {
            switch(result.strokeDirection)
            {
                case GestureDetector::StrokeDirection::UP:
                    lastDetectedGesture = Gestures::STROKE_UP;
                    break;
                case GestureDetector::StrokeDirection::DOWN:
                    lastDetectedGesture = Gestures::STROKE_DOWN;
                    break;
                case GestureDetector::StrokeDirection::LEFT:
                    lastDetectedGesture = Gestures::STROKE_LEFT;
                    break;
                case GestureDetector::StrokeDirection::RIGHT:
                    lastDetectedGesture = Gestures::STROKE_RIGHT;
                    break;
            }
        }
        
        // Store confidence for OSC
        lastConfidence = result.confidence;
    }
    else
    {
        // No gesture detected
        lastDetectedGesture = Gestures::NO_GESTURE;
        lastConfidence = 0.0f;
    }
    
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
        // Send gesture with confidence
        juce::OSCMessage gestureMessage("/gesture");
        gestureMessage.addInt32(static_cast<int>(lastDetectedGesture));
        gestureMessage.addString(Gestures::getGestureName(lastDetectedGesture));
        gestureMessage.addFloat32(lastConfidence);
        
        // Send sensor data
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
        
        // Send all messages
        if (!oscSender.send(gestureMessage) || !oscSender.send(accMessage) ||
            !oscSender.send(gyroMessage) || !oscSender.send(magMessage))
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
