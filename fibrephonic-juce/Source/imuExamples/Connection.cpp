/*
  ==============================================================================

    Connection.cpp
    Created: 24 Jun 2025 12:03:11pm
    Author:  Joseph B

  ==============================================================================
*/

#include "Connection.h"
#include "../Data/BluetoothConnectionManager.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <inttypes.h>

Connection::Connection(BluetoothConnectionManager* parent)
    : parentManager(parent)
{
    setupCallbacks();
}

void Connection::setupCallbacks()
{
    // Use this function to declare which Pre defined IMU callbacks are to be used...
    // Also route as to where the data is going/ being passed to.

    decodeErrorCallback = [](auto error)
        {
            std::cout << ximu3::XIMU3_decode_error_to_string(error) << std::endl;
        };

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

    endOfFileCallback = []
        {
            std::cout << "End of file" << std::endl;
        };
}

void Connection::runconnection(const ximu3::ConnectionInfo& connectionInfo)
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

    const std::vector<std::string> commands{ "{\"strobe\":null}" };
    connection.sendCommands(commands, 2, 500);

    std::this_thread::sleep_for(std::chrono::seconds(60));
    connection.close();
}


