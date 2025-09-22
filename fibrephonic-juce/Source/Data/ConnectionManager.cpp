/**
 * @file ConnectionManager.cpp
 * @brief Manages the connection to x-IMU3 devices
 * @author Maisie Palmer
 * @date Created: 20 Aug 2025
 */

#include "ConnectionManager.h"
#include "GestureManager.h"

ConnectionManager::ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance)
: juce::Thread("IMU Connection Thread")
, gestureManager(gestureManagerInstance)
{
    gyroscopeX = gyroscopeY = gyroscopeZ = 0;
    accelerationX = accelerationY = accelerationZ = 0;
    
    connectionHandler = std::make_unique<Connection>(this);
}

ConnectionManager::~ConnectionManager()
{
    signalThreadShouldExit();
    stopThread(2000);
}

void ConnectionManager::startConnection()
{
    startThread();
}

void ConnectionManager::stopConnection()
{
    signalThreadShouldExit();
    stopThread(2000);
    isConnected = false;
}

void ConnectionManager::run()
{
    auto onConnectionSuccess = [this]() {
        isConnected = true;
        if (auto gm = gestureManager.lock())
        {
            gm->startPolling();
        }
    };

    // Create network announcement for device discovery
    std::unique_ptr<ximu3::NetworkAnnouncement> networkAnnouncement;
    
    try
    {
        networkAnnouncement = std::make_unique<ximu3::NetworkAnnouncement>();
        
        if (networkAnnouncement->getResult() != ximu3::XIMU3_ResultOk)
        {
            isConnected = false;
            DBG("ERROR: Unable to open network announcement socket");
            DBG("Make sure x-IMU3 GUI is closed and port 10000 is available");
            return;
        }
    }
    catch (const std::exception& e)
    {
        DBG("Exception creating network announcement: " << e.what());
        return;
    }

    // Main connection loop
    int noDeviceCount = 0;
    
    while (!threadShouldExit())
    {
        try
        {
            const auto messages = networkAnnouncement->getMessagesAfterShortDelay();

            if (messages.empty())
            {
                if (isConnected)
                {
                    isConnected = false;
                    if (auto gm = gestureManager.lock())
                    {
                        gm->stopPolling();
                    }
                }
                
                // Log occasionally
                if (++noDeviceCount == 1 || noDeviceCount % 50 == 0)
                {
                    DBG("Searching for x-IMU3 devices...");
                }
                
                juce::Thread::sleep(100);
                continue;
            }

            // Device found
            noDeviceCount = 0;
            const auto& firstDevice = messages[0];
            
            DBG("Connected: " << firstDevice.device_name
                << " (Battery: " << static_cast<int>(firstDevice.battery) << "%)");
            
            const auto& udpInfo = ximu3::XIMU3_network_announcement_message_to_udp_connection_info(firstDevice);
            const auto& connectionInfo = ximu3::UdpConnectionInfo(udpInfo);

            // This blocks until device disconnects or thread exits
            connectionHandler->runConnection(connectionInfo,
                                            [this]() { return threadShouldExit(); },
                                            onConnectionSuccess);

            if (!threadShouldExit())
            {
                isConnected = false;
                DBG("Device disconnected");
                if (auto gm = gestureManager.lock())
                {
                    gm->stopPolling();
                }
                juce::Thread::sleep(2000);
            }
        }
        catch (const std::exception& e)
        {
            DBG("Connection error: " << e.what());
            isConnected = false;
            if (!threadShouldExit())
            {
                juce::Thread::sleep(2000);
            }
        }
    }

    isConnected = false;
}
