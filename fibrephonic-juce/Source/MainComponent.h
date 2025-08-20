#pragma once

#include <JuceHeader.h>
#include "Data/BluetoothConnectionManager.h"
#include "Data/GestureManager.h"

//==============================================================================

class MainComponent : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    //==============================================================================
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;
    std::shared_ptr<GestureManager> gestureManager;
    
    int functionCount = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
