#pragma once
#include <deque>
#include <vector>
#include <cmath>
#include <numeric>
#include <string>
#include "../Helpers.h"

class GestureDetector
{
public:
    struct Calibration
    {
        float baselineMean = 0.0f;
        float baselineStd  = 1.0f;
        float tapThreshold = 3.0f;     ///< how many std above baseline counts as a tap
        float strokeThreshold = 30.0f; ///< accel sum threshold for stroke
        bool calibrated = false;
    };

    GestureDetector(size_t bufferSize = 200);

    void pushSample(const IMUData& sample);
    Gestures::GestureType detect();

    void startCalibration();
    void stopCalibration();
    void resetCalibration();

    bool isCalibrated() const { return calib.calibrated; }
    Calibration getCalibration() const { return calib; }

    void setTapThreshold(float v)     { calib.tapThreshold = v; }
    void setStrokeThreshold(float v)  { calib.strokeThreshold = v; }

    static std::string getGestureName(Gestures::GestureType g);

private:
    std::deque<IMUData> buffer;
    size_t maxBuffer;
    Calibration calib;

    std::vector<float> calibMags;
    bool calibrating = false;

    int cooldown = 0;

    float magnitude(const IMUData& d) const;
    float mean(const std::vector<float>& v) const;
    float stddev(const std::vector<float>& v, float m) const;
};
