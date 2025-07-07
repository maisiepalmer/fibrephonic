/*
  ==============================================================================

    MIDIDataHandler.h
    Created: 4 Jul 2025 4:22:09pm
    Author:  Joseph B

    For the exporting of global MIDI messages from gesture handler class. 

  ==============================================================================
*/

#pragma once

#include  "GestureManager.h"

using namespace juce;
using namespace std;

class MIDIHandler : protected GestureManager
{
private:

    unique_ptr<MidiOutput> midiOut;
    GestureManager* GesturManagerInstance = nullptr;
    //GestureManager::datastreams* Data;

public:

    MIDIHandler(shared_ptr<BluetoothConnectionManager> BluetoothConnectionManager);
    ~MIDIHandler();

    // Initialisation
    bool openDeviceByIndex(int index);                         
    bool openDeviceByName(const String& deviceName);      
    void closeDevice();

    // Utility (Not entirely necessary but kept anyway)
    StringArray getAvailableDeviceNames() const;          
    int getNumAvailableDevices() const;

    // MIDI Send
    void sendNoteOn(int channel, int note, int velocity);       
    void sendNoteOff(int channel, int note);                    
    void sendCC(int channel, int controller, int value);        
    void sendRawMessage(const juce::MidiMessage& msg);

private:

    void getDataFromManager(GestureManager::datastreams* DataStreamsPointer);
};