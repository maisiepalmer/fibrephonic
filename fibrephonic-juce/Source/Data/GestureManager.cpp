/*
  ==============================================================================
    GestureManager.cpp
    Created: 24 Jun 2025 2:18:38pm
    Author:  Joseph B
  ==============================================================================
*/

#include "GestureManager.h"
#include "ConnectionManager.h"

GestureManager::GestureManager()
: gestureDetector(gestureThresholds)
{
    gestureThresholds.tapSoftThreshold = 1.5f;
    gestureThresholds.tapHardThreshold = 2.5f;
    gestureThresholds.strokeMinAccel = 0.5f;
    gestureDetector.setThresholds(gestureThresholds);
    
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
    
    // Create IMU data structure for gesture detector
    IMUData imuData(
        sensorData.accelX, sensorData.accelY, sensorData.accelZ,
        sensorData.gyroX, sensorData.gyroY, sensorData.gyroZ,
        sensorData.magX, sensorData.magY, sensorData.magZ
    );
    
    // Process gesture detection
    auto detectedGesture = gestureDetector.processIMUData(imuData);
    
    // Update last detected gesture
    if (detectedGesture != Gestures::NO_GESTURE)
    {
        lastDetectedGesture = detectedGesture;
    }
    
    // Send ALL data via OSC at refresh rate (not just when gesture detected)
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
        // Always send current gesture state
        juce::OSCMessage gestureMessage("/gesture");
        gestureMessage.addInt32(static_cast<int>(lastDetectedGesture));
        gestureMessage.addString(Gestures::getGestureName(getLastGesture()));
        
        // Always send sensor data
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
