/**
 * @file GestureDetector.h
 * @author Maisie Palmer
 * @brief Gesture detection system with calibration support
 * @date 24 Sep 2025 11:16:18am
 */

#pragma once
#include <JuceHeader.h>
#include "../Helpers.h"
#include <vector>
#include <deque>
#include <cmath>
#include <numeric>

class GestureDetector
{
public:
    enum class GestureType
    {
        NONE,
        TAP,
        STROKE
    };
    
    enum class TapStrength
    {
        SOFT,
        MEDIUM,
        HARD
    };
    
    enum class StrokeDirection
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };
    
    struct CalibrationData
    {
        // Neutral position (resting state)
        float accelX = 0, accelY = 0, accelZ = 9.8f;
        float gyroX = 0, gyroY = 0, gyroZ = 0;
        float magX = 0, magY = 0, magZ = 0;
        
        // Noise thresholds (calculated during calibration)
        float accelNoiseThreshold = 0.5f;
        float gyroNoiseThreshold = 5.0f;
        
        // Gesture thresholds (can be tuned)
        float tapThresholdSoft = 2.0f;    // g units
        float tapThresholdHard = 5.0f;    // g units
        float strokeGyroThreshold = 30.0f; // deg/s
        
        bool isCalibrated = false;
    };
    
    struct DetectionResult
    {
        GestureType type = GestureType::NONE;
        TapStrength tapStrength = TapStrength::SOFT;
        StrokeDirection strokeDirection = StrokeDirection::UP;
        float confidence = 0.0f;
        bool valid = false;
        
        juce::String toString() const
        {
            if (type == GestureType::TAP)
            {
                juce::String strength = (tapStrength == TapStrength::HARD) ? "Hard" :
                                       (tapStrength == TapStrength::MEDIUM) ? "Medium" : "Soft";
                return "Tap (" + strength + ")";
            }
            else if (type == GestureType::STROKE)
            {
                juce::String dir;
                switch(strokeDirection)
                {
                    case StrokeDirection::UP: dir = "Up"; break;
                    case StrokeDirection::DOWN: dir = "Down"; break;
                    case StrokeDirection::LEFT: dir = "Left"; break;
                    case StrokeDirection::RIGHT: dir = "Right"; break;
                }
                return "Stroke " + dir;
            }
            return "None";
        }
    };
    
    GestureDetector()
    {
        buffer.clear();
        calibrationBuffer.clear();
    }
    
    // Start calibration (collect neutral position data)
    void startCalibration()
    {
        calibrationBuffer.clear();
        isCalibrating = true;
        calibrationStartTime = juce::Time::getCurrentTime();
    }
    
    // Stop calibration and calculate neutral position
    void stopCalibration()
    {
        if (!isCalibrating || calibrationBuffer.empty())
            return;
            
        isCalibrating = false;
        
        // Calculate average values for neutral position
        CalibrationData newCalib;
        float n = calibrationBuffer.size();
        
        for (const auto& sample : calibrationBuffer)
        {
            newCalib.accelX += sample.accelX / n;
            newCalib.accelY += sample.accelY / n;
            newCalib.accelZ += sample.accelZ / n;
            newCalib.gyroX += sample.gyroX / n;
            newCalib.gyroY += sample.gyroY / n;
            newCalib.gyroZ += sample.gyroZ / n;
            newCalib.magX += sample.magX / n;
            newCalib.magY += sample.magY / n;
            newCalib.magZ += sample.magZ / n;
        }
        
        // Calculate noise thresholds (standard deviation)
        float accelVar = 0, gyroVar = 0;
        for (const auto& sample : calibrationBuffer)
        {
            float dax = sample.accelX - newCalib.accelX;
            float day = sample.accelY - newCalib.accelY;
            float daz = sample.accelZ - newCalib.accelZ;
            accelVar += (dax*dax + day*day + daz*daz) / (3.0f * n);
            
            float dgx = sample.gyroX - newCalib.gyroX;
            float dgy = sample.gyroY - newCalib.gyroY;
            float dgz = sample.gyroZ - newCalib.gyroZ;
            gyroVar += (dgx*dgx + dgy*dgy + dgz*dgz) / (3.0f * n);
        }
        
        // Set thresholds at 3 standard deviations above noise
        newCalib.accelNoiseThreshold = 3.0f * std::sqrt(accelVar);
        newCalib.gyroNoiseThreshold = 3.0f * std::sqrt(gyroVar);
        
        newCalib.isCalibrated = true;
        calibration = newCalib;
        
        DBG("Calibration complete:");
        DBG("  Neutral accel: " << calibration.accelX << ", " << calibration.accelY << ", " << calibration.accelZ);
        DBG("  Accel noise threshold: " << calibration.accelNoiseThreshold);
        DBG("  Gyro noise threshold: " << calibration.gyroNoiseThreshold);
    }
    
    // Add sensor data
    void addSensorData(const IMUData& data)
    {
        if (isCalibrating)
        {
            calibrationBuffer.push_back(data);
            
            // Auto-stop calibration after 2 seconds
            if ((juce::Time::getCurrentTime() - calibrationStartTime).inSeconds() > 2.0)
            {
                stopCalibration();
            }
        }
        
        // Always add to main buffer
        buffer.push_back(data);
        if (buffer.size() > BUFFER_SIZE)
            buffer.pop_front();
    }
    
    // Detect gesture
    DetectionResult detect()
    {
        DetectionResult result;
        
        if (!calibration.isCalibrated || buffer.size() < MIN_SAMPLES)
            return result;
        
        // Get recent samples for analysis
        std::vector<IMUData> recent(buffer.end() - WINDOW_SIZE, buffer.end());
        
        // Calculate features relative to calibration
        float maxAccelMag = 0;
        float maxGyroMag = 0;
        float avgGyroX = 0, avgGyroY = 0, avgGyroZ = 0;
        
        for (const auto& sample : recent)
        {
            // Remove gravity/neutral position
            float ax = sample.accelX - calibration.accelX;
            float ay = sample.accelY - calibration.accelY;
            float az = sample.accelZ - calibration.accelZ;
            float accelMag = std::sqrt(ax*ax + ay*ay + az*az);
            maxAccelMag = std::max(maxAccelMag, accelMag);
            
            // Gyro relative to neutral
            float gx = sample.gyroX - calibration.gyroX;
            float gy = sample.gyroY - calibration.gyroY;
            float gz = sample.gyroZ - calibration.gyroZ;
            float gyroMag = std::sqrt(gx*gx + gy*gy + gz*gz);
            maxGyroMag = std::max(maxGyroMag, gyroMag);
            
            avgGyroX += gx / WINDOW_SIZE;
            avgGyroY += gy / WINDOW_SIZE;
            avgGyroZ += gz / WINDOW_SIZE;
        }
        
        // Detect gesture type
        bool significantAccel = maxAccelMag > calibration.accelNoiseThreshold;
        bool significantGyro = maxGyroMag > calibration.gyroNoiseThreshold;
        
        if (significantAccel && maxAccelMag > maxGyroMag * 0.1f)
        {
            // TAP detected (acceleration dominant)
            result.type = GestureType::TAP;
            
            // Determine tap strength
            if (maxAccelMag > calibration.tapThresholdHard)
                result.tapStrength = TapStrength::HARD;
            else if (maxAccelMag > calibration.tapThresholdSoft)
                result.tapStrength = TapStrength::MEDIUM;
            else
                result.tapStrength = TapStrength::SOFT;
                
            result.confidence = std::min(1.0f, maxAccelMag / calibration.tapThresholdHard);
            result.valid = true;
            
            // Clear buffer after tap detection
            buffer.clear();
        }
        else if (significantGyro && maxGyroMag > calibration.strokeGyroThreshold)
        {
            // STROKE detected (rotation dominant)
            result.type = GestureType::STROKE;
            
            // Determine stroke direction based on dominant gyro axis
            float absGyroX = std::abs(avgGyroX);
            float absGyroY = std::abs(avgGyroY);
            float absGyroZ = std::abs(avgGyroZ);
            
            if (absGyroX > absGyroY && absGyroX > absGyroZ)
            {
                // Rotation around X-axis = up/down stroke
                result.strokeDirection = (avgGyroX > 0) ? StrokeDirection::DOWN : StrokeDirection::UP;
            }
            else if (absGyroY > absGyroX && absGyroY > absGyroZ)
            {
                // Rotation around Y-axis = left/right tilt
                result.strokeDirection = (avgGyroY > 0) ? StrokeDirection::RIGHT : StrokeDirection::LEFT;
            }
            else
            {
                // Rotation around Z-axis = left/right stroke
                result.strokeDirection = (avgGyroZ > 0) ? StrokeDirection::LEFT : StrokeDirection::RIGHT;
            }
            
            result.confidence = std::min(1.0f, maxGyroMag / (calibration.strokeGyroThreshold * 2));
            result.valid = true;
            
            // Clear buffer after stroke detection
            buffer.clear();
        }
        else
        {
            result.type = GestureType::NONE;
            result.confidence = 0.0f;
            result.valid = false;
        }
        
        return result;
    }
    
    // Check if calibrated
    bool isCalibrated() const { return calibration.isCalibrated; }
    
    // Get calibration data
    const CalibrationData& getCalibration() const { return calibration; }
    
    // Update thresholds
    void setTapSoftThreshold(float threshold)
    {
        if (threshold > 0) calibration.tapThresholdSoft = threshold;
    }
    
    void setTapHardThreshold(float threshold)
    {
        if (threshold > 0) calibration.tapThresholdHard = threshold;
    }
    
    void setStrokeThreshold(float threshold)
    {
        if (threshold > 0) calibration.strokeGyroThreshold = threshold;
    }
    
    // Reset calibration
    void resetCalibration()
    {
        calibration = CalibrationData();
        calibrationBuffer.clear();
        isCalibrating = false;
    }
    
    // Clear buffer
    void clearBuffer() { buffer.clear(); }
    
private:
    static constexpr size_t BUFFER_SIZE = 200;
    static constexpr size_t WINDOW_SIZE = 50;
    static constexpr size_t MIN_SAMPLES = 20;
    
    std::deque<IMUData> buffer;
    std::deque<IMUData> calibrationBuffer;
    CalibrationData calibration;
    
    bool isCalibrating = false;
    juce::Time calibrationStartTime;
};
