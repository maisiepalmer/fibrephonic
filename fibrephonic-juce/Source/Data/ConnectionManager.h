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
#include "x-IMU3/Cpp/ConnectionInfo.hpp"
#include <vector>

class GestureManager; // Forward declare GestureManager to resolve circular dependency

class ConnectionManager : public juce::Thread
{
public:
    //==============================================================================
    ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance);
    ~ConnectionManager();
    
    //==============================================================================
    void startConnection();
    void stopConnection();
    bool getIsConnected() const { return isConnected; }
    
    //==============================================================================
    inline void setGyroscopeValues(double x, double y, double z) { gyroscopeX = x; gyroscopeY = y; gyroscopeZ = z; }
    inline void setAccelerometerValues(double x, double y, double z) { accelerationX = x; accelerationY = y; accelerationZ = z; }
    
    inline void setConnectionBool(bool b) { isConnected = b; }
    
    inline double getGyroscopeX() const { return gyroscopeX; }
    inline double getGyroscopeY() const { return gyroscopeY; }
    inline double getGyroscopeZ() const { return gyroscopeZ; }
    
    inline double getAccelerationX() const { return accelerationX; }
    inline double getAccelerationY() const { return accelerationY; }
    inline double getAccelerationZ() const { return accelerationZ; }
    
    //==============================================================================
    void run() override;

private:
    //==============================================================================
    std::unique_ptr<Connection> connectionHandler;
    juce::WaitableEvent connectionEstablishedEvent;
    
    std::shared_ptr<GestureManager> gestureManager;
    
    float gyroscopeX, gyroscopeY, gyroscopeZ;
    float accelerationX, accelerationY, accelerationZ;
    
    bool isConnected = false;
    
    std::vector<ximu3::XIMU3_Device> deviceList;
    ximu3::XIMU3_Device selectedDevice;
    
    const int pollRate = 125;
};
