/**
 * @file SimpleGestureDetector.h
 * @brief Choreographic gesture detection for textile-based IMU sensors
 * @author Maisie Palmer
 * @date Created: 18 Sep 2025
 *
 * Detects expressive gestures suitable for musical costume interaction
 * and dance performance with fabric-embedded sensors.
 */

#pragma once

#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>

/**
 * @class SimpleGestureDetector
 * @brief Detects choreographic gestures from IMU data embedded in textiles
 *
 * Optimized for detecting expressive movements in performance contexts
 * where the IMU sensor is embedded in fabric or costume materials.
 * Focuses on gestures that work well with the properties of textile materials.
 */
class SimpleGestureDetector
{
public:
    /**
     * @enum GestureType
     * @brief Types of gestures optimized for choreographic/textile applications
     */
    enum GestureType
    {
        NO_GESTURE,
        
        // Choreographic gestures for textile
        PAT,              ///< Pat or tap on fabric surface
        WAVE_HORIZONTAL,  ///< Horizontal wave motion through fabric
        WAVE_VERTICAL,    ///< Vertical wave motion through fabric
        SPIN_LEFT,        ///< Counter-clockwise spinning motion
        SPIN_RIGHT,       ///< Clockwise spinning motion
        STRETCH,          ///< Fabric stretching/tension
        FLUTTER,          ///< Rapid flutter/shake of fabric
        HOLD,             ///< Holding still/pose
        
        // Legacy gestures (kept for compatibility)
        TAP,              ///< Sharp tap
        DOUBLE_TAP,       ///< Double tap
        SHAKE,            ///< Shake motion
        TILT_LEFT,        ///< Left tilt
        TILT_RIGHT,       ///< Right tilt
        TILT_FORWARD,     ///< Forward tilt
        TILT_BACKWARD,    ///< Backward tilt
        CIRCLE_CW,        ///< Clockwise circle
        CIRCLE_CCW,       ///< Counter-clockwise circle
        SWIPE_LEFT,       ///< Left swipe
        SWIPE_RIGHT,      ///< Right swipe
        SWIPE_UP,         ///< Up swipe
        SWIPE_DOWN        ///< Down swipe
    };

    /**
     * @struct IMUData
     * @brief Container for IMU sensor readings
     */
    struct IMUData
    {
        float accelX, accelY, accelZ; ///< Accelerometer values (m/s²)
        float gyroX, gyroY, gyroZ;    ///< Gyroscope values (deg/s)
        float magX, magY, magZ;       ///< Magnetometer values (μT)
        
        /** @brief Default constructor - initializes all values to zero */
        IMUData() : accelX(0), accelY(0), accelZ(0),
                   gyroX(0), gyroY(0), gyroZ(0),
                   magX(0), magY(0), magZ(0) {}
        
        /** @brief Parameterized constructor */
        IMUData(float ax, float ay, float az,
                float gx, float gy, float gz,
                float mx, float my, float mz)
            : accelX(ax), accelY(ay), accelZ(az),
              gyroX(gx), gyroY(gy), gyroZ(gz),
              magX(mx), magY(my), magZ(mz) {}
    };

    /** @brief Default constructor */
    SimpleGestureDetector() {}
    
    /**
     * @brief Process new IMU data and detect gestures
     * @param newData Latest IMU sensor readings
     * @return Detected gesture type (or NO_GESTURE if none detected)
     */
    GestureType processIMUData(const IMUData& newData);
    
    /**
     * @brief Get human-readable name for a gesture type
     * @param gesture The gesture type to describe
     * @return String representation of the gesture
     */
    std::string getGestureName(GestureType gesture) const;

private:
    /** @name Configuration Constants
     *  @{
     */
    static constexpr size_t BUFFER_SIZE = 50;      ///< ~0.5 seconds at 100Hz
    static constexpr size_t GESTURE_WINDOW = 20;   ///< Analysis window size
    
    // Thresholds optimized for fabric/textile response
    static constexpr float PAT_THRESHOLD = 12.0f;      ///< Gentle pat on fabric
    static constexpr float WAVE_THRESHOLD = 1500.0f;   ///< Wave motion through fabric
    static constexpr float SPIN_THRESHOLD = 250.0f;    ///< Spinning/rotation threshold
    static constexpr float STRETCH_THRESHOLD = 3.0f;   ///< Fabric stretch detection
    static constexpr float FLUTTER_THRESHOLD = 8.0f;   ///< Flutter/vibration in fabric
    static constexpr float HOLD_THRESHOLD = 0.5f;      ///< Stillness detection
    /** @} */
    
    /** @name Data Buffers
     *  @{
     */
    std::deque<IMUData> dataBuffer;       ///< Rolling buffer of IMU data
    /** @} */
    
    /** @name State Tracking
     *  @{
     */
    GestureType lastGesture = NO_GESTURE; ///< Previously detected gesture
    int gestureCooldown = 0;              ///< Cooldown timer between gestures
    /** @} */
    
    /** @name Helper Functions - Math Utilities
     *  @{
     */
    float calculateMagnitude(float x, float y, float z) const;
    float calculateVariance(const std::vector<float>& data) const;
    float calculateMean(const std::vector<float>& data) const;
    int64_t getCurrentTimeMs() const;
    /** @} */
    
    /** @name Choreographic Gesture Detectors
     *  Specialized detection functions for textile-based gestures
     *  @{
     */
    bool detectPat(const std::vector<IMUData>& window) const;
    GestureType detectWave(const std::vector<IMUData>& window) const;
    GestureType detectSpin(const std::vector<IMUData>& window) const;
    bool detectStretch(const std::vector<IMUData>& window) const;
    bool detectFlutter(const std::vector<IMUData>& window) const;
    bool detectHold(const std::vector<IMUData>& window) const;
    /** @} */
};
