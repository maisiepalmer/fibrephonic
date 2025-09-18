/*
 ==============================================================================
 
 ConnectionManager.cpp
 Created: 20 Aug 2025 5:34:45pm
 Author:  Maisie Palmer
 
 ==============================================================================
 */

#include "ConnectionManager.h"
#include "GestureManager.h"

ConnectionManager::ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance)
    : juce::Thread("IMU Connection Thread")
    , gestureManager(gestureManagerInstance)
{
    connectionHandler = std::make_unique<Connection>(this);
    DBG("ConnectionManager created");
}

ConnectionManager::~ConnectionManager()
{
    DBG("ConnectionManager shutting down...");
    signalThreadShouldExit();
    stopThread(2000); // Wait up to 2 seconds for clean shutdown
}

void ConnectionManager::startConnection()
{
    DBG("Starting connection thread...");
    startThread();
}

void ConnectionManager::stopConnection()
{
    DBG("Stopping connection...");
    signalThreadShouldExit();
    stopThread(1000);
    isConnected = false;
}

void ConnectionManager::setAccelerometerValues(double x, double y, double z)
{
    accelerationX = x;
    accelerationY = y;
    accelerationZ = z;
}

void ConnectionManager::setGyroscopeValues(double x, double y, double z)
{
    gyroscopeX = x;
    gyroscopeY = y;
    gyroscopeZ = z;
}

void ConnectionManager::setMagnetometerValues(double x, double y, double z)
{
    magnetometerX = x;
    magnetometerY = y;
    magnetometerZ = z;
}

void ConnectionManager::run()
{
    DBG("Connection thread started - searching for IMU devices...");

    // Callback for when connection is successfully established
    auto onConnectionSuccess = [this]() {
        isConnected = true;
        DBG("IMU device connection established!");
        
        // Start gesture processing once we have a connection
        if (gestureManager)
        {
            gestureManager->startPolling();
        }
    };

    // Main connection loop
    while (!threadShouldExit())
    {
        try
        {
            ximu3::NetworkAnnouncement networkAnnouncement;
            
            if (networkAnnouncement.getResult() != ximu3::XIMU3_ResultOk)
            {
                DBG("Unable to open network announcement socket - retrying in 5 seconds...");
                isConnected = false;
                juce::Thread::sleep(5000);
                continue;
            }

            // Look for available devices
            DBG("Scanning for IMU devices...");
            const auto messages = networkAnnouncement.getMessagesAfterShortDelay();

            if (messages.empty())
            {
                isConnected = false;
                DBG("No IMU devices found - retrying in 3 seconds...");
                juce::Thread::sleep(3000);
                continue;
            }

            // Use the first available device
            const auto& udpInfo = ximu3::XIMU3_network_announcement_message_to_udp_connection_info(messages[0]);
            const auto& connectionInfo = ximu3::UdpConnectionInfo(udpInfo);

            DBG("Found IMU device, attempting to connect: " << connectionInfo.toString());

            // This will block until connection is lost or thread should exit
            connectionHandler->runConnection(
                connectionInfo,
                [this]() { return threadShouldExit(); },
                onConnectionSuccess
            );

            // If we reach here, the connection was lost (unless we're exiting)
            if (!threadShouldExit())
            {
                isConnected = false;
                DBG("Connection lost - will retry in 3 seconds...");
                
                // Stop gesture processing until reconnected
                if (gestureManager)
                {
                    gestureManager->stopPolling();
                }
                
                juce::Thread::sleep(3000);
            }
        }
        catch (const std::exception& e)
        {
            DBG("Exception in connection thread: " << e.what());
            isConnected = false;
            juce::Thread::sleep(5000);
        }
    }

    isConnected = false;
    DBG("Connection thread exiting");
}
