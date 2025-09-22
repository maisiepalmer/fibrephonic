/*
 ==============================================================================
 
 ConnectionManager.cpp
 Created: 20 Aug 2025 5:34:45pm
 Author:  Maisie Palmer
 
 ==============================================================================
 */

#include "ConnectionManager.h"
#include "GestureManager.h"

//==============================================================================
ConnectionManager::ConnectionManager(std::shared_ptr<GestureManager> gestureManagerInstance)
: juce::Thread("IMU Connection Thread")
, gestureManager(gestureManagerInstance)
{
    gyroscopeX = gyroscopeY = gyroscopeZ = 0;
    accelerationX = accelerationY = accelerationZ = 0;
    
    // The connection handler is created here with a pointer to this manager.
    connectionHandler = std::make_unique<Connection>(this);
}

ConnectionManager::~ConnectionManager()
{
    // The destructor needs to signal the thread to exit and wait for it.
    signalThreadShouldExit();
    stopThread(500);
}

//==============================================================================
void ConnectionManager::startConnection()
{
    startThread();
}

void ConnectionManager::stopConnection()
{
    signalThreadShouldExit();
    stopThread(500);
    isConnected = false;
    DBG("Connection stopped.");
}

void ConnectionManager::run()
{
    DBG("Connection thread started.");

    auto onConnectionSuccess = [this]() {
        isConnected = true;
        DBG("Connection established with device.");
        gestureManager->startPolling();
    };

    ximu3::NetworkAnnouncement networkAnnouncement;
    if (networkAnnouncement.getResult() != ximu3::XIMU3_ResultOk)
    {
        isConnected = false;
        std::cout << "Unable to open network announcement socket" << std::endl;
        return;
    }

    while (!threadShouldExit())
    {
        const auto messages = networkAnnouncement.getMessagesAfterShortDelay();

        if (messages.empty())
        {
            isConnected = false;
            DBG("No devices found. Retrying in 2 seconds...");
            juce::Thread::sleep(2000);
            continue;
        }

        const auto& udpInfo = ximu3::XIMU3_network_announcement_message_to_udp_connection_info(messages[0]);
        const auto& connectionInfo = ximu3::UdpConnectionInfo(udpInfo);

        DBG("Attempting to connect to device...");

        // This will block until threadShouldExit() is true OR the device disconnects
        connectionHandler->runConnection(connectionInfo,
                                         [this]() { return threadShouldExit(); },
                                         onConnectionSuccess);

        // If we get here, runConnection has ended (disconnected)
        if (!threadShouldExit())
        {
            isConnected = false;
            DBG("Connection lost. Retrying in 2 seconds...");
            gestureManager->stopPolling(); // stop polling until reconnected
            juce::Thread::sleep(2000);
        }
    }

    isConnected = false;
    DBG("Connection thread exiting.");
}
