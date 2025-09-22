/**
 * @file SimpleGestureDetector.cpp
 * @brief Choreographic gesture detection for textile-based IMU sensors
 * @author Maisie Palmer
 * @date Created: 18 Sep 2025
 *
 * Designed for detecting expressive movements in performance contexts
 * where the IMU is embedded in fabric/costume.
 */

#include "SimpleGestureDetector.h"
#include <chrono>

/**
 * @brief Calculate 3D vector magnitude
 * @param x X component
 * @param y Y component
 * @param z Z component
 * @return Magnitude of the vector
 */
float SimpleGestureDetector::calculateMagnitude(float x, float y, float z) const
{
    return std::sqrt(x*x + y*y + z*z);
}

/**
 * @brief Calculate variance of a data series
 * @param data Vector of float values
 * @return Statistical variance
 */
float SimpleGestureDetector::calculateVariance(const std::vector<float>& data) const
{
    if (data.size() < 2) return 0.0f;
    
    float mean = calculateMean(data);
    float variance = 0.0f;
    
    for (float value : data)
    {
        variance += (value - mean) * (value - mean);
    }
    
    return variance / (data.size() - 1);
}

/**
 * @brief Calculate mean of a data series
 * @param data Vector of float values
 * @return Arithmetic mean
 */
float SimpleGestureDetector::calculateMean(const std::vector<float>& data) const
{
    if (data.empty()) return 0.0f;
    
    float sum = 0.0f;
    for (float value : data)
    {
        sum += value;
    }
    
    return sum / data.size();
}

/**
 * @brief Detect sudden impact/pat on fabric
 * @param window Recent IMU data
 * @return true if pat detected
 */
bool SimpleGestureDetector::detectPat(const std::vector<IMUData>& window) const
{
    // A pat on fabric creates a sharp but smaller acceleration spike
    // followed by quick damping (fabric absorbs energy)
    if (window.size() < 5) return false;
    
    float maxAccel = 0;
    int maxIndex = -1;
    
    for (size_t i = 0; i < window.size(); ++i)
    {
        float mag = calculateMagnitude(window[i].accelX, window[i].accelY, window[i].accelZ);
        if (mag > maxAccel)
        {
            maxAccel = mag;
            maxIndex = i;
        }
    }
    
    // Check for sharp rise and quick decay (characteristic of fabric impact)
    if (maxAccel > PAT_THRESHOLD && maxIndex > 0 && maxIndex < window.size() - 2)
    {
        float beforeMag = calculateMagnitude(window[maxIndex-1].accelX,
                                            window[maxIndex-1].accelY,
                                            window[maxIndex-1].accelZ);
        float afterMag = calculateMagnitude(window[maxIndex+2].accelX,
                                           window[maxIndex+2].accelY,
                                           window[maxIndex+2].accelZ);
        
        // Sharp rise and fall pattern
        if (maxAccel > beforeMag * 2.0f && maxAccel > afterMag * 2.0f)
        {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Detect smooth wave-like motion
 * @param window Recent IMU data
 * @return Wave direction if detected, NO_GESTURE otherwise
 */
SimpleGestureDetector::GestureType SimpleGestureDetector::detectWave(const std::vector<IMUData>& window) const
{
    // Wave motion in fabric creates sinusoidal acceleration patterns
    if (window.size() < 15) return NO_GESTURE;
    
    // Look for oscillating patterns in gyroscope data
    float gyroXSum = 0, gyroYSum = 0, gyroZSum = 0;
    int directionChangesX = 0, directionChangesY = 0;
    
    for (size_t i = 1; i < window.size(); ++i)
    {
        gyroXSum += std::abs(window[i].gyroX);
        gyroYSum += std::abs(window[i].gyroY);
        gyroZSum += std::abs(window[i].gyroZ);
        
        // Count direction changes (oscillation)
        if (i > 1)
        {
            bool prevPosX = window[i-1].gyroX > 0;
            bool currPosX = window[i].gyroX > 0;
            if (prevPosX != currPosX && std::abs(window[i].gyroX) > 50) directionChangesX++;
            
            bool prevPosY = window[i-1].gyroY > 0;
            bool currPosY = window[i].gyroY > 0;
            if (prevPosY != currPosY && std::abs(window[i].gyroY) > 50) directionChangesY++;
        }
    }
    
    // Detect wave based on oscillating motion with sufficient amplitude
    if (directionChangesX >= 2 && gyroXSum > WAVE_THRESHOLD)
    {
        return WAVE_HORIZONTAL;
    }
    else if (directionChangesY >= 2 && gyroYSum > WAVE_THRESHOLD)
    {
        return WAVE_VERTICAL;
    }
    
    return NO_GESTURE;
}

/**
 * @brief Detect spinning motion (pirouette-like)
 * @param window Recent IMU data
 * @return Spin direction if detected
 */
SimpleGestureDetector::GestureType SimpleGestureDetector::detectSpin(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return NO_GESTURE;
    
    // Spinning creates sustained rotation in one axis
    float totalRotZ = 0;
    float minRotZ = 1000, maxRotZ = -1000;
    
    for (const auto& data : window)
    {
        totalRotZ += data.gyroZ;
        if (data.gyroZ < minRotZ) minRotZ = data.gyroZ;
        if (data.gyroZ > maxRotZ) maxRotZ = data.gyroZ;
    }
    
    float avgRotZ = totalRotZ / window.size();
    
    // Check for consistent spinning (not just a quick turn)
    bool consistentSpin = (minRotZ > 0 && avgRotZ > SPIN_THRESHOLD) ||
                          (maxRotZ < 0 && avgRotZ < -SPIN_THRESHOLD);
    
    if (consistentSpin && std::abs(avgRotZ) > SPIN_THRESHOLD)
    {
        return (avgRotZ > 0) ? SPIN_RIGHT : SPIN_LEFT;
    }
    
    return NO_GESTURE;
}

/**
 * @brief Detect stretching motion in fabric
 * @param window Recent IMU data
 * @return true if stretch detected
 */
bool SimpleGestureDetector::detectStretch(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return false;
    
    // Stretching fabric creates gradual, sustained acceleration change
    float initialAccel = calculateMagnitude(window[0].accelX, window[0].accelY, window[0].accelZ);
    float finalAccel = calculateMagnitude(window.back().accelX, window.back().accelY, window.back().accelZ);
    
    // Check for gradual increase in acceleration (fabric under tension)
    float deltaAccel = finalAccel - initialAccel;
    
    // Also check gyroscope for minimal rotation (stretch is linear)
    float totalRot = 0;
    for (const auto& data : window)
    {
        totalRot += calculateMagnitude(data.gyroX, data.gyroY, data.gyroZ);
    }
    float avgRot = totalRot / window.size();
    
    // Stretch detected: gradual accel change with minimal rotation
    return (std::abs(deltaAccel) > STRETCH_THRESHOLD && avgRot < 100);
}

/**
 * @brief Detect flutter/shake motion in fabric
 * @param window Recent IMU data
 * @return true if flutter detected
 */
bool SimpleGestureDetector::detectFlutter(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return false;
    
    // Flutter creates rapid, small oscillations
    std::vector<float> accelMags;
    for (const auto& data : window)
    {
        accelMags.push_back(calculateMagnitude(data.accelX, data.accelY, data.accelZ));
    }
    
    // High variance with moderate amplitude indicates flutter
    float variance = calculateVariance(accelMags);
    float mean = calculateMean(accelMags);
    
    // Flutter has high variance relative to mean
    return (variance > FLUTTER_THRESHOLD && mean < 20.0f);
}

/**
 * @brief Detect held pose (stillness)
 * @param window Recent IMU data
 * @return true if holding still
 */
bool SimpleGestureDetector::detectHold(const std::vector<IMUData>& window) const
{
    if (window.size() < 20) return false;
    
    // Calculate variance in all axes
    std::vector<float> accelX, accelY, accelZ;
    std::vector<float> gyroX, gyroY, gyroZ;
    
    for (const auto& data : window)
    {
        accelX.push_back(data.accelX);
        accelY.push_back(data.accelY);
        accelZ.push_back(data.accelZ);
        gyroX.push_back(data.gyroX);
        gyroY.push_back(data.gyroY);
        gyroZ.push_back(data.gyroZ);
    }
    
    // Very low variance in all sensors indicates holding still
    float accelVar = calculateVariance(accelX) + calculateVariance(accelY) + calculateVariance(accelZ);
    float gyroVar = calculateVariance(gyroX) + calculateVariance(gyroY) + calculateVariance(gyroZ);
    
    return (accelVar < HOLD_THRESHOLD && gyroVar < 1000);
}

/**
 * @brief Get current system time in milliseconds
 * @return Current time in ms since epoch
 */
int64_t SimpleGestureDetector::getCurrentTimeMs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

/**
 * @brief Main processing function for new IMU data
 * @param newData Latest IMU readings
 * @return Detected gesture type
 */
SimpleGestureDetector::GestureType SimpleGestureDetector::processIMUData(const IMUData& newData)
{
    // Add new data to buffer
    dataBuffer.push_back(newData);
    if (dataBuffer.size() > BUFFER_SIZE)
    {
        dataBuffer.pop_front();
    }
    
    // Need minimum data to start detection
    if (dataBuffer.size() < GESTURE_WINDOW)
    {
        return NO_GESTURE;
    }
    
    // Handle gesture cooldown
    if (gestureCooldown > 0)
    {
        gestureCooldown--;
        // Don't return last gesture during cooldown, just return NO_GESTURE
        // This prevents gesture "sticking"
        return NO_GESTURE;
    }
    
    // Get analysis windows
    std::vector<IMUData> shortWindow(dataBuffer.end() - 10, dataBuffer.end());
    std::vector<IMUData> mediumWindow(dataBuffer.end() - GESTURE_WINDOW, dataBuffer.end());
    
    GestureType detectedGesture = NO_GESTURE;
    
    // Priority order for choreographic gestures:
    // 1. PAT - immediate tactile feedback
    if (detectPat(shortWindow))
    {
        detectedGesture = PAT;
        gestureCooldown = 15; // Short cooldown for repeated pats
    }
    // 2. FLUTTER - rapid movement
    else if (detectFlutter(shortWindow))
    {
        detectedGesture = FLUTTER;
        gestureCooldown = 20;
    }
    // 3. STRETCH - tension in fabric
    else if (detectStretch(mediumWindow))
    {
        detectedGesture = STRETCH;
        gestureCooldown = 30;
    }
    // 4. WAVE - flowing motion
    else
    {
        GestureType waveType = detectWave(mediumWindow);
        if (waveType != NO_GESTURE)
        {
            detectedGesture = waveType;
            gestureCooldown = 25;
        }
    }
    
    // 5. SPIN - rotation
    if (detectedGesture == NO_GESTURE)
    {
        GestureType spinType = detectSpin(mediumWindow);
        if (spinType != NO_GESTURE)
        {
            detectedGesture = spinType;
            gestureCooldown = 40;
        }
    }
    
    // 6. HOLD - stillness (lowest priority)
    if (detectedGesture == NO_GESTURE && detectHold(mediumWindow))
    {
        // Only trigger HOLD if we weren't already in HOLD state
        if (lastGesture != HOLD)
        {
            detectedGesture = HOLD;
            gestureCooldown = 50; // Long cooldown for hold
        }
    }
    
    // Update last gesture
    if (detectedGesture != NO_GESTURE)
    {
        lastGesture = detectedGesture;
    }
    
    return detectedGesture;
}

/**
 * @brief Get human-readable name for gesture
 * @param gesture Gesture type enum
 * @return String description of the gesture
 */
std::string SimpleGestureDetector::getGestureName(GestureType gesture) const
{
    switch (gesture)
    {
        case NO_GESTURE:        return "None";
        case PAT:              return "Pat";
        case WAVE_HORIZONTAL:  return "Wave Horizontal";
        case WAVE_VERTICAL:    return "Wave Vertical";
        case SPIN_LEFT:        return "Spin Left";
        case SPIN_RIGHT:       return "Spin Right";
        case STRETCH:          return "Stretch";
        case FLUTTER:          return "Flutter";
        case HOLD:             return "Hold";
        
        // Legacy gestures (keeping for compatibility but not actively detected)
        case TAP:              return "Tap";
        case DOUBLE_TAP:       return "Double Tap";
        case SHAKE:            return "Shake";
        case TILT_LEFT:        return "Tilt Left";
        case TILT_RIGHT:       return "Tilt Right";
        case TILT_FORWARD:     return "Tilt Forward";
        case TILT_BACKWARD:    return "Tilt Backward";
        case CIRCLE_CW:        return "Circle CW";
        case CIRCLE_CCW:       return "Circle CCW";
        case SWIPE_LEFT:       return "Swipe Left";
        case SWIPE_RIGHT:      return "Swipe Right";
        case SWIPE_UP:         return "Swipe Up";
        case SWIPE_DOWN:       return "Swipe Down";
        default:               return "Unknown";
    }
}
