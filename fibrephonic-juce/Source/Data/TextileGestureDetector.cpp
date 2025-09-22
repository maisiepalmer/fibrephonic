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

float TextileGestureDetector::calculateMagnitude(float x, float y, float z) const
{
    return std::sqrt(x*x + y*y + z*z);
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

// -------------------- Gesture Detectors --------------------

bool TextileGestureDetector::detectTap(const std::vector<IMUData>& window) const
{
    if (window.size() < 5) return false;
    float maxAccel = 0.0f;
    for (const auto& d : window)
    {
        float mag = calculateMagnitude(d.accelX, d.accelY, d.accelZ);
        if (mag > maxAccel) maxAccel = mag;
    }
    return maxAccel > thresholds.tapThreshold;
}

bool TextileGestureDetector::detectStroke(const std::vector<IMUData>& window) const
{
    if (window.size() < 5) return false;
    float totalGyro = 0.0f;
    for (const auto& d : window)
        totalGyro += calculateMagnitude(d.gyroX, d.gyroY, d.gyroZ);
    return totalGyro > thresholds.strokeThreshold;
}

bool TextileGestureDetector::detectStretch(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return false;
    float startAccel = calculateMagnitude(window.front().accelX, window.front().accelY, window.front().accelZ);
    float endAccel = calculateMagnitude(window.back().accelX, window.back().accelY, window.back().accelZ);
    return std::abs(endAccel - startAccel) > thresholds.stretchThreshold;
}

bool TextileGestureDetector::detectFlutter(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return false;
    std::vector<float> mags;
    for (const auto& d : window)
        mags.push_back(calculateMagnitude(d.accelX, d.accelY, d.accelZ));
    return calculateVariance(mags) > thresholds.flutterVariance;
}

TextileGestureDetector::GestureType TextileGestureDetector::detectWave(const std::vector<IMUData>& window) const
{
    if (window.size() < 15) return NO_GESTURE;
    float gyroSum = 0.0f;
    for (const auto& d : window)
        gyroSum += calculateMagnitude(d.gyroX, d.gyroY, d.gyroZ);
    return (gyroSum > thresholds.waveThreshold) ? WAVE : NO_GESTURE;
}

// -------------------- Main Processing --------------------

TextileGestureDetector::GestureType TextileGestureDetector::processIMUData(const IMUData& newData)
{
    dataBuffer.push_back(newData);
    if (dataBuffer.size() > BUFFER_SIZE) dataBuffer.pop_front();
    if (dataBuffer.size() < GESTURE_WINDOW) return NO_GESTURE;

    if (gestureCooldown > 0)
    {
        gestureCooldown--;
        return NO_GESTURE;
    }

    std::vector<IMUData> shortWindow(dataBuffer.end() - 10, dataBuffer.end());
    std::vector<IMUData> mediumWindow(dataBuffer.end() - GESTURE_WINDOW, dataBuffer.end());

    GestureType detected = NO_GESTURE;

    if (detectTap(shortWindow)) { detected = TAP; gestureCooldown = 15; }
    else if (detectFlutter(shortWindow)) { detected = FLUTTER; gestureCooldown = 20; }
    else if (detectStretch(mediumWindow)) { detected = STRETCH; gestureCooldown = 30; }
    else
    {
        GestureType waveG = detectWave(mediumWindow);
        if (waveG != NO_GESTURE) { detected = waveG; gestureCooldown = 25; }
        else if (detectStroke(mediumWindow)) { detected = STROKE; gestureCooldown = 20; }
    }

    if (detected != NO_GESTURE) lastGesture = detected;
    return detected;
}

std::string TextileGestureDetector::getGestureName(GestureType g) const
{
    switch(g)
    {
        case NO_GESTURE: return "None";
        case TAP: return "Tap";
        case STROKE: return "Stroke";
        case STRETCH: return "Stretch";
        case FLUTTER: return "Flutter";
        case WAVE: return "Wave";
        default: return "Unknown";
    }
}
