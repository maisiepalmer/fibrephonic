#include "FastGestureClassifier.h"
#include <numeric>
#include <algorithm>
#include <cmath>

FastGestureClassifier::FastGestureClassifier()
{
    classes = {"no_gesture", "stroke_down", "stroke_left", "stroke_right", "stroke_up", "tap_hard", "tap_soft"};
    scalerMean = {-0.033299f, 0.000967f, 4.477509f, 0.504318f, 0.002056f, 12.659040f, 0.465346f, 0.001398f, 13.013467f, -0.652776f, 225.487805f, 19184.887056f, -0.691340f, 31.560195f, 3474.205850f, -0.799760f, 63.719107f, 8433.158398f, 0.536194f, 0.000502f, 104.643063f, -0.576810f, 0.000528f, 27.548671f, 0.331796f, 0.001821f, 47.118122f};
    scalerScale = {0.383633f, 0.003931f, 5.254801f, 0.406910f, 0.012275f, 10.436134f, 0.464583f, 0.007607f, 11.269536f, 20.336724f, 1611.847264f, 137568.439743f, 9.152525f, 157.656866f, 16114.560364f, 14.722314f, 403.334370f, 49687.838173f, 1.788881f, 0.002663f, 388.911425f, 0.764886f, 0.003091f, 62.038542f, 1.207764f, 0.014189f, 152.174922f};
}

bool FastGestureClassifier::initialise()
{
    ready = true;
    DBG("Fast Gesture Classifier initialised");
    DBG("Features: " << N_FEATURES);
    DBG("Classes: " << N_CLASSES);
    return true;
}

void FastGestureClassifier::addSensorData(const IMUData& data)
{
    buffer.push_back(data);
    if (buffer.size() > 200) // Keep sliding window
        buffer.pop_front();
}

FastGestureClassifier::Result FastGestureClassifier::classify()
{
    Result result;
    
    if (!ready || buffer.size() < 20)
        return result;
    
    auto features = extractFeatures();
    if (features.size() != N_FEATURES)
        return result;
    
    // Scale features
    for (size_t i = 0; i < features.size(); ++i)
        features[i] = (features[i] - scalerMean[i]) / scalerScale[i];
    
    return predictGesture(features);
}

std::vector<float> FastGestureClassifier::extractFeatures()
{
    std::vector<float> features;
    
    if (buffer.size() < 20)
    {
        features.resize(N_FEATURES, 0.0f);
        return features;
    }
    
    // Use last 200 samples (or all if less)
    size_t windowSize = std::min(size_t(200), buffer.size());
    size_t startIdx = buffer.size() - windowSize;
    
    // Extract sensor data
    std::vector<float> ax, ay, az, gx, gy, gz, mx, my, mz;
    
    for (size_t i = startIdx; i < buffer.size(); ++i)
    {
        const auto& data = buffer[i];
        ax.push_back(data.accelX);
        ay.push_back(data.accelY);
        az.push_back(data.accelZ);
        gx.push_back(data.gyroX);
        gy.push_back(data.gyroY);
        gz.push_back(data.gyroZ);
        mx.push_back(data.magX);
        my.push_back(data.magY);
        mz.push_back(data.magZ);
    }
    
    // Extract features for each axis (mean, variance, energy)
    addAxisFeatures(features, ax);
    addAxisFeatures(features, ay);
    addAxisFeatures(features, az);
    addAxisFeatures(features, gx);
    addAxisFeatures(features, gy);
    addAxisFeatures(features, gz);
    addAxisFeatures(features, mx);
    addAxisFeatures(features, my);
    addAxisFeatures(features, mz);
    
    return features;
}

void FastGestureClassifier::addAxisFeatures(std::vector<float>& features, const std::vector<float>& data)
{
    if (data.empty())
    {
        features.insert(features.end(), {0.0f, 0.0f, 0.0f});
        return;
    }
    
    // Mean
    float mean = std::accumulate(data.begin(), data.end(), 0.0f) / data.size();
    
    // Variance  
    float variance = 0.0f;
    for (float value : data)
        variance += (value - mean) * (value - mean);
    variance /= data.size();
    
    // Energy
    float energy = 0.0f;
    for (float value : data)
        energy += value * value;
    
    features.push_back(mean);
    features.push_back(variance);
    features.push_back(energy);
}

FastGestureClassifier::Result FastGestureClassifier::predictGesture(const std::vector<float>& features)
{
    Result result;
    
    // Calculate feature magnitudes for different sensor types
    float accelMagnitude = std::sqrt(features[0]*features[0] + features[3]*features[3] + features[6]*features[6]); // accel means
    float gyroMagnitude = std::sqrt(features[9]*features[9] + features[12]*features[12] + features[15]*features[15]); // gyro means
    
    float accelVariance = features[1] + features[4] + features[7]; // accel variances
    float accelEnergy = features[2] + features[5] + features[8]; // accel energies
    
    // Simple decision tree based on your training data patterns
    if (accelEnergy > 500.0f) // High energy = tap
    {
        if (accelEnergy > 1000.0f)
        {
            result.gesture = Gestures::TAP_HARD;
            result.confidence = 0.85f;
        }
        else
        {
            result.gesture = Gestures::TAP_SOFT;
            result.confidence = 0.75f;
        }
    }
    else if (gyroMagnitude > 2.0f) // High gyro = stroke
    {
        // Use gyro direction to determine stroke direction
        float gyroX = features[9];
        float gyroY = features[12];
        
        if (std::abs(gyroX) > std::abs(gyroY))
        {
            result.gesture = (gyroX > 0) ? Gestures::STROKE_RIGHT : Gestures::STROKE_LEFT;
        }
        else
        {
            result.gesture = (gyroY > 0) ? Gestures::STROKE_UP : Gestures::STROKE_DOWN;
        }
        result.confidence = 0.70f;
    }
    else if (accelVariance > 1.0f) // Medium variance = soft gesture
    {
        result.gesture = Gestures::TAP_SOFT;
        result.confidence = 0.60f;
    }
    else
    {
        result.gesture = Gestures::NO_GESTURE;
        result.confidence = 0.90f;
    }
    
    result.valid = true;
    return result;
}
