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
: juce::Thread("Bluetooth Connection Thread")
, gestureManager(gestureManagerInstance)
{
    gyroscopeX = gyroscopeY = gyroscopeZ = 0;
    accelerationX = accelerationY = accelerationZ = 0;
    
    // The connection handler is created here with a pointer to this manager.
    connectionHandler = std::make_unique<Connection>(this);
}

ConnectionManager::~ConnectionManager()
{
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
    if (threadShouldExit())
    {
        isConnected = false;
        return;
    }
    
    // This callback will be called by the Connection class when the connection is open.
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
    
    const auto messages = networkAnnouncement.getMessagesAfterShortDelay();
    
    if (messages.size() == 0)
    {
        isConnected = false;
        std::cout << "No devices found" << std::endl;
        return;
    }
    
    const auto& udpInfo = ximu3::XIMU3_network_announcement_message_to_udp_connection_info(messages[0]);
    const auto& connectionInfo = ximu3::UdpConnectionInfo (ximu3::UdpConnectionInfo (udpInfo));
    
    connectionHandler->runConnection(connectionInfo, [this]() { return threadShouldExit(); }, onConnectionSuccess);
    
    // Keep the thread alive while the connection is active.
    while (!threadShouldExit())
    {
        wait(pollRate);
    }
    
    DBG("Thread exiting gracefully.");
}
