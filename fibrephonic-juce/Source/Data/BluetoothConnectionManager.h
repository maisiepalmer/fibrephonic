/*
 ==============================================================================
 
 BluetoothConnectionManager.h
 Created: 11 Jun 2025 10:32:59am
 Author:  Joseph B
 
 Establlishes bluetooth connection using IMU connection example and separates
 and sends sensor data values.
 
 Use for immediate input scaling and poll rate adjustment, also manage port
 connections.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "../Connection.h"
#include "x-IMU3/Cpp/ConnectionInfo.hpp"
#include "../Identifiers.h"

class BluetoothConnectionManager : public Connection, public Thread, public ValueTree::Listener
{
public:
    //==============================================================================
    BluetoothConnectionManager(std::shared_ptr<ValueTree> calibrationTree)
    : Thread("Bluetooth Connection Thread")
    , vt(calibrationTree)
    {
        gX = gY = gZ = 0;
        accX = accY = accZ = 0;
        
        connectionInstance = std::make_unique<Connection>(this);
        
        vt->addListener(this);
    }
    
    ~BluetoothConnectionManager()
    {
        signalThreadShouldExit();
        stopThread(500);
        vt->removeListener(this);
    }
    
    //==============================================================================
    juce::StringArray pollConnections()
    {
        deviceList = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);
        juce::StringArray names;
        names.clear();
        if (!deviceList.empty())
        {
            for (int i = 0; i < deviceList.size(); i++)
            {
                juce::String deviceName (deviceList[i].device_name);
                names.add(deviceName);
            }
        }
        return names;
    }
    
    void selectDevice(int index)
    {
        selectedDevice = deviceList[index];
    }
    
    void run() override
    {
        while (!threadShouldExit())
        {
            auto connectionInfoPtr = ximu3::connectionInfoFrom(selectedDevice);
            if (!connectionInfoPtr)
            {
                DBG("Failed to create connection info.");
                return;
            }
            
            connectionInstance->runConnection(*connectionInfoPtr, [this]() { return threadShouldExit(); });
            
            gX = connectionInstance->getX();
            gY = connectionInstance->getY();
            gZ = connectionInstance->getZ();
            
            accX = connectionInstance->getaccX();
            accY = connectionInstance->getaccY();
            accZ = connectionInstance->getaccZ();
        }
    }
    
    //==============================================================================
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override
    {
        if(property == Identifiers::Calibration::BluetoothPoll && treeWhosePropertyHasChanged.getProperty(property))
        {
            juce::Atomic<juce::StringArray> names;
            names.set(pollConnections());
            names.get().isEmpty() ? vt->setProperty(Identifiers::Calibration::BluetoothOptions, "", nullptr) : vt->setProperty(Identifiers::Calibration::BluetoothOptions, names.get(), nullptr);
            vt->setProperty(Identifiers::Calibration::BluetoothPoll, false, nullptr);
        }
        else if(property == Identifiers::Calibration::BluetoothSelection)
        {
            int index = treeWhosePropertyHasChanged.getProperty(property);
            selectDevice(index);
            startThread();
        }
    }
    
    //==============================================================================
    inline void setGyroscopeValues(double x, double y, double z) { gX = x; gY = y; gZ = z; }
    inline void setAccelerometerValues(double x, double y, double z) { accX = x; accY = y; accZ = z; }
    
    inline double getGyroscopeX() { return gX; }
    inline double getGyroscopeY() { return gY; }
    inline double getGyroscopeZ() { return gZ; }
    
    inline double getAccelerationX() { return accX; }
    inline double getAccelerationY() { return accY; }
    inline double getAccelerationZ() { return accZ; }
    
private:
    //==============================================================================
    std::unique_ptr<Connection> connectionInstance;
    std::shared_ptr<ValueTree> vt;
    
    float gX, gY, gZ;
    float accX, accY, accZ;
    
    bool isConnected = false;
    
    std::vector<ximu3::XIMU3_Device> deviceList;
    ximu3::XIMU3_Device selectedDevice;
};
