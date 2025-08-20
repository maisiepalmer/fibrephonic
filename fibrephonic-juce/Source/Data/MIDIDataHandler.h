/*
  ==============================================================================

    MIDIDataHandler.h
    Created: 4 Jul 2025 4:22:09pm
    Author:  Joseph B

    For the exporting of global MIDI messages from gesture handler class. 

  ==============================================================================
*/

#pragma once

#include "GestureManager.h"

using namespace juce;
using namespace std;

#define MAXNO_MIDICHANNELS 16
#define MAXNO_MIDIVAL 127

class MIDIHandler : public Thread 
{
public:
    MIDIHandler(shared_ptr<GestureManager> gestureManagerInstance);
    ~MIDIHandler() override;

    void run() override;
    void stop();
    
    // Utility (Not entirely necessary but kept anyway)
    StringArray getAvailableDeviceNames() const;
    int getNumAvailableDevices() const;
    
    // More Getters
    int BPM;
    void getBPMSliderVal(int val) { BPM = val; }
    
    
private:
    unique_ptr<MidiOutput> midiOut;
    shared_ptr<GestureManager> gestureManager;
    GestureManager::datastreams* Data;
    GestureManager::Gesture* GESTURES;

    bool midiOutFlag, quantise;
    int numChannels, note, velocity, ccVal;

    vector<int> X, Y, Z;

    // Initialisation
    bool openDeviceByIndex(int index);                         
    bool openDeviceByName(const String& deviceName);      
    void closeDevice();

    // MIDI Send
    void sendNoteOn(int channel, int note, int velocity);       
    void sendNoteOff(int channel, int note);                    
    void sendCC(int channel, int CCVal, int CCParamVal);        
    void sendRawMessage(const juce::MidiMessage& msg);
    void getGestureManagerData();

    // Functionality 
    void MIDIgestureHandling(vector<int>& X, vector<int>& Y, vector<int>& Z, int& Note, int& Velocity, int& CCVal);
    void MIDIOUT();
};
