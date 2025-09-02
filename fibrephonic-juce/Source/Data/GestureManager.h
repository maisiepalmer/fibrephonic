/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B

    Gesture Manager class to identify, scale, handle and export IMU sensor data
    over various connection types.

  ==============================================================================
*/

#pragma once

#define WAVE_EXPORT

#define IMUINERTIALREFRESH 100  // Reduced from 400 to prevent overload
#define DATAWINDOW 256 // Needs to be power of 2

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include "../Wavelib/wavelet2s.h"

// Forward Declare Manager Class, Avoids Circular Dependancy.
class ConnectionManager;

// Circular Buffer class for efficient data management
class CircularBuffer {
private:
    std::vector<double> buffer;
    size_t head = 0;
    size_t capacity;
    bool filled = false;
    
public:
    CircularBuffer(size_t size) : capacity(size) {
        buffer.resize(size, 0.0);
    }
    
    void push(double value) {
        buffer[head] = value;
        head = (head + 1) % capacity;
        if (head == 0) filled = true;
    }
    
    std::vector<double> getData() const {
        if (!filled) {
            // If buffer not full yet, return from beginning to head
            return std::vector<double>(buffer.begin(), buffer.begin() + head);
        } else {
            // If buffer is full, return properly ordered data
            std::vector<double> result;
            result.reserve(capacity);
            
            // Add data from head to end
            result.insert(result.end(), buffer.begin() + head, buffer.end());
            // Add data from beginning to head
            result.insert(result.end(), buffer.begin(), buffer.begin() + head);
            
            return result;
        }
    }
    
    size_t size() const {
        return filled ? capacity : head;
    }
    
    void clear() {
        head = 0;
        filled = false;
        std::fill(buffer.begin(), buffer.end(), 0.0);
    }
};

class GestureManager : private juce::Timer
{
public:
    GestureManager();
    ~GestureManager();
    
    void setConnectionManager(std::shared_ptr<ConnectionManager> connectionManagerInstance) { connectionManager = connectionManagerInstance; };

    void startPolling();
    void stopPolling();
    
    enum Gesture {
        NO_GESTURE,
        PITCH,
        ROLL,
        YAW,
        TAP,
        STROKE
    };

    struct DataStreams {
        // Incoming raw sensor data from ConnectionManager
        double gx_raw, gy_raw, gz_raw;
        double accX_raw, accY_raw, accZ_raw;
        double jerkX, jerkY, jerkZ;

        // Circular buffers for efficient data management
        CircularBuffer accelXBuffer, accelYBuffer, accelZBuffer;
        CircularBuffer gyroXBuffer, gyroYBuffer, gyroZBuffer;

        // Raw Data Vectors (time domain data) - populated from circular buffers
        std::vector<double> accelXData, accelYData, accelZData;
        std::vector<double> gyroXData, gyroYData, gyroZData;

        // Wavelet decomposition coefficients and bookkeeping for Accelerometer X axis
        std::vector<double> accelXCoeff;
        std::vector<double> accelXApprox;
        std::vector<double> accelXDetail;
        std::vector<double> accelXBookkeeping;
        std::vector<double> accelXLengths;

        // Wavelet decomposition coefficients and bookkeeping for Accelerometer Y axis
        std::vector<double> accelYCoeff;
        std::vector<double> accelYApprox;
        std::vector<double> accelYDetail;
        std::vector<double> accelYBookkeeping;
        std::vector<double> accelYLengths;

        // Wavelet decomposition coefficients and bookkeeping for Accelerometer Z axis
        std::vector<double> accelZCoeff;
        std::vector<double> accelZApprox;
        std::vector<double> accelZDetail;
        std::vector<double> accelZBookkeeping;
        std::vector<double> accelZLengths;

        // Scaled Data (post wavelet transform filtering)
        std::vector<double> accelXScaled;
        std::vector<double> accelYScaled;
        std::vector<double> accelZScaled;
        
        // Scaled Gyroscope Data (Raw Data, not filtered)
        std::vector<double> gyroXScaled;
        std::vector<double> gyroYScaled;
        std::vector<double> gyroZScaled;
        
        // Constructor to initialize circular buffers
        DataStreams() : accelXBuffer(DATAWINDOW), accelYBuffer(DATAWINDOW), accelZBuffer(DATAWINDOW),
                       gyroXBuffer(DATAWINDOW), gyroYBuffer(DATAWINDOW), gyroZBuffer(DATAWINDOW) {}
    };
    
    std::vector<double>& getScaledAccelX() { return data.accelXScaled; }
    std::vector<double>& getScaledAccelY() { return data.accelYScaled; }
    std::vector<double>& getScaledAccelZ() { return data.accelZScaled; }

    DataStreams data;
    Gesture gesture;

private:
    std::weak_ptr<ConnectionManager> connectionManager;
    
    int pollCount = 0;
    int oscReconnectAttempts = 0;
    static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
    
    juce::OSCSender oscSender;
    juce::String oscHost = "192.169.1.2";
    int oscPort = 5006;
    bool oscConnected = false;

    void timerCallback() override;
    void pollGestures();

    bool ensureOSCConnection();
    void getConnectionManagerValues();

    void fillDataBuffers(double gyroX_in, double gyroY_in, double gyroZ_in,
                        double accelX_in, double accelY_in, double accelZ_in);

    void decomposeAxis(std::vector<double>& input,
                       std::string wavelet,
                       int levels,
                       std::vector<double>& coeffs,
                       std::vector<double>& approx,
                       std::vector<double>& detail,
                       std::vector<double>& bookkeeping,
                       std::vector<double>& lengths);

    void modifyWaveletDomain(std::vector<double>& accelXApprox, const std::vector<double>& accelXDetail,
                             std::vector<double>& accelYApprox, const std::vector<double>& accelYDetail,
                             std::vector<double>& accelZApprox, const std::vector<double>& accelZDetail);

    Gesture identifyGesture(const std::vector<double>& accelXApprox, const std::vector<double>& accelXDetail,
                            const std::vector<double>& accelYApprox, const std::vector<double>& accelYDetail,
                            const std::vector<double>& accelZApprox, const std::vector<double>& accelZDetail);

    void reconstructAxis(std::vector<double>& coeffs,
                         std::vector<double>& approx,
                         std::vector<double>& detail,
                         std::vector<double>& bookkeeping,
                         std::vector<double>& lengths,
                         std::string wavelet,
                         std::vector<double>& reconstructed);

    void softThresholding(std::vector<double>& detailCoeffs);

    void perform1DWaveletTransform();

    void scaleAndCopy(const std::vector<double>& input, std::vector<double>& output);

    std::vector<double> normaliseData(double min, double max, const std::vector<double>& input);
    
    void sendProcessedDataAsBundle(const std::vector<double>& accelXData,
                                   const std::vector<double>& accelYData,
                                   const std::vector<double>& accelZData,
                                   const std::vector<double>& gyroXData,
                                   const std::vector<double>& gyroYData,
                                   const std::vector<double>& gyroZData);
};
