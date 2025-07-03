/*
  ==============================================================================

    GestureManager.h
    Created: 13 Jun 2025 3:33:06pm
    Author:  Joseph B 

  ==============================================================================
*/

#pragma once

#define WAVE_EXPORT

#define IMUINERTIALREFRESH 400
#define DATAWINDOW 256 // Needs to be power of 2

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "../Wavelib/wavelet2s.h"

class BluetoothConnectionManager; // Forward Declare Bluetooth Manager Class, Avoids Circular Dependancy. 

class GestureManager : private juce::Timer
{
public:

    GestureManager(std::shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance);
    ~GestureManager();

    void startPolling();
    void stopPolling();

private:

    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;

    int pollcount = 0;

    enum Gesture {
        PITCH,
        ROLL,
        YAW,
        TAP,
        STROKE
    };

    struct datastreams {
        // Incoming sensor data
        double gX, gY, gZ;
        double accX, accY, accZ;
        double jerkX, jerkY, jerkZ;

        // Raw Data Vectors (time domain data)
        std::vector<double> accXData, accYData, accZData;
        std::vector<double> XData, YData, ZData;

        // Wavelet decomposition coefficients and bookkeeping for X axis
        std::vector<double> xCoeff;         // full wavelet coeffs vector (approx + details)
        std::vector<double> xApprox;        // approximation coefficients extracted
        std::vector<double> xDetail;        // detail coefficients extracted
        std::vector<double> xBookkeeping;   // bookkeeping info (e.g., levels, zero padding)
        std::vector<double> xLengths;       // lengths of coefficient segments

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
    };


private:

    void timerCallback() override;
    void PollGestures();

    void getConnectionManagerValues();

    // Fetches Sensor data from bluetooth manager class
    void fillDataVectors(std::vector<double>* xaccdata,
                         std::vector<double>* yaccdata,
                         std::vector<double>* zaccdata,
                         std::vector<double>* xdata,
                         std::vector<double>* ydata,
                         std::vector<double>* zdata,
                                          double* x,
                                          double* y,
                                          double* z,
                                          double* accx,
                                          double* accy,
                                          double* accz);

    // Performs DWT transforming signal into wavelet domain
    void decomposeAxis(std::vector<double>& input, std::string wavelet, int levels,

                       std::vector<double>& coeffs,
                       std::vector<double>& approx,
                       std::vector<double>& detail,
                       std::vector<double>& bookkeeping,
                       std::vector<double>& lengths);

    // Reconstructs singnal back to time domain via IDWT
    void reconstructAxis(std::vector<double>& coeffs,
                         std::vector<double>& approx,
                         std::vector<double>& detail,
                         std::vector<double>& bookkeeping,
                         std::vector<double>& lengths,
                         std::string wavelet,
                         std::vector<double>& reconstructed);

    // Performs all necessary transforms on all axis allows for signal editing and mapping
    void perform1DWaveletTransform();

private:

    datastreams DATA;
};