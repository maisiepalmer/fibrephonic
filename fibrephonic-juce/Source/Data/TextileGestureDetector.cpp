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

// -------------------- Math Helpers --------------------
float TextileGestureDetector::calculateMagnitude(const IMUData& data) const
{
    // Include accel, gyro, and mag for optional directional robustness
    return std::sqrt(data.accelX*data.accelX +
                     data.accelY*data.accelY +
                     data.accelZ*data.accelZ +
                     data.gyroX*data.gyroX +
                     data.gyroY*data.gyroY +
                     data.gyroZ*data.gyroZ +
                     data.magX*data.magX +
                     data.magY*data.magY +
                     data.magZ*data.magZ);
}

float TextileGestureDetector::calculateMean(const std::vector<float>& data) const
{
    if (data.empty()) return 0.0f;
    float sum = 0.0f;
    for (float v : data) sum += v;
    return sum / data.size();
}

float TextileGestureDetector::calculateVariance(const std::vector<float>& data) const
{
    if (data.size() < 2) return 0.0f;
    float mean = calculateMean(data);
    float var = 0.0f;
    for (float v : data) var += (v - mean) * (v - mean);
    return var / (data.size() - 1);
}

// -------------------- Tap Detection --------------------
TextileGestureDetector::GestureType TextileGestureDetector::detectTap(const std::vector<IMUData>& window) const
{
    if (window.empty()) return NO_GESTURE;

    float maxAccel = 0.0f;
    for (const auto& d : window)
    {
        float mag = std::sqrt(d.accelX*d.accelX +
                              d.accelY*d.accelY +
                              d.accelZ*d.accelZ);
        if (mag > maxAccel) maxAccel = mag;
    }

    if (maxAccel > thresholds.tapHardThreshold) return TAP_HARD;
    if (maxAccel > thresholds.tapSoftThreshold) return TAP_SOFT;
    return NO_GESTURE;
}

// -------------------- Stroke Detection --------------------
TextileGestureDetector::GestureType TextileGestureDetector::detectStroke(const std::vector<IMUData>& window) const
{
    if (window.size() < 2) return NO_GESTURE;

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
        return NO_GESTURE;

    if (absX > absY) return (sumX > 0) ? STROKE_RIGHT : STROKE_LEFT;
    else return (sumY > 0) ? STROKE_UP : STROKE_DOWN;
}

// -------------------- Main Processing --------------------
TextileGestureDetector::GestureType TextileGestureDetector::processIMUData(const IMUData& newData)
{
    // Add new data to buffer
    dataBuffer.push_back(newData);
    if (dataBuffer.size() > BUFFER_SIZE) dataBuffer.pop_front();

    // Handle cooldown
    if (gestureCooldown > 0)
    {
        gestureCooldown--;
        return NO_GESTURE;
    }

    // Always analyze last few samples for tap
    std::vector<IMUData> tapWindow(dataBuffer.end() - std::min<size_t>(STROKE_WINDOW, dataBuffer.size()),
                                   dataBuffer.end());

    GestureType detected = detectTap(tapWindow);
    if (detected != NO_GESTURE)
    {
        // Tap detected → set cooldown and mark as stroke candidate
        lastGesture = detected;
        gestureCooldown = 5;  // short delay before next detection
        return detected;
    }

    // Only detect stroke **if last gesture was a tap**
    if (lastGesture == TAP_SOFT || lastGesture == TAP_HARD)
    {
        std::vector<IMUData> strokeWindow(dataBuffer.end() - std::min<size_t>(STROKE_WINDOW, dataBuffer.size()),
                                          dataBuffer.end());
        detected = detectStroke(strokeWindow);
        if (detected != NO_GESTURE)
        {
            // Stroke detected → reset lastGesture to avoid repeated strokes
            lastGesture = NO_GESTURE;
            gestureCooldown = 10;  // prevent double detection
            return detected;
        }
    }

    return NO_GESTURE;
}

// -------------------- Gesture Name --------------------
std::string TextileGestureDetector::getGestureName(GestureType g) const
{
    switch(g)
    {
        case NO_GESTURE: return "None";
        case TAP_SOFT: return "Tap Soft";
        case TAP_HARD: return "Tap Hard";
        case STROKE_UP: return "Stroke Up";
        case STROKE_DOWN: return "Stroke Down";
        case STROKE_LEFT: return "Stroke Left";
        case STROKE_RIGHT: return "Stroke Right";
        default: return "Unknown";
    }
}
