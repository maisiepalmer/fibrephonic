/*
 ==============================================================================
 
 ConnectionManager.h
 Created: 11 Jun 2025 10:32:59am
 Author:  Joseph B
 
 Establishes bluetooth connection using IMU connection example and separates
 and sends sensor data values.
 
 Use for immediate input scaling and poll rate adjustment, also manage port
 connections.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "../Connection.h"
#include <memory>

class GestureManager;

class ConnectionManager : public juce::Thread
{
public:
    explicit ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance);
    ~ConnectionManager();
    
    // Connection control
    void startConnection();
    void stopConnection();
    bool getIsConnected() const { return isConnected; }
    
    // Sensor data accessors - thread-safe
    double getAccelerationX() const { return accelerationX.load(); }
    double getAccelerationY() const { return accelerationY.load(); }
    double getAccelerationZ() const { return accelerationZ.load(); }
    
    double getGyroscopeX() const { return gyroscopeX.load(); }
    double getGyroscopeY() const { return gyroscopeY.load(); }
    double getGyroscopeZ() const { return gyroscopeZ.load(); }
    
    double getMagnetometerX() const { return magnetometerX.load(); }
    double getMagnetometerY() const { return magnetometerY.load(); }
    double getMagnetometerZ() const { return magnetometerZ.load(); }
    
    // Called by Connection class to update sensor values
    void setAccelerometerValues(double x, double y, double z);
    void setGyroscopeValues(double x, double y, double z);
    void setMagnetometerValues(double x, double y, double z);

protected:
    void run() override;

private:
    std::unique_ptr<Connection> connectionHandler;
    std::shared_ptr<GestureManager> gestureManager;
    
    // Thread-safe atomic sensor data storage
    std::atomic<double> accelerationX{0.0}, accelerationY{0.0}, accelerationZ{0.0};
    std::atomic<double> gyroscopeX{0.0}, gyroscopeY{0.0}, gyroscopeZ{0.0};
    std::atomic<double> magnetometerX{0.0}, magnetometerY{0.0}, magnetometerZ{0.0};
    
    std::atomic<bool> isConnected{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionManager)
};
