/*
  ==============================================================================

    SimpleGestureDetector.cpp
    Created: 18 Sep 2025 3:13:59pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#include "SimpleGestureDetector.h"
#include <chrono>

float SimpleGestureDetector::calculateMagnitude(float x, float y, float z) const
{
    return std::sqrt(x*x + y*y + z*z);
}

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

bool SimpleGestureDetector::isAccelerationSpike(const std::vector<float>& magnitudes, size_t index, float threshold) const
{
    if (index == 0 || index >= magnitudes.size() - 1) return false;
    
    float current = magnitudes[index];
    float prev = magnitudes[index - 1];
    float next = magnitudes[index + 1];
    
    // Check if current value is significantly higher than neighbors
    return (current > threshold && current > prev * 1.5f && current > next * 1.5f);
}

SimpleGestureDetector::GestureType SimpleGestureDetector::detectTiltDirection(const std::vector<IMUData>& window) const
{
    // Calculate average acceleration over the window
    float avgX = 0, avgY = 0, avgZ = 0;
    
    for (const auto& data : window)
    {
        avgX += data.accelX;
        avgY += data.accelY;
        avgZ += data.accelZ;
    }
    
    avgX /= window.size();
    avgY /= window.size();
    avgZ /= window.size();
    
    // Determine dominant tilt direction
    if (std::abs(avgX) > TILT_THRESHOLD)
    {
        return (avgX > 0) ? TILT_RIGHT : TILT_LEFT;
    }
    else if (std::abs(avgY) > TILT_THRESHOLD)
    {
        return (avgY > 0) ? TILT_FORWARD : TILT_BACKWARD;
    }
    
    return NO_GESTURE;
}

SimpleGestureDetector::GestureType SimpleGestureDetector::detectSwipeDirection(const std::vector<IMUData>& window) const
{
    if (window.size() < 3) return NO_GESTURE;
    
    // Look for rapid acceleration followed by deceleration
    float maxAccelX = 0, maxAccelY = 0, maxAccelZ = 0;
    
    for (size_t i = 1; i < window.size() - 1; ++i)
    {
        float accelX = std::abs(window[i].accelX);
        float accelY = std::abs(window[i].accelY);
        float accelZ = std::abs(window[i].accelZ);
        
        if (accelX > maxAccelX) maxAccelX = accelX;
        if (accelY > maxAccelY) maxAccelY = accelY;
        if (accelZ > maxAccelZ) maxAccelZ = accelZ;
    }
    
    // Determine swipe direction based on dominant axis
    if (maxAccelX > SWIPE_THRESHOLD && maxAccelX > maxAccelY && maxAccelX > maxAccelZ)
    {
        // Determine if left or right based on sign
        float totalX = 0;
        for (const auto& data : window) totalX += data.accelX;
        return (totalX > 0) ? SWIPE_RIGHT : SWIPE_LEFT;
    }
    else if (maxAccelZ > SWIPE_THRESHOLD && maxAccelZ > maxAccelX && maxAccelZ > maxAccelY)
    {
        float totalZ = 0;
        for (const auto& data : window) totalZ += data.accelZ;
        return (totalZ > 0) ? SWIPE_UP : SWIPE_DOWN;
    }
    
    return NO_GESTURE;
}

SimpleGestureDetector::GestureType SimpleGestureDetector::detectCircularMotion(const std::vector<IMUData>& window) const
{
    if (window.size() < 10) return NO_GESTURE;
    
    // Sum up rotation rates to detect sustained circular motion
    float totalRotationZ = 0;
    
    for (const auto& data : window)
    {
        totalRotationZ += data.gyroZ;
    }
    
    float avgRotation = totalRotationZ / window.size();
    
    if (std::abs(avgRotation) > CIRCULAR_THRESHOLD)
    {
        return (avgRotation > 0) ? CIRCLE_CW : CIRCLE_CCW;
    }
    
    return NO_GESTURE;
}

int64_t SimpleGestureDetector::getCurrentTimeMs() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

SimpleGestureDetector::GestureType SimpleGestureDetector::processIMUData(const IMUData& newData)
{
    // Add new data to buffer
    dataBuffer.push_back(newData);
    if (dataBuffer.size() > BUFFER_SIZE)
    {
        dataBuffer.pop_front();
    }
    
    // Calculate acceleration magnitude for this sample
    float magnitude = calculateMagnitude(newData.accelX, newData.accelY, newData.accelZ);
    accelMagnitudes.push_back(magnitude);
    if (accelMagnitudes.size() > BUFFER_SIZE)
    {
        accelMagnitudes.erase(accelMagnitudes.begin());
    }
    
    // Need minimum data to start detection
    if (dataBuffer.size() < GESTURE_WINDOW)
    {
        return NO_GESTURE;
    }
    
    // Decrement gesture timeout
    if (gestureTimeout > 0)
    {
        gestureTimeout--;
        return lastGesture; // Continue returning the same gesture during timeout
    }
    
    // Get recent data window for analysis
    std::vector<IMUData> window(dataBuffer.end() - GESTURE_WINDOW, dataBuffer.end());
    std::vector<float> magWindow(accelMagnitudes.end() - GESTURE_WINDOW, accelMagnitudes.end());
    
    GestureType detectedGesture = NO_GESTURE;
    
    // TAP (sharp acceleration spike)
    for (size_t i = 1; i < magWindow.size() - 1; ++i)
    {
        if (isAccelerationSpike(magWindow, i, TAP_THRESHOLD))
        {
            int64_t currentTime = getCurrentTimeMs();
            
            // Check for double tap (within 500ms of previous tap)
            if (currentTime - lastTapTime < 500)
            {
                tapCount++;
                if (tapCount >= 2)
                {
                    detectedGesture = DOUBLE_TAP;
                    tapCount = 0;
                }
            }
            else
            {
                tapCount = 1;
                detectedGesture = TAP;
            }
            
            lastTapTime = currentTime;
            break;
        }
    }
    
    // SHAKE (high variance in acceleration)
    if (detectedGesture == NO_GESTURE)
    {
        float variance = calculateVariance(magWindow);
        if (variance > SHAKE_THRESHOLD)
        {
            detectedGesture = SHAKE;
        }
    }
    
    // CIRCULAR MOTION (sustained rotation)
    if (detectedGesture == NO_GESTURE)
    {
        detectedGesture = detectCircularMotion(window);
    }
    
    // SWIPE (rapid directional acceleration)
    if (detectedGesture == NO_GESTURE)
    {
        detectedGesture = detectSwipeDirection(window);
    }
    
    // TILT (sustained directional acceleration)
    if (detectedGesture == NO_GESTURE)
    {
        detectedGesture = detectTiltDirection(window);
    }
    
    // Set timeout to prevent rapid re-triggering of the same gesture
    if (detectedGesture != NO_GESTURE)
    {
        gestureTimeout = 20; // ~200ms timeout at 100Hz
        lastGesture = detectedGesture;
    }
    
    return detectedGesture;
}

std::string SimpleGestureDetector::getGestureName(GestureType gesture) const
{
    switch (gesture)
    {
        case NO_GESTURE:     return "None";
        case TAP:            return "Tap";
        case DOUBLE_TAP:     return "Double Tap";
        case SHAKE:          return "Shake";
        case TILT_LEFT:      return "Tilt Left";
        case TILT_RIGHT:     return "Tilt Right";
        case TILT_FORWARD:   return "Tilt Forward";
        case TILT_BACKWARD:  return "Tilt Backward";
        case CIRCLE_CW:      return "Circle Clockwise";
        case CIRCLE_CCW:     return "Circle Counter-Clockwise";
        case SWIPE_LEFT:     return "Swipe Left";
        case SWIPE_RIGHT:    return "Swipe Right";
        case SWIPE_UP:       return "Swipe Up";
        case SWIPE_DOWN:     return "Swipe Down";
        default:             return "Unknown";
    }
}
