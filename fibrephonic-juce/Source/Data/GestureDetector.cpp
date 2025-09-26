//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * Portions of this code adapted from Mi.mu Gloves DrumDetector and GestureDetector
 * Original Mi.mu code: http://www.mimugloves.com
 *
 * Adapted for textile gesture detection research
 */
//======================================================================

#include "GestureDetector.h"
#include <algorithm>

GestureDetector::GestureDetector(size_t bufferSize)
    : maxBuffer(bufferSize), offThreshold(tapThreshold) {}

float GestureDetector::magnitude(const IMUData& d) const
{
    return std::sqrt(d.accelX*d.accelX + d.accelY*d.accelY + d.accelZ*d.accelZ);
}

float GestureDetector::mean(const std::vector<float>& v) const
{
    if (v.empty()) return 0.0f;
    return std::accumulate(v.begin(), v.end(), 0.0f) / v.size();
}

float GestureDetector::stddev(const std::vector<float>& v, float m) const
{
    if (v.size() < 2) return 0.0f;
    float s = 0.0f;
    for (auto x : v) s += (x-m)*(x-m);
    return std::sqrt(s / (v.size()-1));
}

void GestureDetector::pushSample(const IMUData& sample)
{
    buffer.push_back(sample);
    if (buffer.size() > maxBuffer)
        buffer.pop_front();

    if (calibrating)
    {
        calibrationBuffer.push_back(sample);
    }
}

void GestureDetector::startCalibration()
{
    calibrating = true;
    calibrationBuffer.clear();
    calib.calibrated = false;
}

void GestureDetector::stopCalibration()
{
    calibrating = false;
    if (!calibrationBuffer.empty())
    {
        calculateCalibration();
        calib.calibrated = true;
    }
}

// Calibration approach adapted from Mi.mu GestureDetector
// Uses statistical baseline (mean + standard deviation) for threshold normalization
void GestureDetector::calculateCalibration()
{
    std::vector<float> magnitudes, xVals, yVals, zVals;
    
    for (const auto& sample : calibrationBuffer)
    {
        magnitudes.push_back(magnitude(sample));
        xVals.push_back(sample.accelX);
        yVals.push_back(sample.accelY);
        zVals.push_back(sample.accelZ);
    }
    
    // Overall magnitude baseline - Mi.mu approach for gesture normalization
    calib.baselineMagnitude = mean(magnitudes);
    calib.baselineStd = stddev(magnitudes, calib.baselineMagnitude);
    
    // Individual axis baselines for directional analysis
    calib.baselineX = mean(xVals);
    calib.baselineY = mean(yVals);
    calib.baselineZ = mean(zVals);
    calib.stdX = stddev(xVals, calib.baselineX);
    calib.stdY = stddev(yVals, calib.baselineY);
    calib.stdZ = stddev(zVals, calib.baselineZ);
}

void GestureDetector::resetCalibration()
{
    calib = Calibration{};
    calibrationBuffer.clear();
    calibrating = false;
}

// Adapted from Mi.mu DrumDetector for textile tap detection
// Original algorithm designed for detecting drum hits via gyroscope analysis
float GestureDetector::detectTap()
{
    if (buffer.empty()) return 0.0f;
    
    if (countDownTimer > 0)
        --countDownTimer;
    
    float input = buffer.back().gyroZ;
    
    tapBuffer.push_back(input);
    if (tapBuffer.size() > 50) // Keep last 50 samples (~0.5s at 100Hz)
        tapBuffer.pop_front();
    
    if (isThresholdExceeded(input))
    {
        if (countDownTimer == 0)
        {
            tapPending = true;
        }
        else
        {
            countDownTimer = static_cast<int>(0.01 * sampleRate); // Reset to 20ms
        }
    }
    else if (tapPending)
    {
        float velocity = getMaxMagnitude();
        tapPending = false;
        offThreshold = tapThreshold;
        tapBuffer.clear();
        countDownTimer = static_cast<int>(0.01 * sampleRate);
        return velocity;
    }
    
    return 0.0f;
}

// Directly adapted from Mi.mu DrumDetector::isThreshExceeded()
// Implements adaptive threshold with hysteresis for reliable detection
bool GestureDetector::isThresholdExceeded(float input)
{
    if (!tapPending)
    {
        return tapThreshold > 0.0f ? input > tapThreshold : input < tapThreshold;
    }
    else
    {
        if (tapThreshold > 0.0f)
        {
            if (input > offThreshold + 5)
            {
                offThreshold = input - 5;
            }
            return input > offThreshold;
        }
        else
        {
            if (input < offThreshold - 5)
            {
                offThreshold = input + 5;
            }
            return input < offThreshold;
        }
    }
}

// Adapted from Mi.mu DrumDetector::getMaxMagnitude()
// Returns peak velocity from recent samples for dynamics
float GestureDetector::getMaxMagnitude()
{
    if (tapBuffer.empty()) return 0.0f;
    
    if (tapThreshold > 0.0f)
    {
        return *std::max_element(tapBuffer.begin(), tapBuffer.end());
    }
    else
    {
        return std::abs(*std::min_element(tapBuffer.begin(), tapBuffer.end()));
    }
}

float GestureDetector::getMagnitude() const
{
    return buffer.empty() ? 0.0f : magnitude(buffer.back());
}

float GestureDetector::getCalibratedMagnitude() const
{
    if (buffer.empty() || !calib.calibrated) return 0.0f;
    return magnitude(buffer.back()) - calib.baselineMagnitude;
}

float GestureDetector::getCalibratedX() const
{
    if (buffer.empty() || !calib.calibrated) return 0.0f;
    return buffer.back().accelX - calib.baselineX;
}

float GestureDetector::getCalibratedY() const
{
    if (buffer.empty() || !calib.calibrated) return 0.0f;
    return buffer.back().accelY - calib.baselineY;
}

float GestureDetector::getCalibratedZ() const
{
    if (buffer.empty() || !calib.calibrated) return 0.0f;
    return buffer.back().accelZ - calib.baselineZ;
}

// Directional analysis adapted from Mi.mu DirectionProcessor concept
// Provides continuous directional information for Max/MSP analysis
GestureDetector::DirectionalInfo GestureDetector::getDirectionalInfo() const
{
    DirectionalInfo info;
    
    if (buffer.empty() || !calib.calibrated)
        return info;
    
    const auto& current = buffer.back();
    
    // Calculate normalised directional tilts based on calibrated baselines
    // Approach similar to Mi.mu's directional vector calculations
    float deltaX = current.accelX - calib.baselineX;
    float deltaY = current.accelY - calib.baselineY;
    float deltaZ = current.accelZ - calib.baselineZ;
    
    // Normalise by standard deviations (Mi.mu statistical approach)
    // This gives direction relative to calibrated neutral position
    if (calib.stdX > 0.001f) info.tiltX = std::clamp(deltaX / (3.0f * calib.stdX), -1.0f, 1.0f);
    if (calib.stdY > 0.001f) info.tiltY = std::clamp(deltaY / (3.0f * calib.stdY), -1.0f, 1.0f);
    if (calib.stdZ > 0.001f) info.tiltZ = std::clamp(deltaZ / (3.0f * calib.stdZ), -1.0f, 1.0f);
    
    // Overall movement magnitude
    info.magnitude = std::sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
    
    // Movement threshold - consider "moving" if above 2 standard deviations
    float movementThreshold = 2.0f * calib.baselineStd;
    info.isMoving = info.magnitude > movementThreshold;
    
    return info;
}
