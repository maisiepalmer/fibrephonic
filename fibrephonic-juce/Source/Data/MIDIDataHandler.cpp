/*
  ==============================================================================

    MIDIDataHandler.cpp
    Created: 4 Jul 2025 4:22:09pm
    Author: Joseph B

  ==============================================================================
*/

#include "MIDIDataHandler.h"

// Initialisation
MIDIHandler::MIDIHandler(shared_ptr<GestureManager> gestureManagerInstance)
    : gestureManager(move(gestureManagerInstance))
{
    Data = gestureManager->pDATA;
    GESTURES = gestureManager->pGestures;

    midioutflag = false;

    Channel = Note = CCVal = Velocity = 0;

    X.resize(DATAWINDOW);
    Y.resize(DATAWINDOW);
    Z.resize(DATAWINDOW);
}

MIDIHandler::~MIDIHandler()
{
    midioutflag = false;

    Data = nullptr;
    delete Data;

    GESTURES = nullptr;
    delete GESTURES;

    X.clear(); Y.clear(); Z.clear();
}

bool MIDIHandler::openDeviceByIndex(int index){ return false;}
bool MIDIHandler::openDeviceByName(const String& deviceName){ return false;}
void MIDIHandler::closeDevice(){}

// Utility
StringArray MIDIHandler::getAvailableDeviceNames() const{ return {}; }
int MIDIHandler::getNumAvailableDevices() const{ return 0; }

// MIDI Send
void MIDIHandler::sendNoteOn(int channel, int note, int velocity)
{
    if (midiOut && isPositiveAndBelow(channel, MAXNO_MIDICHANNELS) &&
                   isPositiveAndBelow(note, MAXNO_MIDIVAL) &&
                   isPositiveAndBelow(velocity, MAXNO_MIDIVAL)) 
    {
        MidiMessage msg = MidiMessage::noteOn(channel, note, (uint8)velocity);
        midiOut->sendMessageNow(msg);
    }
}

void MIDIHandler::sendNoteOff(int channel, int note){}
void MIDIHandler::sendCC(int channel, int controller, int value){}
void MIDIHandler::sendRawMessage(const MidiMessage& msg){}

void MIDIHandler::copyVectors() 
{
    copy(Data->xScaled.begin(), Data->xScaled.end(), X.begin());
    copy(Data->yScaled.begin(), Data->yScaled.end(), Y.begin());
    copy(Data->zScaled.begin(), Data->zScaled.end(), Z.begin());
}

// Functionality 
void MIDIHandler::MIDIOUT() 
{
    midioutflag = true; // Allows for force stop 

    copyVectors();

    while (midioutflag == true) {

        for (int channel = 0; channel < MAXNO_MIDICHANNELS; channel++) {

            int CurrentXMessage, CurrentYMessage, CurrentZMessage;

            for (int midibufferpos = 0; midibufferpos < MAXNO_MIDIVAL; midibufferpos++) {

                Note = X[midibufferpos];
                Velocity = Y[midibufferpos];
                CCVal = Z[midibufferpos];

                /* Send note on based on identified gesture from gesture manager...
                
                // Example: send note-on
                if (midiOut && juce::isPositiveAndBelow(note, 128))
                {
                    midiOut->sendMessageNow(juce::MidiMessage::noteOn(channel, note, (uint8)velocity));
                    midiOut->sendMessageNow(juce::MidiMessage::controllerEvent(channel, 74, ccValue)); // CC74 = brightness
                }
             
                NOTE: this will be wrapped up with send functions 
                */
            }
        }
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}