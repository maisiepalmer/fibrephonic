//
//  Ximu3DeviceManager.h
//  ximu3Connect - App
//
//  Created by Tom Mitchell on 06/03/2025.
//

#pragma once

#include <JuceHeader.h>
#include <x-IMU3/Cpp/Ximu3.hpp>

class Ximu3DeviceManager : private Thread
{
public:
    Ximu3DeviceManager() : Thread("Ximu3DeviceManager Thread")
    {

    }
    ~Ximu3DeviceManager() = default;

private:
    void run() override
    {

    }
};
