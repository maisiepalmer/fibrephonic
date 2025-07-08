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
    : Thread("MIDIOutThread"), 
    gestureManager(move(gestureManagerInstance)),
    midioutflag(false)
{
    Data = gestureManager->pDATA;
    GESTURES = gestureManager->pGestures;

    Channel = Note = CCVal = Velocity = 0;

    X.resize(DATAWINDOW);
    Y.resize(DATAWINDOW);
    Z.resize(DATAWINDOW);

    /*
    bool ConnectionSuccess = openDeviceByName("Springbeats vMIDI1");
    if (!ConnectionSuccess)
        DBG("Failed to Open Device...");
    else
        DBG("Connection Successful");
    */
    //getAvailableDeviceNames();
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

bool MIDIHandler::openDeviceByIndex(int index)
{
    auto devices = MidiOutput::getAvailableDevices();
    if (index < 0 || index >= devices.size())
        return false;

    midiOut = MidiOutput::openDevice(devices[index].identifier);
    return midiOut != nullptr;
}

bool MIDIHandler::openDeviceByName(const String& deviceName)
{
    auto devices = MidiOutput::getAvailableDevices();

    for (const auto& device : devices)
    {
        if (device.name == deviceName)
        {
            midiOut = MidiOutput::openDevice(device.identifier);
            return midiOut != nullptr;
        }
    }

    return false; // Device not found
}

void MIDIHandler::closeDevice()
{
    if (midiOut)
    {
        midiOut.reset();  // Closes and deletes the device internally
    }
}

// Utility
StringArray MIDIHandler::getAvailableDeviceNames() const
{
    StringArray names;
    for (auto& device : MidiOutput::getAvailableDevices())
        names.add(device.name);
    return names;
}

int MIDIHandler::getNumAvailableDevices() const
{
    return MidiOutput::getAvailableDevices().size();
}

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

void MIDIHandler::sendNoteOff(int channel, int note)
{
    if (midiOut &&
        juce::isPositiveAndBelow(channel, 16) &&
        juce::isPositiveAndBelow(note, 128))
    {
        juce::MidiMessage msg = juce::MidiMessage::noteOff(channel, note);
        midiOut->sendMessageNow(msg);
    }
}

void MIDIHandler::sendCC(int channel, int controller, int value)
{
    if (midiOut &&
        juce::isPositiveAndBelow(channel, 16) &&
        juce::isPositiveAndBelow(controller, 128) &&
        juce::isPositiveAndBelow(value, 128))
    {
        juce::MidiMessage ccMsg = juce::MidiMessage::controllerEvent(channel, controller, value);
        midiOut->sendMessageNow(ccMsg);
    }
}

void MIDIHandler::sendRawMessage(const juce::MidiMessage& msg)
{
    if (midiOut)
        midiOut->sendMessageNow(msg);
}

void MIDIHandler::getGestureManagerData()
{
    if (!gestureManager) { DBG("GestureManager is null!"); return; }

    vector<double> xScaledDouble = gestureManager->getScaledX();

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

                Note = X[midibufferpos]; Velocity = Y[midibufferpos]; CCVal = Z[midibufferpos];

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