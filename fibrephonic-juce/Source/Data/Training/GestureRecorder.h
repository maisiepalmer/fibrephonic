/*
  ==============================================================================

    GestureRecorder.h
    Created: 23 Sep 2025 5:56:49pm
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../../Helpers.h"
#include "../TextileGestureDetector.h"

struct FeatureVector
{
    std::vector<float> values;
    std::string label;
};

class CSVLogger
{
    public:
    CSVLogger(const juce::File& file) : csvFile(file)
    {
        if (!csvFile.existsAsFile())
            csvFile.create();
    }
    
    void logFeature(const FeatureVector& fv)
    {
        juce::StringArray row;
        
        for (auto v : fv.values)
            row.add(juce::String(v, 6)); // 6 decimal places
        
        row.add(fv.label);
        juce::String line = row.joinIntoString(",");
        
        // Append to file
        juce::FileOutputStream stream(csvFile, true); // append mode
        if (stream.openedOk())
        {
            stream.writeText(line + "\n", false, false, "\n");
        }
    }
    
    private:
    juce::File csvFile;
};

class GestureRecorder  : public juce::Component,
                         private juce::Timer
{
public:
    GestureRecorder(TextileGestureDetector& detectorRef, CSVLogger& loggerRef)
        : detector(detectorRef), logger(loggerRef)
    {
        addAndMakeVisible(startButton);
        startButton.setButtonText("Record Gesture");
        startButton.onClick = [this]() { startRecording("tap_soft"); }; // pick your label
    }

    void startRecording(const std::string& labelToUse)
    {
        label = labelToUse;
        countdown = 3;           // 3…2…1
        recordingActive = false; // not recording yet
        startTimer(1000);        // tick every 1 second
        DBG("Starting countdown...");
    }

private:
    void timerCallback() override
    {
        if (countdown > 0)
        {
            DBG("Countdown: " << countdown);
            countdown--;
        }
        else if (!recordingActive)
        {
            DBG("GO! Perform gesture now.");
            recordingActive = true;
            countdown = gestureDuration; // number of seconds to record
        }
        else if (countdown > 0)
        {
            countdown--;
        }
        else
        {
            stopTimer();
            recordingActive = false;
            saveWindow();
        }
    }

    void saveWindow()
    {
        size_t windowSize = 200;
        auto fv = extractWindowFeatures(detector.getBuffer(), windowSize, label);
        logger.logFeature(fv);

        DBG("Saved gesture: " << label);
    }
    
    FeatureVector extractWindowFeatures(const std::deque<IMUData>& buffer,
                                        size_t windowSize,
                                        const std::string& label)
    {
        FeatureVector fv;
        fv.label = label;

        if (buffer.size() < windowSize)
            return fv;

        // Collect last windowSize samples
        std::vector<float> ax, ay, az, gx, gy, gz, mx, my, mz;
        for (size_t i = buffer.size() - windowSize; i < buffer.size(); ++i)
        {
            const auto& d = buffer[i];
            ax.push_back(d.accelX);
            ay.push_back(d.accelY);
            az.push_back(d.accelZ);
            gx.push_back(d.gyroX);
            gy.push_back(d.gyroY);
            gz.push_back(d.gyroZ);
            mx.push_back(d.magX);
            my.push_back(d.magY);
            mz.push_back(d.magZ);
        }

        addFeatures(fv.values, ax);
        addFeatures(fv.values, ay);
        addFeatures(fv.values, az);

        addFeatures(fv.values, gx);
        addFeatures(fv.values, gy);
        addFeatures(fv.values, gz);

        addFeatures(fv.values, mx);
        addFeatures(fv.values, my);
        addFeatures(fv.values, mz);

        return fv;
    }
    
    void addFeatures(std::vector<float>& dest, const std::vector<float>& data)
    {
        if (data.empty())
        {
            dest.push_back(0.0f); // mean
            dest.push_back(0.0f); // variance
            dest.push_back(0.0f); // energy
            return;
        }
        
        float mean = std::accumulate(data.begin(), data.end(), 0.0f) / data.size();
        
        float variance = 0.0f;
        for (auto v : data)
            variance += (v - mean) * (v - mean);
        variance /= data.size();
        
        float energy = 0.0f;
        for (auto v : data)
            energy += v * v;
        
        dest.push_back(mean);
        dest.push_back(variance);
        dest.push_back(energy);
    }

    // References to your existing objects
    TextileGestureDetector& detector;
    CSVLogger& logger;

    juce::TextButton startButton { "Record" };

    // State
    std::string label;
    int countdown = 0;
    bool recordingActive = false;
    const int gestureDuration = 2; // seconds to capture gesture window
};
