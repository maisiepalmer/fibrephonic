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

        //Incoming
        double gX, gY, gZ;
        double accX, accY, accZ;
        double jerkX, jerkY, jerkZ;

        // Data Vectors
        std::vector<double> accXData, accYData, accZData,
            XData, YData, ZData;
    };

private:

    void timerCallback() override;
    void PollGestures();

    void getConnectionManagerValues();

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

    void perform1DWaveletTransform(std::vector<double>& xaccdata,
                                   std::vector<double>& yaccdata,
                                   std::vector<double>& zaccdata);

private:

    datastreams DATA;
};