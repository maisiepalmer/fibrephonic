#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : presetTree("Presets")
{
    {
        //Setup for XML file directory Ensures it exists and file path is valid....
        juce::File directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("fibrephonic-juce");

        directory.createDirectory();

        xmlFile = directory.getChildFile("Presets.xml");
    }
    
    //Resize Main Window 
    setSize (1200, 700);

    {
        //Buttons and Toggles
        connectionsbutton.setLookAndFeel(&buttonlookandfeel);
        connectionsbutton.setButtonText("Connections");

        pConnectionsButton->onClick = [this] { isConnectionsToggled = !isConnectionsToggled; 
        DBG("isConnections = " << (isConnectionsToggled ? "true" : "false"));
        if(isCalibrationToggled == true)
            isConnectionsToggled = false;
            };

        calibrationbutton.setLookAndFeel(&buttonlookandfeel);
        calibrationbutton.setButtonText("Calibration");

        pCalibrationButton->onClick = [this] { isCalibrationToggled = !isCalibrationToggled; 
        DBG("isCalibration = " << (isCalibrationToggled ? "true" : "false"));
        if (isConnectionsToggled == true)
            isCalibrationToggled = false;
            };

        addAndMakeVisible(connectionsbutton);
        addAndMakeVisible(calibrationbutton);
    }

    {
        //Parameters
        
        juce::ValueTree mainTree("MainTreeRoot");

        SwatchTree.clear();
        SwatchTree.resize(6);

        presetTree = juce::ValueTree("PresetTree"); 
        presetTree.removeAllChildren(nullptr);

        // Clear previous children to avoid duplicates
        presetTree.removeAllChildren(nullptr); 

        for (int i = 0; i < SwatchTree.size(); ++i)
        {
            SwatchTree[i] = juce::ValueTree("Swatch");

            SwatchTree[i].setProperty("index", i, nullptr);
            SwatchTree[i].setProperty("enabled", true, nullptr);
            SwatchTree[i].setProperty("VID_NFC", 1111, nullptr);
            SwatchTree[i].setProperty("MIDI_Channel", 0, nullptr);
            SwatchTree[i].setProperty("Input_Max", 1111, nullptr);
            SwatchTree[i].setProperty("Input_Min", 0, nullptr);
            SwatchTree[i].setProperty("Transform", (int)0, nullptr);
            SwatchTree[i].setProperty("Output_Max", 1111, nullptr);
            SwatchTree[i].setProperty("Output_Min", 0, nullptr);
            SwatchTree[i].setProperty("Reset", (bool)0, nullptr);

            // Add to the parameterTree as child
            presetTree.addChild(SwatchTree[i], -1, nullptr);
        }

        juce::ValueTree parameterTree("ParameterTree");
        parameterTree.setProperty("someParameter", 42, nullptr);

        mainTree.addChild(presetTree, 0, nullptr);
        mainTree.addChild(parameterTree, -1, nullptr);

        // Save parameters to XML file
        if (auto xml = mainTree.createXml())
        {
            if (xml->writeTo(xmlFile))
                DBG("Saved XML successfully to: " << xmlFile.getFullPathName());
            else
                DBG("Failed to save XML to: " << xmlFile.getFullPathName());
        }
        else
        {
            DBG("Failed to create XML from presetTree");
        }
    }

}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Fill the background with a gradient
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // Define the gradient
    juce::ColourGradient gradient(
        juce::Colours::black, 0.0f, 0.0f,
        juce::Colours::darkgrey, (float)getWidth(), (float)getHeight(), false);

    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());

    // Parameters for outlines and corners
    float outlinethickness = 3.0f;
    float cornerRadius = 50.0f;

    // Draw main rounded rectangle outline
    juce::Rectangle<float> bounds = getLocalBounds().toFloat().reduced(outlinethickness * 0.5f);
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds, cornerRadius, outlinethickness);

    float lineY = cornerRadius + outlinethickness * 0.5f;

    g.fillRect(0.0f, lineY - (outlinethickness / 2.0f), (float)getWidth(), outlinethickness);

    // Draw title text centered above the line 
    float textY = lineY - 40.0f;  

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Fibrephonic Mapper", 0, (int)textY, getWidth(), 30, juce::Justification::centred);

    g.setFont(juce::Font(15.0f));

    //MUST CALL REPAINT
    if (isConnectionsToggled == true) {
        



        repaint(); 
    }
    else if (isCalibrationToggled == true){





        repaint();
    }
}

void MainComponent::resized()
{
    //Buttons and Toggles
    connectionsbutton.setBounds(0, 50, getWidth() / 2, 60);
    calibrationbutton.setBounds(getWidth() / 2, 50, getWidth() / 2, 60);
}

//==============================================================================
void MainComponent::timerCallback(){
    startTimerHz(30);  
    repaint();
}