/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B

    Gesture Manager class to identify, scale, handle and export IMU sensor data
    over various connection types.

  ==============================================================================
*/

#pragma once

#define WAVE_EXPORT

#define IMUINERTIALREFRESH 400
#define DATAWINDOW 256 // Needs to be power of 2

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include "../Wavelib/wavelet2s.h"

// Forward Declare Manager Class, Avoids Circular Dependancy.
class ConnectionManager;

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

    struct DataStreams { // Renamed from datastreams to follow PascalCase for structs
        // Incoming sensor data
        double gx, gy, gz; // Renamed to gx, gy, gz to conform to camelCase
        double accX, accY, accZ;
        double jerkX, jerkY, jerkZ;

        // Raw Data Vectors (time domain data)
        std::vector<double> accXData, accYData, accZData;
        std::vector<double> xData, yData, zData; // Renamed to xData, yData, zData

        // Wavelet decomposition coefficients and bookkeeping for X axis
        std::vector<double> xCoeff;
        std::vector<double> xApprox;
        std::vector<double> xDetail;
        std::vector<double> xBookkeeping;
        std::vector<double> xLengths;

        // Wavelet decomposition coefficients and bookkeeping for Y axis
        std::vector<double> yCoeff;
        std::vector<double> yApprox;
        std::vector<double> yDetail;
        std::vector<double> yBookkeeping;
        std::vector<double> yLengths;

        // Wavelet decomposition coefficients and bookkeeping for Z axis
        std::vector<double> zCoeff;
        std::vector<double> zApprox;
        std::vector<double> zDetail;
        std::vector<double> zBookkeeping;
        std::vector<double> zLengths;

        // Scaled Data (post transform)
        std::vector<double> xScaled;
        std::vector<double> yScaled;
        std::vector<double> zScaled;
    };
    
    std::vector<double>& getScaledX() { return data.xScaled; }
    std::vector<double>& getScaledY() { return data.yScaled; }
    std::vector<double>& getScaledZ() { return data.zScaled; }

    DataStreams data; // Renamed from DATA to data
    Gesture gesture;

private:
    std::shared_ptr<ConnectionManager> connectionManager;
    
    int pollCount = 0; // Renamed pollcount to pollCount
    
    juce::OSCSender oscSender;

    void timerCallback() override;
    void pollGestures();

    void getConnectionManagerValues();

    void fillDataVectors(std::vector<double>& xaccdata,
                         std::vector<double>& yaccdata,
                         std::vector<double>& zaccdata,
                         std::vector<double>& xdata,
                         std::vector<double>& ydata,
                         std::vector<double>& zdata,
                         double& x,
                         double& y,
                         double& z,
                         double& accx,
                         double& accy,
                         double& accz);

    void decomposeAxis(std::vector<double>& input,
                       std::string wavelet,
                       int levels,
                       std::vector<double>& coeffs,
                       std::vector<double>& approx,
                       std::vector<double>& detail,
                       std::vector<double>& bookkeeping,
                       std::vector<double>& lengths);

    void modifyWaveletDomain(std::vector<double>& xApprox, const std::vector<double>& xDetail,
                             std::vector<double>& yApprox, const std::vector<double>& yDetail,
                             std::vector<double>& zApprox, const std::vector<double>& zDetail); // Renamed to modifyWaveletDomain

    Gesture identifyGesture(const std::vector<double>& xApprox, const std::vector<double>& xDetail,
                            const std::vector<double>& yApprox, const std::vector<double>& yDetail,
                            const std::vector<double>& zApprox, const std::vector<double>& zDetail); // Renamed to identifyGesture

    void reconstructAxis(std::vector<double>& coeffs,
                         std::vector<double>& approx,
                         std::vector<double>& detail,
                         std::vector<double>& bookkeeping,
                         std::vector<double>& lengths,
                         std::string wavelet,
                         std::vector<double>& reconstructed);

    void softThresholding(std::vector<double>& detailCoeffs); // Renamed to softThresholding

    void perform1DWaveletTransform();

    void scaleAndCopy(std::vector<double>& xaccdata, std::vector<double>& yaccdata, std::vector<double>& zaccdata,
                      std::vector<double>& xscale, std::vector<double>& yscale, std::vector<double>& zscale); // Renamed to scaleAndCopy

    std::vector<double> normaliseData(double min, double max, const std::vector<double>& input); // Renamed to normaliseData
    
    void sendProcessedDataAsBundle(const std::vector<double>& accelXData,
                                   const std::vector<double>& accelYData,
                                   const std::vector<double>& accelZData);
};
