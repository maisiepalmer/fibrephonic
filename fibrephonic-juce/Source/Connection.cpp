#include "Connection.h"
#include "Data/BluetoothConnectionManager.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <inttypes.h>

Connection::Connection(BluetoothConnectionManager* parent) : parentManager(parent)
{
    setupCallbacks();
}

void Connection::setupCallbacks()
{
    decodeErrorCallback = [](auto error){ std::cout << ximu3::XIMU3_decode_error_to_string(error) << std::endl; };
    
    statisticsCallback = [](auto statistics)
    {
    printf(TIMESTAMP_FORMAT UINT64_FORMAT " bytes" UINT32_FORMAT " bytes/s" UINT64_FORMAT " messages" UINT32_FORMAT " messages/s" UINT64_FORMAT " errors" UINT32_FORMAT " errors/s\n",
           statistics.timestamp,
           statistics.data_total,
           statistics.data_rate,
           statistics.message_total,
           statistics.message_rate,
           statistics.error_total,
           statistics.error_rate);
    };
    
    inertialCallback = [this](auto message)
    {
        if (parentManager)
        {
            parentManager->setGyroscopeValues(
                                              message.gyroscope_x,
                                              message.gyroscope_y,
                                              message.gyroscope_z);
            
            parentManager->setAccelerometerValues(
                                                  message.accelerometer_x,
                                                  message.accelerometer_y,
                                                  message.accelerometer_z);
        }
    };
    
    endOfFileCallback = []{ std::cout << "End of file" << std::endl; };
}

void Connection::runConnection(const ximu3::ConnectionInfo& connectionInfo, std::function<bool()> shouldExit, std::function<void()> onConnectionSuccess)
{
    ximu3::Connection connection(connectionInfo);
    
    connection.addDecodeErrorCallback(decodeErrorCallback);
    connection.addStatisticsCallback(statisticsCallback);
    connection.addInertialCallback(inertialCallback);
    connection.addEndOfFileCallback(endOfFileCallback);
    
    if (connection.open() != ximu3::XIMU3_ResultOk)
    {
        std::cout << "Unable to open " << connectionInfo.toString() << std::endl;
        return;
    }
    
    // After a successful connection, we must call the callback
    onConnectionSuccess();
    
    const std::vector<std::string> commands{ "{\"strobe\":null}" };
    connection.sendCommands(commands, 2, 500);
    
    // Interruptible sleep
    for (int i = 0; i < 600 && !shouldExit(); ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    connection.close();
}
