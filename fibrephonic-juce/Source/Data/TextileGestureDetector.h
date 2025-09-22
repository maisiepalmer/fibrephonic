/**
 * @file TextileGestureDetector.h
 * @brief Simplified gesture detection for fabric-based IMU sensors
 * @author Maisie Palmer
 * @date 22 Sep 2025
 *
 * Focused on expressive textile interactions: Tap, Stroke, Stretch, Flutter, Wave.
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
        TAP,        ///< Sharp pat/tap
        STROKE,     ///< Smooth directional stroke
        STRETCH,    ///< Pulling/elongating fabric
        FLUTTER,    ///< Rapid flutter/shake
        WAVE        ///< Optional flowing wave motion
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
        float tapThreshold;       ///< Accel spike for tap
        float strokeThreshold;    ///< Accel/gyro change for stroke
        float stretchThreshold;   ///< Gradual accel change for stretch
        float flutterVariance;    ///< Variance for flutter
        float waveThreshold;      ///< Oscillation energy for wave
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
    static constexpr size_t BUFFER_SIZE = 50;      ///< Rolling buffer size
    static constexpr size_t GESTURE_WINDOW = 20;   ///< Window for analysis
    GestureType lastGesture = NO_GESTURE;
    int gestureCooldown = 0;

    // Math helpers
    float calculateMagnitude(float x, float y, float z) const;
    float calculateMean(const std::vector<float>& data) const;
    float calculateVariance(const std::vector<float>& data) const;

    // Gesture detectors
    bool detectTap(const std::vector<IMUData>& window) const;
    bool detectStroke(const std::vector<IMUData>& window) const;
    bool detectStretch(const std::vector<IMUData>& window) const;
    bool detectFlutter(const std::vector<IMUData>& window) const;
    GestureType detectWave(const std::vector<IMUData>& window) const;
};
