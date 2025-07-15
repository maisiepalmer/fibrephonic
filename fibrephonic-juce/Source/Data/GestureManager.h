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

#define IMUINERTIALREFRESH 400
#define DATAWINDOW 256 // Needs to be power of 2

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <algorithm>
#include "../Wavelib/wavelet2s.h"

using namespace std;

class BluetoothConnectionManager; // Forward Declare Bluetooth Manager Class, Avoids Circular Dependancy. 

class GestureManager : private juce::Timer
{
public:

    GestureManager(shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance);
    ~GestureManager();

    void startPolling();
    void stopPolling();

private:

    shared_ptr<BluetoothConnectionManager> bluetoothConnection;

    int pollcount = 0;

public:

    enum Gesture {
        NO_GESTURE,
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
        vector<double> accXData, accYData, accZData;
        vector<double> XData, YData, ZData;

        // Wavelet decomposition coefficients and bookkeeping for X axis
        vector<double> xCoeff;         // full wavelet coeffs vector (approx + details)
        vector<double> xApprox;        // approximation coefficients extracted
        vector<double> xDetail;        // detail coefficients extracted
        vector<double> xBookkeeping;   // bookkeeping info (e.g., levels, zero padding)
        vector<double> xLengths;       // lengths of coefficient segments

        // Wavelet decomposition coefficients and bookkeeping for Y axis
        vector<double> yCoeff;
        vector<double> yApprox;
        vector<double> yDetail;
        vector<double> yBookkeeping;
        vector<double> yLengths;

        // Wavelet decomposition coefficients and bookkeeping for Z axis
        vector<double> zCoeff;
        vector<double> zApprox;
        vector<double> zDetail;
        vector<double> zBookkeeping;
        vector<double> zLengths;

        // Scaled Data (post transform)
        vector<double> xScaled;
        vector<double> yScaled;
        vector<double> zScaled;
    };


private:

    void timerCallback() override;
    void PollGestures(); // Main running function, to be called in main component

    void getConnectionManagerValues();

    // Fetches Sensor data from bluetooth manager class
    void fillDataVectors(vector<double>* xaccdata,
                         vector<double>* yaccdata,
                         vector<double>* zaccdata,
                         vector<double>* xdata,
                         vector<double>* ydata,
                         vector<double>* zdata,
                                          double* x,
                                          double* y,
                                          double* z,
                                          double* accx,
                                          double* accy,
                                          double* accz);

    // Performs DWT transforming signal into wavelet domain
    void decomposeAxis(vector<double>& input, string wavelet, int levels,

                       vector<double>& coeffs,
                       vector<double>& approx,
                       vector<double>& detail,
                       vector<double>& bookkeeping,
                       vector<double>& lengths);

    // Passes results of axis DWT providing point of modification and gestural identification
    void ModifyWaveletDomain(vector<double>& XApprox, vector<double> XDetail,
                             vector<double>& YApprox, vector<double> YDetail,
                             vector<double>& ZApprox, vector<double> ZDetail);

    // Reconstructs singnal back to time domain via IDWT
    void reconstructAxis(vector<double>& coeffs,
                         vector<double>& approx,
                         vector<double>& detail,
                         vector<double>& bookkeeping,
                         vector<double>& lengths,
                         string wavelet,
                         vector<double>& reconstructed);

    // Performs all necessary transforms on all axis allows for signal editing and mapping
    void perform1DWaveletTransform();

    // Takes in Vectors after transforming, calculates min/ max and scales in range 1 to 127 for MIDI CC.
    void scaleandCopy(vector<double>& xaccdata, vector<double>& yaccdata, vector<double>& zaccdata,
                      vector<double>& xscale,   vector<double>& yscale,   vector<double>& zscale);

    // Normalises data to range and scales
    vector<double> normaliseData(double min, double max, vector<double>& input);

public:

    inline vector<double>& getScaledX() { return DATA.xScaled; }
    inline vector<double>& getScaledY() { return DATA.yScaled; }
    inline vector<double>& getScaledZ() { return DATA.zScaled; }

public:

    datastreams DATA;
    datastreams* pDATA = &DATA;

    Gesture gesture;
    Gesture* pGestures = &gesture;
};