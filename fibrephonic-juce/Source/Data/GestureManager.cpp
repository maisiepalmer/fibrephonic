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
{
    DBG("GestureManager initialized");
    
    // Initialize OSC connection
    ensureOSCConnection();
}

GestureManager::~GestureManager()
{
    stopPolling();
}

void GestureManager::startPolling()
{
    DBG("Starting gesture polling at " << POLLING_RATE_HZ << "Hz");
    pollCount = 0;
    startTimerHz(POLLING_RATE_HZ);
}

void GestureManager::stopPolling()
{
    DBG("Stopping gesture polling");
    stopTimer();
}

void GestureManager::timerCallback()
{
    pollGestures();
}

void GestureManager::pollGestures()
{
    pollCount++;
    
    // Get latest sensor data from connection manager
    if (!getSensorDataFromConnection())
        return; // No valid data available
    
    // Create IMU data structure for gesture detector
    SimpleGestureDetector::IMUData imuData(
        sensorData.accelX, sensorData.accelY, sensorData.accelZ,
        sensorData.gyroX, sensorData.gyroY, sensorData.gyroZ,
        sensorData.magX, sensorData.magY, sensorData.magZ
    );
    
    // Process gesture detection
    auto detectedGesture = gestureDetector.processIMUData(imuData);
    
    // Check if a new gesture was detected
    if (detectedGesture != SimpleGestureDetector::NO_GESTURE && detectedGesture != lastDetectedGesture)
    {
        lastDetectedGesture = detectedGesture;
        DBG("New gesture detected: " << gestureDetector.getGestureName(detectedGesture));
        
        // Send gesture data via OSC
        sendGestureDataViaOSC();
    }
    else if (detectedGesture == SimpleGestureDetector::NO_GESTURE)
    {
        lastDetectedGesture = SimpleGestureDetector::NO_GESTURE;
    }
    
    // Debug output every second
    if (pollCount % POLLING_RATE_HZ == 0)
    {
        DBG("Sensor data - Accel: " << sensorData.accelX << ", " << sensorData.accelY << ", " << sensorData.accelZ);
        DBG("OSC Connected: " << oscConnected << " | Last Gesture: " << getLastGestureName());
    }
}

bool GestureManager::getSensorDataFromConnection()
{
    // Check if connection manager is still valid
    auto lockedManager = connectionManager.lock();
    if (!lockedManager)
    {
        DBG("ConnectionManager no longer available!");
        stopPolling();
        return false;
    }
    
    // Check if we have a valid connection
    if (!lockedManager->getIsConnected())
    {
        // Only log occasionally to avoid spam
        if (pollCount % (POLLING_RATE_HZ * 5) == 0) // Every 5 seconds
        {
            DBG("IMU device not connected");
        }
        return false;
    }
    
    try
    {
        // Get latest sensor readings
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
    catch (const std::exception& e)
    {
        DBG("Exception getting sensor data: " << e.what());
        return false;
    }
}

bool GestureManager::ensureOSCConnection()
{
    if (oscConnected && oscSender.connect(oscHost, oscPort))
    {
        return true; // Already connected and working
    }
    
    if (oscSender.connect(oscHost, oscPort))
    {
        oscConnected = true;
        oscReconnectAttempts = 0;
        DBG("OSC connected successfully to " << oscHost << ":" << oscPort);
        return true;
    }
    else
    {
        oscConnected = false;
        oscReconnectAttempts++;
        
        if (oscReconnectAttempts <= MAX_RECONNECT_ATTEMPTS)
        {
            DBG("OSC connection failed, attempt " << oscReconnectAttempts << "/" << MAX_RECONNECT_ATTEMPTS);
        }
        return false;
    }
}

void GestureManager::sendGestureDataViaOSC()
{
    if (!ensureOSCConnection())
    {
        if (oscReconnectAttempts > MAX_RECONNECT_ATTEMPTS)
        {
            if (pollCount % (POLLING_RATE_HZ * 10) == 0) // Log every 10 seconds
            {
                DBG("OSC connection failed after " << MAX_RECONNECT_ATTEMPTS << " attempts");
            }
        }
        return;
    }
    
    try
    {
        // Send gesture information
        juce::OSCMessage gestureMessage("/gesture/detected");
        gestureMessage.addInt32(static_cast<int>(lastDetectedGesture));
        gestureMessage.addString(getLastGestureName());
        
        // Send raw sensor data for external processing if needed
        juce::OSCMessage sensorMessage("/sensor/raw");
        sensorMessage.addFloat32(sensorData.accelX);
        sensorMessage.addFloat32(sensorData.accelY);
        sensorMessage.addFloat32(sensorData.accelZ);
        sensorMessage.addFloat32(sensorData.gyroX);
        sensorMessage.addFloat32(sensorData.gyroY);
        sensorMessage.addFloat32(sensorData.gyroZ);
        sensorMessage.addFloat32(sensorData.magX);
        sensorMessage.addFloat32(sensorData.magY);
        sensorMessage.addFloat32(sensorData.magZ);
        
        // Send both messages
        if (!oscSender.send(gestureMessage) || !oscSender.send(sensorMessage))
        {
            DBG("Failed to send OSC messages!");
            oscConnected = false;
        }
        else
        {
            oscReconnectAttempts = 0; // Reset on success
        }
    }
    catch (const std::exception& e)
    {
        DBG("Exception sending OSC data: " << e.what());
        oscConnected = false;
    }
}
