/*
  ==============================================================================

    MIDIDataHandler.cpp
    Created: 4 Jul 2025 4:22:09pm
    Author: Joseph B

  ==============================================================================
*/

#include "MIDIDataHandler.h"

using namespace juce;

MIDIHandler::MIDIHandler(shared_ptr<BluetoothConnectionManager> BluetoothConnectionManager)
    : GestureManager(BluetoothConnectionManager)
{

}

MIDIHandler::~MIDIHandler()
{

}

bool MIDIHandler::openDeviceByIndex(int index){ return false;}
bool MIDIHandler::openDeviceByName(const String& deviceName){ return false;}
void MIDIHandler::closeDevice(){}
StringArray MIDIHandler::getAvailableDeviceNames() const{ return {}; }
int MIDIHandler::getNumAvailableDevices() const{ return 0; }
void MIDIHandler::sendNoteOn(int channel, int note, int velocity){}
void MIDIHandler::sendNoteOff(int channel, int note){}
void MIDIHandler::sendCC(int channel, int controller, int value){}
void MIDIHandler::sendRawMessage(const MidiMessage& msg){}
void MIDIHandler::getDataFromManager(GestureManager::datastreams* DataStreamsPointer){}
