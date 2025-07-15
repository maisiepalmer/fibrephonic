#pragma once

#include "Connection.h"

class BluetoothConnection : public Connection
{
public:
    std::unique_ptr<ximu3::ConnectionInfo> connectionInfo;

    BluetoothConnection()
    {
        if (helpers::yesOrNo("Search for connections?"))
        {
            const auto devices = ximu3::PortScanner::scanFilter(ximu3::XIMU3_ConnectionTypeBluetooth);

            if (devices.empty())
            {
                std::cout << "No Bluetooth connections available" << std::endl;
                return;
            }

            std::cout << "Found " << devices[0].device_name << " " << devices[0].serial_number << std::endl;

            auto connectionInfo = ximu3::connectionInfoFrom(devices[0]);
        }
        else
        {
            // Replace with your actual fallback port
            auto connectionInfo = std::make_unique<ximu3::BluetoothConnectionInfo>("COM1");
        }
    }

    // Defer running the connection so it can be controlled externally
    void connectAndRun(std::function<bool()> shouldExit)
    {
        if (connectionInfo)
            runconnection(*connectionInfo, shouldExit);
    }
};

