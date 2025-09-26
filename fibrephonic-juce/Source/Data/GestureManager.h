/**
 * @file GestureManager.h
 * @brief Manages textile gesture detection with calibration support
 * Uses drum-detector style approach for material interactions
 */

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include "GestureDetector.h"
#include "../Helpers.h"

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
    
    // Core functions
    void startPolling();
    void stopPolling();
    
    // Calibration
    void startCalibration();
    void stopCalibration();
    bool isCalibrated() const;
    GestureDetector* getDetector() { return gestureDetector.get(); }
    
    // For UI feedback
    float getLastTapVelocity() const { return lastTapVelocity; }

private:
    static constexpr int POLLING_RATE_HZ = 100;
    
    std::unique_ptr<GestureDetector> gestureDetector;
    std::weak_ptr<ConnectionManager> connectionManager;
    
    // OSC Communication
    juce::OSCSender oscSender;
    juce::String oscHost = "192.169.1.2";
    int oscPort = 5006;
    bool oscConnected = false;
    int oscReconnectAttempts = 0;
    
    // State
    std::atomic<int> pollCount{0};
    std::atomic<bool> isPolling{false};
    float lastTapVelocity = 0.0f;
    
    // Raw sensor data
    struct SensorData
    {
        float accelX = 0, accelY = 0, accelZ = 0;
        float gyroX = 0, gyroY = 0, gyroZ = 0;
        float magX = 0, magY = 0, magZ = 0;
    } sensorData;
    
    void timerCallback() override;
    void pollGestures();
    bool getSensorDataFromConnection();
    bool ensureOSCConnection();
    void sendDataViaOSC();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureManager)
};
