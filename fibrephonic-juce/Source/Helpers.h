/*
  ==============================================================================

    Helpers.h
    Created: 23 Sep 2025 12:39:54pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

/** @brief IMU data container */
struct IMUData
{
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float magX, magY, magZ;

    IMUData() : accelX(0), accelY(0), accelZ(0),
                gyroX(0), gyroY(0), gyroZ(0),
                magX(0), magY(0), magZ(0) {}

    IMUData(float ax, float ay, float az,
            float gx, float gy, float gz,
            float mx, float my, float mz)
        : accelX(ax), accelY(ay), accelZ(az),
          gyroX(gx), gyroY(gy), gyroZ(gz),
          magX(mx), magY(my), magZ(mz) {}
};

struct Gestures
{
    /** @brief Gesture types for fabric interaction */
    enum GestureType
    {
        NONE,
        TAP,
        STROKE_UP,
        STROKE_DOWN,
        STROKE_LEFT,
        STROKE_RIGHT
    };

    static std::string getGestureName(GestureType g)
    {
        switch(g)
        {
            case NONE: return "None";
            case TAP: return "Tap";
            case STROKE_UP: return "Stroke Up";
            case STROKE_DOWN: return "Stroke Down";
            case STROKE_LEFT: return "Stroke Left";
            case STROKE_RIGHT: return "Stroke Right";
            default: return "Unknown";
        }
    }

};
