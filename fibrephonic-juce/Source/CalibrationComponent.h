/**
 * @file CalibrationComponent.h
 * @brief UI component for gesture detector calibration
 * @author Maisie Palmer
 * @date 24 Sep 2025 11:19:12am
 */

#pragma once
#include <JuceHeader.h>
#include "Data/GestureDetector.h"

class CalibrationComponent : public juce::Component,
                             private juce::Timer
{
public:
    CalibrationComponent(GestureDetector& detectorRef)
        : detector(detectorRef)
    {
        setupUI();
    }
    
    ~CalibrationComponent()
    {
        stopTimer();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey.withAlpha(0.1f));
        
        // Draw border
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 1.0f);
        
        // Draw calibration status indicator
        auto statusBounds = getLocalBounds().removeFromTop(40).reduced(10, 5);
        
        if (detector.isCalibrated())
        {
            g.setColour(juce::Colours::green.withAlpha(0.3f));
            g.fillRoundedRectangle(statusBounds.toFloat(), 3.0f);
        }
        else if (isCalibrating)
        {
            // Pulsing animation during calibration
            float alpha = 0.3f + 0.2f * std::sin(animationPhase);
            g.setColour(juce::Colours::orange.withAlpha(alpha));
            g.fillRoundedRectangle(statusBounds.toFloat(), 3.0f);
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        
        // Title
        titleLabel.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(5);
        
        // Status
        statusLabel.setBounds(bounds.removeFromTop(25));
        bounds.removeFromTop(10);
        
        // Calibrate button
        calibrateButton.setBounds(bounds.removeFromTop(35));
        bounds.removeFromTop(5);
        
        // Reset button
        resetButton.setBounds(bounds.removeFromTop(30));
        bounds.removeFromTop(10);
        
        // Instructions
        instructionsLabel.setBounds(bounds.removeFromTop(60));
        bounds.removeFromTop(10);
        
        // Threshold controls
        auto thresholdArea = bounds.removeFromTop(120);
        
        tapSoftLabel.setBounds(thresholdArea.removeFromTop(20));
        tapSoftSlider.setBounds(thresholdArea.removeFromTop(25));
        
        tapHardLabel.setBounds(thresholdArea.removeFromTop(20));
        tapHardSlider.setBounds(thresholdArea.removeFromTop(25));
        
        strokeThresholdLabel.setBounds(thresholdArea.removeFromTop(20));
        strokeThresholdSlider.setBounds(thresholdArea.removeFromTop(25));
    }
    
private:
    void setupUI()
    {
        // Title
        addAndMakeVisible(titleLabel);
        titleLabel.setText("Gesture Calibration", juce::dontSendNotification);
        titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        titleLabel.setJustificationType(juce::Justification::centred);
        
        // Status
        addAndMakeVisible(statusLabel);
        updateStatusLabel();
        statusLabel.setFont(juce::FontOptions(14.0f));
        statusLabel.setJustificationType(juce::Justification::centred);
        
        // Calibrate button
        addAndMakeVisible(calibrateButton);
        calibrateButton.setButtonText("Start Calibration");
        calibrateButton.onClick = [this]() { startCalibration(); };
        
        // Reset button
        addAndMakeVisible(resetButton);
        resetButton.setButtonText("Reset Calibration");
        resetButton.setEnabled(false);
        resetButton.onClick = [this]() { resetCalibration(); };
        
        // Instructions
        addAndMakeVisible(instructionsLabel);
        instructionsLabel.setText("Hold the sensor in neutral position and click 'Start Calibration'. "
                                 "Keep still for 2 seconds.",
                                 juce::dontSendNotification);
        instructionsLabel.setFont(juce::FontOptions(12.0f));
        instructionsLabel.setJustificationType(juce::Justification::centredLeft);
        instructionsLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
        
        // Threshold sliders
        setupThresholdSlider(tapSoftSlider, tapSoftLabel, "Tap Soft Threshold", 0.5, 5.0, 2.0);
        setupThresholdSlider(tapHardSlider, tapHardLabel, "Tap Hard Threshold", 2.0, 10.0, 5.0);
        setupThresholdSlider(strokeThresholdSlider, strokeThresholdLabel, "Stroke Threshold", 10.0, 100.0, 30.0);
        
        // Set initial size
        setSize(300, 400);
    }
    
    void setupThresholdSlider(juce::Slider& slider, juce::Label& label,
                             const juce::String& text, double min, double max, double defaultVal)
    {
        addAndMakeVisible(label);
        label.setText(text + ": " + juce::String(defaultVal, 1), juce::dontSendNotification);
        label.setFont(juce::FontOptions(11.0f));
        
        addAndMakeVisible(slider);
        slider.setRange(min, max, 0.1);
        slider.setValue(defaultVal);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.onValueChange = [this, &slider, &label, text]()
        {
            label.setText(text + ": " + juce::String(slider.getValue(), 1),
                         juce::dontSendNotification);
            updateThresholds();
        };
        
        // Initially disabled until calibrated
        slider.setEnabled(false);
    }
    
    void startCalibration()
    {
        isCalibrating = true;
        calibrationProgress = 0.0f;
        
        detector.startCalibration();
        calibrateButton.setEnabled(false);
        calibrateButton.setButtonText("Calibrating...");
        statusLabel.setText("Hold still...", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
        
        // Start animation timer
        startTimerHz(30);
        
        // Schedule calibration completion
        juce::Timer::callAfterDelay(2000, [this]()
        {
            completeCalibration();
        });
    }
    
    void completeCalibration()
    {
        detector.stopCalibration();
        isCalibrating = false;
        
        stopTimer();
        
        if (detector.isCalibrated())
        {
            calibrateButton.setButtonText("Recalibrate");
            calibrateButton.setEnabled(true);
            resetButton.setEnabled(true);
            
            // Enable threshold sliders
            tapSoftSlider.setEnabled(true);
            tapHardSlider.setEnabled(true);
            strokeThresholdSlider.setEnabled(true);
            
            statusLabel.setText("Calibration Complete!", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
            
            // Update thresholds with current calibration values
            auto calib = detector.getCalibration();
            tapSoftSlider.setValue(calib.tapThresholdSoft);
            tapHardSlider.setValue(calib.tapThresholdHard);
            strokeThresholdSlider.setValue(calib.strokeGyroThreshold);
        }
        else
        {
            calibrateButton.setButtonText("Start Calibration");
            calibrateButton.setEnabled(true);
            statusLabel.setText("Calibration Failed - Try Again", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
        }
        
        repaint();
    }
    
    void resetCalibration()
    {
        detector.resetCalibration();
        
        calibrateButton.setButtonText("Start Calibration");
        calibrateButton.setEnabled(true);
        resetButton.setEnabled(false);
        
        // Disable threshold sliders
        tapSoftSlider.setEnabled(false);
        tapHardSlider.setEnabled(false);
        strokeThresholdSlider.setEnabled(false);
        
        updateStatusLabel();
        repaint();
    }
    
    void updateStatusLabel()
    {
        if (detector.isCalibrated())
        {
            statusLabel.setText("Status: Calibrated", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
        }
        else
        {
            statusLabel.setText("Status: Not Calibrated", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
        }
    }
    
    void updateThresholds()
    {
        // Update detector thresholds
        detector.setTapSoftThreshold(tapSoftSlider.getValue());
        detector.setTapHardThreshold(tapHardSlider.getValue());
        detector.setStrokeThreshold(strokeThresholdSlider.getValue());
    }
    
    void timerCallback() override
    {
        if (isCalibrating)
        {
            animationPhase += 0.1f;
            if (animationPhase > 2.0f * M_PI)
                animationPhase -= 2.0f * M_PI;
            
            calibrationProgress += 0.0167f; // ~60Hz timer, 2 seconds total
            calibrationProgress = std::min(1.0f, calibrationProgress);
            
            repaint();
        }
    }
    
    // Reference to the gesture detector
    GestureDetector& detector;
    
    // UI Components
    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::TextButton calibrateButton;
    juce::TextButton resetButton;
    juce::Label instructionsLabel;
    
    // Threshold controls
    juce::Slider tapSoftSlider;
    juce::Label tapSoftLabel;
    juce::Slider tapHardSlider;
    juce::Label tapHardLabel;
    juce::Slider strokeThresholdSlider;
    juce::Label strokeThresholdLabel;
    
    // State
    bool isCalibrating = false;
    float calibrationProgress = 0.0f;
    float animationPhase = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CalibrationComponent)
};
