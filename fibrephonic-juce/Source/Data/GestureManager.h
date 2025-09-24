/**
 * @file GestureManager.h
 * @brief Manages gesture detection and OSC communication
 * @author Joseph B
 * @date Created: 13 Jun 2025
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <deque>
#include <atomic>
#include "Training/FastGestureClassifier.h"

class ConnectionManager;

/**
 * @class GestureManager
 * @brief Processes IMU data for gesture detection and sends data via OSC
 *
 * Polls sensor data from ConnectionManager at 100Hz, performs gesture
 * detection, and continuously streams both raw sensor data and detected
 * gestures via OSC.
 */
class GestureManager : private juce::Timer
{
public:
    GestureManager();
    ~GestureManager();
    
    /**
     * @brief Set the connection manager reference
     * @param connectionManagerInstance Shared pointer to connection manager
     */
    void setConnectionManager(std::shared_ptr<ConnectionManager> connectionManagerInstance)
    {
        connectionManager = connectionManagerInstance;
    }
    
    /** @brief Start polling sensor data and detecting gestures */
    void startPolling();
    
    /** @brief Stop polling and gesture detection */
    void stopPolling();
    
    /** @brief Get the last detected gesture type */
    Gestures::GestureType getLastGesture() const { return lastDetectedGesture; }

private:
    static constexpr int POLLING_RATE_HZ = 100;  ///< Sensor polling rate
    static constexpr int MAX_RECONNECT_ATTEMPTS = 5; ///< Max OSC reconnection attempts
    
    std::unique_ptr<FastGestureClassifier> mlClassifier;
    std::weak_ptr<ConnectionManager> connectionManager; ///< Weak ref to connection manager
    
    /** @name OSC Communication
     *  @{
     */
    juce::OSCSender oscSender;
    juce::String oscHost = "192.169.1.2";
    int oscPort = 5006;
    bool oscConnected = false;
    int oscReconnectAttempts = 0;
    /** @} */
    
    /** @name State Tracking
     *  @{
     */
    Gestures::GestureType lastDetectedGesture = Gestures::NO_GESTURE;
    std::atomic<int> pollCount{0};
    std::atomic<bool> isPolling{false};
    /** @} */
    
    /** @brief Raw sensor data structure */
    struct SensorData
    {
        float accelX = 0, accelY = 0, accelZ = 0;
        float gyroX = 0, gyroY = 0, gyroZ = 0;
        float magX = 0, magY = 0, magZ = 0;
    } sensorData;
    
    /** @brief Timer callback - called at POLLING_RATE_HZ */
    void timerCallback() override;
    
    /** @brief Main gesture processing loop */
    void pollGestures();
    
    /** @brief Get sensor data from connection manager */
    bool getSensorDataFromConnection();
    
    /** @brief Ensure OSC connection is active */
    bool ensureOSCConnection();
    
    /** @brief Send all data via OSC (continuous streaming) */
    void sendDataViaOSC();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GestureManager)
};
