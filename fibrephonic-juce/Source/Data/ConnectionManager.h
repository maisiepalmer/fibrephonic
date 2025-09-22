/**
 * @file ConnectionManager.h
 * @brief Manages the connection to x-IMU3 devices and handles sensor data
 * @author Joseph B
 * @date Created: 11 Jun 2025
 */

#pragma once

#include <JuceHeader.h>
#include "../Connection.h"
#include <memory>
#include <atomic>

class GestureManager;

/**
 * @class ConnectionManager
 * @brief Thread-safe manager for x-IMU3 device connections
 *
 * Handles device discovery via network announcement, maintains connection,
 * and provides thread-safe access to sensor data (accelerometer, gyroscope, magnetometer).
 */
class ConnectionManager : public juce::Thread
{
public:
    /**
     * @brief Constructor
     * @param gestureManagerInstance Shared pointer to the gesture manager
     */
    explicit ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance);
    
    /**
     * @brief Destructor - ensures clean shutdown
     */
    ~ConnectionManager();
    
    /** @brief Start the connection thread and begin device discovery */
    void startConnection();
    
    /** @brief Stop the connection thread and disconnect from device */
    void stopConnection();
    
    /** @brief Check if currently connected to a device */
    bool getIsConnected() const { return isConnected.load(); }
    
    /** @name Sensor Data Accessors
     *  Thread-safe getters for sensor values
     *  @{
     */
    double getAccelerationX() const { return accelerationX.load(); }
    double getAccelerationY() const { return accelerationY.load(); }
    double getAccelerationZ() const { return accelerationZ.load(); }
    
    double getGyroscopeX() const { return gyroscopeX.load(); }
    double getGyroscopeY() const { return gyroscopeY.load(); }
    double getGyroscopeZ() const { return gyroscopeZ.load(); }
    
    double getMagnetometerX() const { return magnetometerX.load(); }
    double getMagnetometerY() const { return magnetometerY.load(); }
    double getMagnetometerZ() const { return magnetometerZ.load(); }
    /** @} */
    
    /** @name Sensor Data Setters
     *  Called by Connection class to update sensor values
     *  @{
     */
    void setAccelerometerValues(double x, double y, double z)
    {
        accelerationX.store(x);
        accelerationY.store(y);
        accelerationZ.store(z);
    }
    
    void setGyroscopeValues(double x, double y, double z)
    {
        gyroscopeX.store(x);
        gyroscopeY.store(y);
        gyroscopeZ.store(z);
    }
    
    void setMagnetometerValues(double x, double y, double z)
    {
        magnetometerX.store(x);
        magnetometerY.store(y);
        magnetometerZ.store(z);
    }
    /** @} */

protected:
    /** @brief Main thread loop - handles device discovery and connection */
    void run() override;

private:
    std::unique_ptr<Connection> connectionHandler;
    std::weak_ptr<GestureManager> gestureManager; ///< Weak reference to avoid circular dependency
    
    /** @name Atomic Sensor Data Storage
     *  Thread-safe storage for sensor values
     *  @{
     */
    std::atomic<double> accelerationX{0.0}, accelerationY{0.0}, accelerationZ{0.0};
    std::atomic<double> gyroscopeX{0.0}, gyroscopeY{0.0}, gyroscopeZ{0.0};
    std::atomic<double> magnetometerX{0.0}, magnetometerY{0.0}, magnetometerZ{0.0};
    /** @} */
    
    std::atomic<bool> isConnected{false}; ///< Connection status flag
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionManager)
};
