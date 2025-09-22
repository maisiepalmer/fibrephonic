#pragma once

#include <x-IMU3/Cpp/Ximu3.hpp>
#include <functional>
#include <JuceHeader.h>
#include <vector>

#define TIMESTAMP_FORMAT "%8" PRIu64 " us"
#define UINT32_FORMAT " %8" PRIu32
#define UINT64_FORMAT " %8" PRIu64
#define FLOAT_FORMAT " %8.3f"
#define STRING_FORMAT " \"%s\""

class ConnectionManager;

class Connection
{
public:
    explicit Connection(ConnectionManager* parent = nullptr);

    void runConnection(const ximu3::ConnectionInfo& connectionInfo,
                       std::function<bool()> shouldExit,
                       std::function<void()> onConnectionSuccess);

private:
    ConnectionManager* parentManager = nullptr;

    std::function<void(ximu3::XIMU3_DecodeError error)> decodeErrorCallback;
    std::function<void(ximu3::XIMU3_Statistics statistics)> statisticsCallback;
    std::function<void(ximu3::XIMU3_InertialMessage message)> inertialCallback;
    std::function<void(ximu3::XIMU3_MagnetometerMessage message)> magnetometerCallback;
    std::function<void()> endOfFileCallback;

    void setupCallbacks();
};
