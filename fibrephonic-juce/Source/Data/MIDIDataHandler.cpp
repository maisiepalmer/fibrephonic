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
    : Thread("MIDIOutThread"), gestureManager(move(gestureManagerInstance))
{
    midioutflag = false;

    Data = gestureManager->pDATA;
    GESTURES = gestureManager->pGestures;

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

    stop();
}

void MIDIHandler::run()
{
    midioutflag = true;
    MIDIOUT();
}

void MIDIHandler::stop()
{
    midioutflag = false;
    stopThread(500);  
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

void MIDIHandler::getGestureManagerData()
{
    if (!gestureManager) { DBG("GestureManager is null!"); return; }

    vector<double> xScaledDouble = gestureManager->getScaledX();

    /*
    for (int i = 0; i < xScaledDouble.size(); i++) {
        DBG(xScaledDouble[i]);
    }
    */

    X.clear(); Y.clear(); Z.clear();

    X.reserve(xScaledDouble.size());
    for (double i : xScaledDouble)
        X.push_back(static_cast<int>(i));
    
    vector<double> yScaledDouble = gestureManager->getScaledY();

    Y.reserve(yScaledDouble.size());
    for (double i : yScaledDouble)
        Y.push_back(static_cast<int>(i));

    vector<double> zScaledDouble = gestureManager->getScaledZ();

    Z.reserve(zScaledDouble.size());
    for (double i : zScaledDouble)
        Z.push_back(static_cast<int>(i));
}

// Functionality 
void MIDIHandler::MIDIOUT() 
{
    while (midioutflag) {

        getGestureManagerData();

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
                DBG(Note);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}