#pragma once
#include <JuceHeader.h>
#include "../../Helpers.h"
#include <vector>
#include <deque>
#include <map>

class FastGestureClassifier
{
public:
    struct Result
    {
        Gestures::GestureType gesture = Gestures::NO_GESTURE;
        float confidence = 0.0f;
        bool valid = false;
        
        std::string toString() const {
            return Gestures::getGestureName(gesture) + " (" + std::string(confidence * 100, 1) + "%)";
        }
    };
    
    FastGestureClassifier();
    bool initialise();
    void addSensorData(const IMUData& data);
    Result classify();
    bool isReady() const { return ready; }
    void clearBuffer() { buffer.clear(); }
    
private:
    bool ready = false;
    std::deque<IMUData> buffer;
    
    // Model data
    static constexpr int N_FEATURES = 27;
    static constexpr int N_CLASSES = 7;
    static constexpr int N_TREES = 100;
    
    std::vector<juce::String> classes;
    std::vector<float> scalerMean;
    std::vector<float> scalerScale;
    
    // Feature extraction
    std::vector<float> extractFeatures();
    void addAxisFeatures(std::vector<float>& features, const std::vector<float>& data);
    
    // Simple prediction using feature thresholds
    Result predictGesture(const std::vector<float>& features);
};
