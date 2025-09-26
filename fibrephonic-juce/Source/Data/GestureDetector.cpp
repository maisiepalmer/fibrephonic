#include "GestureDetector.h"

GestureDetector::GestureDetector(size_t bufferSize)
    : maxBuffer(bufferSize) {}

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

void GestureDetector::startCalibration()
{
    calibrating = true;
    calibMags.clear();
    calib.calibrated = false;
}

void GestureDetector::stopCalibration()
{
    calibrating = false;
    if (!calibMags.empty())
    {
        float m = mean(calibMags);
        float s = stddev(calibMags, m);
        calib.baselineMean = m;
        calib.baselineStd  = s;
        calib.calibrated   = true;
    }
}

void GestureDetector::resetCalibration()
{
    calib = Calibration{};
    calibMags.clear();
    calibrating = false;
}

void GestureDetector::pushSample(const IMUData& sample)
{
    buffer.push_back(sample);
    if (buffer.size() > maxBuffer)
        buffer.pop_front();

    if (calibrating)
        calibMags.push_back(magnitude(sample));
}

Gestures::GestureType GestureDetector::detect()
{
    if (buffer.empty() || !calib.calibrated)
        return Gestures::NONE;

    if (cooldown > 0)
    {
        cooldown--;
        return Gestures::NONE;
    }

    // check latest sample magnitude
    float mag = magnitude(buffer.back());
    float delta = mag - calib.baselineMean;

    // Tap detection
    if (delta > calib.tapThreshold * calib.baselineStd)
    {
        cooldown = 15;
        return Gestures::TAP;
    }

    // Stroke detection over last N samples
    const size_t N = 30;
    if (buffer.size() >= N)
    {
        float sumX=0, sumY=0;
        auto it = buffer.end()-N;
        for (; it!=buffer.end(); ++it)
        {
            sumX += it->accelX;
            sumY += it->accelY;
        }
        if (std::fabs(sumX) > calib.strokeThreshold ||
            std::fabs(sumY) > calib.strokeThreshold)
        {
            cooldown = 25;
            if (std::fabs(sumX) > std::fabs(sumY))
                return (sumX > 0 ? Gestures::STROKE_RIGHT : Gestures::STROKE_LEFT);
            else
                return (sumY > 0 ? Gestures::STROKE_UP : Gestures::STROKE_DOWN);
        }
    }

    return Gestures::NONE;
}

std::string GestureDetector::getGestureName(Gestures::GestureType g)
{
    switch (g)
    {
        case Gestures::TAP:             return "Tap";
        case Gestures::STROKE_UP:       return "Stroke Up";
        case Gestures::STROKE_DOWN:     return "Stroke Down";
        case Gestures::STROKE_LEFT:     return "Stroke Left";
        case Gestures::STROKE_RIGHT:    return "Stroke Right";
        default:                        return "None";
    }
}
