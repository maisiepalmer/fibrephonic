#pragma once

#include <x-IMU3/Cpp/Ximu3.hpp>
#include <functional>
#include <JuceHeader.h>
#include <vector>
#include "../imuExamples/Helpers.hpp"

#define TIMESTAMP_FORMAT "%8" PRIu64 " us"
#define UINT32_FORMAT " %8" PRIu32
#define UINT64_FORMAT " %8" PRIu64
#define FLOAT_FORMAT " %8.3f"
#define STRING_FORMAT " \"%s\""

class BluetoothConnectionManager; // Forward declaration

class Connection
{
public:
    explicit Connection(BluetoothConnectionManager* parent = nullptr);

    void runconnection(const ximu3::ConnectionInfo& connectionInfo);

    float gx, gy, gz;
    float accx, accy, accz;

    inline double getX() const { return gx; }
    inline double getY() const { return gy; }
    inline double getZ() const { return gz; }

    inline double getaccX() const { return accx; }
    inline double getaccY() const { return accy; }
    inline double getaccZ() const { return accz; }

private:
    BluetoothConnectionManager* parentManager = nullptr;

    std::function<void(ximu3::XIMU3_DecodeError error)> decodeErrorCallback;
    std::function<void(ximu3::XIMU3_Statistics statistics)> statisticsCallback;
    std::function<void(ximu3::XIMU3_InertialMessage message)> inertialCallback;
    std::function<void()> endOfFileCallback;

    void setupCallbacks(); 
};

