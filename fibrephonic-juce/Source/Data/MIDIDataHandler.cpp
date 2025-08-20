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
    gestureManager(gestureManagerInstance),
    midiOutFlag(false),
    quantise(true),
    numChannels(3)
{
    Data = gestureManager->pDATA;
    GESTURES = gestureManager->pGestures;

    note = ccVal = velocity = 0;

    X.resize(DATAWINDOW); Y.resize(DATAWINDOW); Z.resize(DATAWINDOW);

    auto names = getAvailableDeviceNames();
    for (const auto& name : names)
        DBG("MIDI Output Device: " << name);

    bool ConnectionSuccess = openDeviceByName("Springbeats vMIDI1");
    if (!ConnectionSuccess)
        DBG("Failed to Open Device...");
    else
        DBG("Connection Successful");
}

MIDIHandler::~MIDIHandler()
{
    midiOutFlag = false;

    Data = nullptr;
    delete Data;

    GESTURES = nullptr;
    delete GESTURES;

    X.clear(); Y.clear(); Z.clear();

    stop();
}

void MIDIHandler::run()
{
    midiOutFlag = true;
    MIDIOUT();
}

void MIDIHandler::stop()
{
    midiOutFlag = false;
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

    GESTURES = gestureManager->pGestures;
}

void MIDIHandler::MIDIgestureHandling(vector<int>& X, vector<int>& Y, vector<int>& Z, 
                                      int& Note, int& Velocity, int& CCVal)
{
    switch (*GESTURES) {
    case GestureManager::Gesture::NO_GESTURE:
        Note = 0;
        Velocity = 0;
        CCVal = 0;
        break;

    case GestureManager::Gesture::PITCH:
        break;
    case GestureManager::Gesture::ROLL:

        break;
    case GestureManager::Gesture::YAW:

        break;
    case GestureManager::Gesture::TAP:

        break;
    case GestureManager::Gesture::STROKE:

        break;
    default:
        *GESTURES = GestureManager::Gesture::NO_GESTURE;
        break;
    }
}

// Functionality 
void MIDIHandler::MIDIOUT()
{
    while (midiOutFlag)
    {
        getGestureManagerData();

        int Note = X.back();
        int Velocity = Y.back();
        int CCVal = Z.back();

        MIDIgestureHandling(X, Y, Z, Note, Velocity, CCVal);

        double normZ = (Z.back() - 1.0) / (6.0 - 1.0); // normalize between 0 and 1, for What CC Val is controlling 
        normZ = clamp(normZ, 0.0, 1.0);

        double normY = std::clamp(Y.back() / 127.0, 0.0, 1.0);

        double bpm = static_cast<double>(BPM);

        double minHoldBeats = 0.25; // quarter note min hold
        double maxHoldBeats = 2.0;  // 2 beats max hold

        double holdBeats = minHoldBeats + (maxHoldBeats - minHoldBeats) * (1.0 - normZ);

        int holdMs = static_cast<int>((60000.0 / bpm) * holdBeats);

        auto start = chrono::steady_clock::now();

        // Filter CC
        int cutoffval, resonanceval;

        cutoffval = juce::jlimit(0, 127, static_cast<int>(normZ * 127.0)); 

        double mappedResonance = juce::jmap(normY, 0.0, 1.0, 0.0, 127.0);
        resonanceval = juce::jlimit(0, 127, static_cast<int>(mappedResonance));

        if (isPositiveAndBelow(Note, 128)) // Checks if incoming note is posotive and above 128
        {
            // Play all notes at once
            for (int channel = 1; channel <= numChannels; ++channel) 
            {
                sendCC(channel, 74, cutoffval);
                sendCC(channel, 71, resonanceval);

                sendNoteOn(channel, Note, Velocity);
            }
                
            this_thread::sleep_until(start + chrono::milliseconds(holdMs)); // Hold notes (Time Between NoteOff)

            for (int channel = 1; channel <= numChannels; ++channel)
            {
                sendNoteOff(channel, Note);
            }
                
        }
        this_thread::sleep_for(chrono::milliseconds(holdMs)); // Interval before next group
    }
}

