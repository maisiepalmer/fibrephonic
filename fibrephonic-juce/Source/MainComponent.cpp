#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: bluetoothConnection(std::make_shared<BluetoothConnectionManager>())
, gestureManager(std::make_shared<GestureManager>(bluetoothConnection))
, midiHandler(std::make_shared<MIDIHandler>(gestureManager))
{
    //Resize Main Window
    setSize (1200, 700);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Fill the background with a gradient
    g.fillAll();
}

void MainComponent::resized()
{
    auto height = getHeight();
    auto width = getWidth();
}
