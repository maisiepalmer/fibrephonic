/**
 * @file TextileGestureDetector.h
 * @brief Directional tap and stroke detection for fabric-based IMU sensors
 * @author Maisie Palmer
 * @date 22 Sep 2025
 */

#pragma once

#include <vector>
#include <deque>
#include <cmath>
#include <string>

class TextileGestureDetector
{
public:
    /** @brief Gesture types for fabric interaction */
    enum GestureType
    {
        NO_GESTURE,
        TAP_SOFT,
        TAP_HARD,
        STROKE_UP,
        STROKE_DOWN,
        STROKE_LEFT,
        STROKE_RIGHT
    };

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

    /** @brief Configurable thresholds for gesture detection */
    struct GestureThresholds
    {
        float tapSoftThreshold;   ///< Min accel for soft tap
        float tapHardThreshold;   ///< Min accel for hard tap
        float strokeMinAccel;     ///< Min net movement for stroke
    };

    /** @brief Constructor with thresholds */
    TextileGestureDetector(const GestureThresholds& thresholds);

    /** @brief Process new IMU data and detect gesture */
    GestureType processIMUData(const IMUData& newData);

    /** @brief Update detection thresholds */
    void setThresholds(const GestureThresholds& newThresholds);

    /** @brief Human-readable gesture name */
    std::string getGestureName(GestureType gesture) const;

private:
    GestureThresholds thresholds;

    std::deque<IMUData> dataBuffer;
    static constexpr size_t BUFFER_SIZE = 30;      ///< Rolling buffer size (~0.3s)
    static constexpr size_t STROKE_WINDOW = 10;   ///< Window size for strokes
    GestureType lastGesture = NO_GESTURE;
    int gestureCooldown = 0;

    // Math helpers
    float calculateMagnitude(const IMUData& data) const;
    float calculateMean(const std::vector<float>& data) const;
    float calculateVariance(const std::vector<float>& data) const;

    // Gesture detectors
    GestureType detectTap(const std::vector<IMUData>& window) const;
    GestureType detectStroke(const std::vector<IMUData>& window) const;
};
