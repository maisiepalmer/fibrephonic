#pragma once

#include <JuceHeader.h>
#include "UI/LookandFeel.h"
#include "UI/Connections.h"
#include "UI/Calibration.h"
#include "Data/BluetoothConnectionManager.h"
#include "Data/SerialPort.h"
#include "Data/GestureManager.h"
#include "Data/MIDIDataHandler.h"

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
    
    //==============================================================================
    std::shared_ptr<juce::ValueTree> initialiseTree();
    void saveState(bool asPreset);
    void loadState(bool asPreset);
    
private:
    //==============================================================================
    std::shared_ptr<juce::ValueTree> parameters;
    std::shared_ptr<juce::ValueTree> calTree;
    std::shared_ptr<juce::ValueTree> conTree;
    std::shared_ptr<juce::ValueTree> swaTree;
    juce::File stateFile;
    
    std::shared_ptr<BluetoothConnectionManager> bluetoothConnection;
    std::shared_ptr<GestureManager> gestureManager;
    std::shared_ptr<MIDIHandler> midiHandler;
    
    //Custom Look and Feel Instances
    ButtonLookandFeel buttonlookandfeel;
    RoundedButtonLookandFeel roundedbuttonlookandfeel;

    TextButton connections, calibration;

    TextButton* pConnectionsButton = &connections;
    TextButton* pCalibrationButton = &calibration;
    
    Connections connectionsWindow;
    Calibration calibrationWindow;
    
    enum Windows
    {
        CONNECTIONS_WINDOW = 0,
        CALIBRATION_WINDOW,
        TOTAL_WINDOWS
    };
    
    int windowSelected = 0;
    
    void updateWindows(int visibleWindow)
    {
        windowSelected = visibleWindow;
        switch(visibleWindow)
        {
            case Windows::CONNECTIONS_WINDOW:
                connectionsWindow.setVisible(true);
                calibrationWindow.setVisible(false);
                break;
            case Windows::CALIBRATION_WINDOW:
                calibrationWindow.setVisible(true);
                connectionsWindow.setVisible(false);
                break;
            default:
                break;
        }
    }
    
    int functionCount = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
