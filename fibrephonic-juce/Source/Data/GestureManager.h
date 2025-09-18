/*
 ==============================================================================
 
 GestureManager.h
 Created: 13 Jun 2025 3:33:06pm
 Author:  Joseph B
 
 Gesture Manager class to identify, scale, handle and export IMU sensor data
 over various connection types.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <deque>
#include "SimpleGestureDetector.h"

// Forward declare to avoid circular dependency
class ConnectionManager;

class GestureManager : private juce::Timer
{
public:
    GestureManager();
    ~GestureManager();
    
    void setConnectionManager(std::shared_ptr<ConnectionManager> connectionManagerInstance)
    {
        connectionManager = connectionManagerInstance;
    }
    
    void startPolling();
    void stopPolling();
    
    SimpleGestureDetector::GestureType getLastGesture() const { return lastDetectedGesture; }
    std::string getLastGestureName() const { return gestureDetector.getGestureName(lastDetectedGesture); }

private:
    static constexpr int POLLING_RATE_HZ = 100;  // 100 Hz polling
    static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
    
    // Core components
    SimpleGestureDetector gestureDetector;
    std::weak_ptr<ConnectionManager> connectionManager;
    
    // OSC communication
    juce::OSCSender oscSender;
    juce::String oscHost = "127.0.0.1";  // Fixed IP address
    int oscPort = 5006;
    bool oscConnected = false;
    int oscReconnectAttempts = 0;
    
    // State tracking
    SimpleGestureDetector::GestureType lastDetectedGesture = SimpleGestureDetector::NO_GESTURE;
    int pollCount = 0;
    
    // Raw sensor data (latest readings)
    struct SensorData
    {
        float accelX = 0, accelY = 0, accelZ = 0;
        float gyroX = 0, gyroY = 0, gyroZ = 0;
        float magX = 0, magY = 0, magZ = 0;
    } sensorData;
    
    // Timer callback - called at POLLING_RATE_HZ
    void timerCallback() override;
    
    // Core processing functions
    void pollGestures();
    bool getSensorDataFromConnection();
    bool ensureOSCConnection();
    void sendGestureDataViaOSC();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureManager)
};
