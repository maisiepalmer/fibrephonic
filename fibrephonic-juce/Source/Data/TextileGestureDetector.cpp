#include "TextileGestureDetector.h"
#include <algorithm>
#include <cmath>
#include <vector>

TextileGestureDetector::TextileGestureDetector(const GestureThresholds& thresholds)
    : thresholds(thresholds) {}

void TextileGestureDetector::setThresholds(const GestureThresholds& newThresholds)
{
    thresholds = newThresholds;
}

// -------------------- Tap Detection --------------------
Gestures::GestureType TextileGestureDetector::detectTap(const std::vector<IMUData>& window) const
{
    if (window.empty()) return Gestures::NO_GESTURE;

    float maxAccel = 0.0f;
    for (const auto& d : window)
    {
        float mag = std::sqrt(d.accelX*d.accelX +
                              d.accelY*d.accelY +
                              d.accelZ*d.accelZ);
        if (mag > maxAccel) maxAccel = mag;
    }

    if (maxAccel > thresholds.tapHardThreshold) return Gestures::TAP_HARD;
    if (maxAccel > thresholds.tapSoftThreshold) return Gestures::TAP_SOFT;
    return Gestures::NO_GESTURE;
}

// -------------------- Stroke Detection --------------------
Gestures::GestureType TextileGestureDetector::detectStroke(const std::vector<IMUData>& window) const
{
    if (window.size() < 2) return Gestures::NO_GESTURE;

    // Compute net movement for X and Y using accel + optional gyro
    float sumX = 0.0f, sumY = 0.0f;
    for (const auto& d : window)
    {
        sumX += d.accelX + d.gyroX + d.magX;
        sumY += d.accelY + d.gyroY + d.magY;
    }

    float absX = std::abs(sumX);
    float absY = std::abs(sumY);

    if (absX < thresholds.strokeMinAccel && absY < thresholds.strokeMinAccel)
        return Gestures::NO_GESTURE;

    if (absX > absY) return (sumX > 0) ? Gestures::STROKE_RIGHT : Gestures::STROKE_LEFT;
    else return (sumY > 0) ? Gestures::STROKE_UP : Gestures::STROKE_DOWN;
}

// -------------------- Main Processing --------------------
Gestures::GestureType TextileGestureDetector::processIMUData(const IMUData& newData)
{
    // Add new data to buffer
    dataBuffer.push_back(newData);
    if (dataBuffer.size() > BUFFER_SIZE) dataBuffer.pop_front();

    
    std::string label = getGestureName(Gestures::TAP_SOFT); // EDIT WHEN CHANGING
    

    return Gestures::NO_GESTURE;
}
