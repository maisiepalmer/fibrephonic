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
#include "../Helpers.h"

class TextileGestureDetector
{
public:
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
    Gestures::GestureType processIMUData(const IMUData& newData);

    /** @brief Update detection thresholds */
    void setThresholds(const GestureThresholds& newThresholds);
    
    const std::deque<IMUData>& getBuffer() const { return dataBuffer; }

private:
    GestureThresholds thresholds;

    std::deque<IMUData> dataBuffer;
    static constexpr size_t BUFFER_SIZE = 30;      ///< Rolling buffer size (~0.3s)
    static constexpr size_t STROKE_WINDOW = 10;   ///< Window size for strokes
    Gestures::GestureType lastGesture = Gestures::NO_GESTURE;
    int gestureCooldown = 0;

    // Gesture detectors
    Gestures::GestureType detectTap(const std::vector<IMUData>& window) const;
    Gestures::GestureType detectStroke(const std::vector<IMUData>& window) const;
};
