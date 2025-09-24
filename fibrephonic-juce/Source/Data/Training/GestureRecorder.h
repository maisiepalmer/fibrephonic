// GestureRecorder.h - Fixed Implementation
#pragma once
#include <JuceHeader.h>
#include "../../Helpers.h"
#include "../TextileGestureDetector.h"
#include <numeric>

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
        {
            csvFile.create();
            writeHeader();
        }
    }
    
    void writeHeader()
    {
        // Write CSV header with all feature names
        juce::StringArray header;
        
        // 9 sensors x 3 features each = 27 features
        std::vector<std::string> sensors = {"ax", "ay", "az", "gx", "gy", "gz", "mx", "my", "mz"};
        std::vector<std::string> features = {"mean", "variance", "energy"};
        
        for (const auto& sensor : sensors)
        {
            for (const auto& feature : features)
            {
                header.add(juce::String(sensor + "_" + feature));
            }
        }
        header.add("label");
        
        juce::String headerLine = header.joinIntoString(",");
        juce::FileOutputStream stream(csvFile, false); // overwrite mode for header
        if (stream.openedOk())
        {
            stream.writeText(headerLine + "\n", false, false, "\n");
        }
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

class GestureRecorder : public juce::Component, private juce::Timer
{
public:
    GestureRecorder(TextileGestureDetector& detectorRef, CSVLogger& loggerRef)
        : detector(detectorRef), logger(loggerRef)
    {
        setupUI();
    }

private:
    void setupUI()
    {
        // Gesture selection dropdown
        addAndMakeVisible(gestureComboBox);
        gestureComboBox.addItem("Tap Soft", 1);
        gestureComboBox.addItem("Tap Hard", 2);
        gestureComboBox.addItem("Stroke Up", 3);
        gestureComboBox.addItem("Stroke Down", 4);
        gestureComboBox.addItem("Stroke Left", 5);
        gestureComboBox.addItem("Stroke Right", 6);
        gestureComboBox.addItem("No Gesture", 7);
        gestureComboBox.setSelectedId(1);
        
        // Record button
        addAndMakeVisible(recordButton);
        recordButton.setButtonText("Record Gesture");
        recordButton.onClick = [this]() { startRecording(); };
        
        // Status labels
        addAndMakeVisible(statusLabel);
        statusLabel.setText("Ready to record", juce::dontSendNotification);
        statusLabel.setFont(juce::FontOptions(16.0f));
        
        addAndMakeVisible(countdownLabel);
        countdownLabel.setText("", juce::dontSendNotification);
        countdownLabel.setFont(juce::FontOptions(24.0f));
        countdownLabel.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(samplesLabel);
        samplesLabel.setText("Samples recorded: 0", juce::dontSendNotification);
        samplesLabel.setFont(juce::FontOptions(14.0f));
        
        setSize(400, 300);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);
        
        gestureComboBox.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(10);
        
        recordButton.setBounds(bounds.removeFromTop(40));
        bounds.removeFromTop(20);
        
        statusLabel.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(10);
        
        countdownLabel.setBounds(bounds.removeFromTop(50));
        bounds.removeFromTop(10);
        
        samplesLabel.setBounds(bounds.removeFromTop(30));
    }

    void startRecording()
    {
        // Get selected gesture label
        int selectedId = gestureComboBox.getSelectedId();
        switch(selectedId)
        {
            case 1: currentLabel = "tap_soft"; break;
            case 2: currentLabel = "tap_hard"; break;
            case 3: currentLabel = "stroke_up"; break;
            case 4: currentLabel = "stroke_down"; break;
            case 5: currentLabel = "stroke_left"; break;
            case 6: currentLabel = "stroke_right"; break;
            case 7: currentLabel = "no_gesture"; break;
            default: currentLabel = "unknown"; break;
        }
        
        countdown = 3;           // 3…2…1
        recordingActive = false; // not recording yet
        recordButton.setEnabled(false);
        gestureComboBox.setEnabled(false);
        
        statusLabel.setText("Get ready...", juce::dontSendNotification);
        startTimer(1000);        // tick every 1 second
        
        DBG("Starting countdown for gesture: " << currentLabel);
    }

    void timerCallback() override
    {
        if (countdown > 0 && !recordingActive)
        {
            countdownLabel.setText(juce::String(countdown), juce::dontSendNotification);
            countdownLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
            DBG("Countdown: " << countdown);
            countdown--;
        }
        else if (!recordingActive)
        {
            countdownLabel.setText("GO!", juce::dontSendNotification);
            countdownLabel.setColour(juce::Label::textColourId, juce::Colours::green);
            statusLabel.setText("Recording " + juce::String(currentLabel) + "...", juce::dontSendNotification);
            
            DBG("GO! Perform gesture now: " << currentLabel);
            recordingActive = true;
            countdown = recordingDuration; // number of seconds to record
        }
        else if (countdown > 0)
        {
            countdownLabel.setText(juce::String(countdown), juce::dontSendNotification);
            countdown--;
        }
        else
        {
            stopRecording();
        }
    }
    
    void stopRecording()
    {
        stopTimer();
        recordingActive = false;
        
        countdownLabel.setText("", juce::dontSendNotification);
        statusLabel.setText("Processing...", juce::dontSendNotification);
        
        // Save the recorded window
        saveWindow();
        
        // Reset UI
        recordButton.setEnabled(true);
        gestureComboBox.setEnabled(true);
        statusLabel.setText("Ready to record", juce::dontSendNotification);
        
        // Update sample count
        samplesRecorded++;
        samplesLabel.setText("Samples recorded: " + juce::String(samplesRecorded),
                           juce::dontSendNotification);
        
        DBG("Recording complete. Sample saved.");
    }

    void saveWindow()
    {
        // Use a larger window size to capture the full gesture
        size_t windowSize = std::min(static_cast<size_t>(200), detector.getBuffer().size());
        
        if (windowSize < 10) // Need minimum samples
        {
            DBG("Not enough samples in buffer: " << windowSize);
            return;
        }
        
        auto fv = extractWindowFeatures(detector.getBuffer(), windowSize, currentLabel);
        
        if (!fv.values.empty())
        {
            logger.logFeature(fv);
            DBG("Saved gesture: " << currentLabel << " with " << fv.values.size() << " features");
        }
        else
        {
            DBG("Failed to extract features");
        }
    }
    
    FeatureVector extractWindowFeatures(const std::deque<IMUData>& buffer,
                                        size_t windowSize,
                                        const std::string& label)
    {
        FeatureVector fv;
        fv.label = label;

        if (buffer.size() < windowSize)
        {
            DBG("Buffer too small: " << buffer.size() << " < " << windowSize);
            return fv;
        }

        // Collect last windowSize samples
        std::vector<float> ax, ay, az, gx, gy, gz, mx, my, mz;
        
        size_t startIndex = buffer.size() - windowSize;
        for (size_t i = startIndex; i < buffer.size(); ++i)
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

        // Extract features for each sensor axis
        addFeatures(fv.values, ax);
        addFeatures(fv.values, ay);
        addFeatures(fv.values, az);
        addFeatures(fv.values, gx);
        addFeatures(fv.values, gy);
        addFeatures(fv.values, gz);
        addFeatures(fv.values, mx);
        addFeatures(fv.values, my);
        addFeatures(fv.values, mz);

        DBG("Extracted " << fv.values.size() << " features from " << windowSize << " samples");
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
        
        // Mean
        float mean = std::accumulate(data.begin(), data.end(), 0.0f) / data.size();
        
        // Variance
        float variance = 0.0f;
        for (auto v : data)
            variance += (v - mean) * (v - mean);
        variance /= data.size();
        
        // Energy (sum of squares)
        float energy = 0.0f;
        for (auto v : data)
            energy += v * v;
        
        dest.push_back(mean);
        dest.push_back(variance);
        dest.push_back(energy);
    }

    // References
    TextileGestureDetector& detector;
    CSVLogger& logger;

    // UI Components
    juce::ComboBox gestureComboBox;
    juce::TextButton recordButton;
    juce::Label statusLabel;
    juce::Label countdownLabel;
    juce::Label samplesLabel;

    // Recording state
    std::string currentLabel;
    int countdown = 0;
    bool recordingActive = false;
    const int recordingDuration = 3; // seconds to capture gesture
    int samplesRecorded = 0;
};
