/*
  ==============================================================================

    SimpleGestureDetector.h
    Created: 18 Sep 2025 3:13:59pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>

class SimpleGestureDetector
{
public:
    enum GestureType
    {
        NO_GESTURE,
        TAP,           // Sharp downward acceleration spike
        DOUBLE_TAP,    // Two taps in quick succession
        SHAKE,         // Rapid back-and-forth movement
        TILT_LEFT,     // Sustained left tilt
        TILT_RIGHT,    // Sustained right tilt
        TILT_FORWARD,  // Sustained forward tilt
        TILT_BACKWARD, // Sustained backward tilt
        CIRCLE_CW,     // Clockwise circular motion
        CIRCLE_CCW,    // Counter-clockwise circular motion
        SWIPE_LEFT,    // Quick left swipe
        SWIPE_RIGHT,   // Quick right swipe
        SWIPE_UP,      // Quick upward swipe
        SWIPE_DOWN     // Quick downward swipe
    };

    struct IMUData
    {
        float accelX, accelY, accelZ;
        float gyroX, gyroY, gyroZ;
        float magX, magY, magZ;
        
        IMUData() : accelX(0), accelY(0), accelZ(0), gyroX(0), gyroY(0), gyroZ(0), magX(0), magY(0), magZ(0) {}
        IMUData(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
            : accelX(ax), accelY(ay), accelZ(az), gyroX(gx), gyroY(gy), gyroZ(gz), magX(mx), magY(my), magZ(mz) {}
    };

private:
    // Configuration
    static constexpr size_t BUFFER_SIZE = 50;      // ~0.5 seconds at 100Hz
    static constexpr size_t GESTURE_WINDOW = 20;   // Window for gesture analysis
    
    // Thresholds
    static constexpr float TAP_THRESHOLD = 15.0f;           // Acceleration spike for tap
    static constexpr float SHAKE_THRESHOLD = 10.0f;         // Acceleration variance for shake
    static constexpr float TILT_THRESHOLD = 5.0f;           // Sustained acceleration for tilt
    static constexpr float SWIPE_THRESHOLD = 8.0f;          // Acceleration for swipe detection
    static constexpr float CIRCULAR_THRESHOLD = 300.0f;     // Gyroscope rotation for circles
    
    // Data buffers
    std::deque<IMUData> dataBuffer;
    std::vector<float> accelMagnitudes;
    
    // State tracking
    GestureType lastGesture = NO_GESTURE;
    int gestureTimeout = 0;
    int tapCount = 0;
    int64_t lastTapTime = 0;
    
    // Helper functions
    float calculateMagnitude(float x, float y, float z) const;
    float calculateVariance(const std::vector<float>& data) const;
    float calculateMean(const std::vector<float>& data) const;
    bool isAccelerationSpike(const std::vector<float>& magnitudes, size_t index, float threshold) const;
    GestureType detectTiltDirection(const std::vector<IMUData>& window) const;
    GestureType detectSwipeDirection(const std::vector<IMUData>& window) const;
    GestureType detectCircularMotion(const std::vector<IMUData>& window) const;
    int64_t getCurrentTimeMs() const;

public:
    SimpleGestureDetector() {}
    
    // Main processing function - call this with each new IMU reading
    GestureType processIMUData(const IMUData& newData);
    
    // Get readable gesture name
    std::string getGestureName(GestureType gesture) const;
};
