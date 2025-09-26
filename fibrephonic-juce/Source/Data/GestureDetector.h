//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * Portions of this code adapted from Mi.mu Gloves DrumDetector and GestureDetector
 * Original Mi.mu code: http://www.mimugloves.com
 *
 * Adapted for textile gesture detection research
 */
//======================================================================

#pragma once
#include <deque>
#include <vector>
#include <cmath>
#include <numeric>
#include "../Helpers.h"

/**
 * Textile gesture detector focusing on calibration + tap detection
 *
 * Key components adapted from Mi.mu Gloves codebase:
 * - Calibration system: baseline mean/std calculation approach
 * - Tap detection: DrumDetector algorithm with threshold management
 * - Buffer management: circular buffer pattern for sensor data
 */
class GestureDetector
{
public:
    struct Calibration
    {
        float baselineMagnitude = 0.0f;
        float baselineStd = 1.0f;
        bool calibrated = false;
        
        // Individual axis baselines for directional analysis
        float baselineX = 0.0f, baselineY = 0.0f, baselineZ = 0.0f;
        float stdX = 1.0f, stdY = 1.0f, stdZ = 1.0f;
    };

    GestureDetector(size_t bufferSize = 100);

    // Core functions
    void pushSample(const IMUData& sample);
    float detectTap();  // Returns velocity if tap detected, 0 otherwise
    
    // Calibration
    void startCalibration();
    void stopCalibration();
    void resetCalibration();
    bool isCalibrated() const { return calib.calibrated; }
    Calibration getCalibration() const { return calib; }
    
    // Getters for Max/MSP streaming
    float getMagnitude() const;
    float getCalibratedMagnitude() const;
    float getCalibratedX() const;
    float getCalibratedY() const;
    float getCalibratedZ() const;
    
    // Settings - drum detector style thresholds
    void setTapThreshold(float v) { tapThreshold = v; }
    void setGyroThreshold(float v) { gyroThreshold = v; }
    
    // Access to buffer for analysis
    const std::deque<IMUData>& getBuffer() const { return buffer; }
    
    // Directional analysis (adapted from Mi.mu DirectionProcessor)
    struct DirectionalInfo
    {
        float tiltX = 0.0f;      // Normalised tilt in X axis (-1 to 1)
        float tiltY = 0.0f;      // Normalised tilt in Y axis (-1 to 1)
        float tiltZ = 0.0f;      // Normalised tilt in Z axis (-1 to 1)
        float magnitude = 0.0f;  // Overall movement magnitude
        bool isMoving = false;   // Whether above movement threshold
    };

    DirectionalInfo getDirectionalInfo() const;

private:
    std::deque<IMUData> buffer;
    std::deque<IMUData> calibrationBuffer;
    size_t maxBuffer;
    Calibration calib;
    bool calibrating = false;
    
    // Drum-style tap detection (adapted from Mi.mu DrumDetector)
    float tapThreshold = 5.f;      // Gyroscope threshold
    float gyroThreshold = 5.f;     // Secondary threshold
    float offThreshold = 5.f;
    bool tapPending = false;
    int countDownTimer = 0;
    float sampleRate = 100.0f;
    std::deque<float> tapBuffer;      // Store recent gyro values for velocity calc
    
    // Helper functions
    float magnitude(const IMUData& d) const;
    float mean(const std::vector<float>& v) const;
    float stddev(const std::vector<float>& v, float m) const;
    void calculateCalibration();
    
    // Tap detection helpers (from Mi.mu DrumDetector)
    bool isThresholdExceeded(float input);
    float getMaxMagnitude();
};
