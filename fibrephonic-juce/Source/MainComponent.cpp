#include "MainComponent.h"
#include "Identifiers.h"

//==============================================================================
MainComponent::MainComponent()
: parameters(initialiseTree())
, calTree(std::make_shared<juce::ValueTree>(parameters->getChildWithName("calibration")))
, conTree(std::make_shared<juce::ValueTree>(parameters->getChildWithName("connections")))
, swaTree(std::make_shared<juce::ValueTree>(parameters->getChildWithName("swatches")))
, bluetoothConnection(std::make_shared<BluetoothConnectionManager>(calTree))
, gestureManager(std::make_shared<GestureManager>(bluetoothConnection))
, midiHandler(std::make_shared<MIDIHandler>(gestureManager))
, calibrationWindow(calTree, swaTree)
{
    loadState(false);
    
    // save state
    juce::File directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("fibrephonic-save-state");
    directory.createDirectory();
    stateFile = directory.getChildFile("State.xml");
    
    
    //Resize Main Window
    setSize (1200, 700);
    
    // Menu
    connections.setLookAndFeel(&buttonlookandfeel);
    connections.setButtonText("Connections");

    pConnectionsButton->onClick = [this] {
        updateWindows(Windows::CONNECTIONS_WINDOW);
    };
    
    calibration.setLookAndFeel(&buttonlookandfeel);
    calibration.setButtonText("Calibration");
    
    pCalibrationButton->onClick = [this] {
        updateWindows(Windows::CALIBRATION_WINDOW);
    };
    
    addAndMakeVisible(connections);
    addAndMakeVisible(calibration);
    addAndMakeVisible(connectionsWindow);
    addAndMakeVisible(calibrationWindow);
}

MainComponent::~MainComponent()
{
    saveState(false);
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
}

void MainComponent::resized()
{
    auto height = getHeight();
    auto width = getWidth();
    
    auto menuHeight = 50;
    
    // menu
    connections.setBounds(0, 0, width/2, menuHeight);
    calibration.setBounds(width/2, 0, width/ 2, menuHeight);
    
    // windows
    connectionsWindow.setBounds(0, menuHeight, width, height-menuHeight);
    calibrationWindow.setBounds(0, menuHeight, width, height-menuHeight);
}

//==============================================================================
std::shared_ptr<juce::ValueTree> MainComponent::initialiseTree()
{
    juce::ValueTree stateTree (Identifiers::EntireState::State);
    
    juce::ValueTree connectionsTree (Identifiers::Connections::State);
    for (int i = 0; i < 8; ++i)
    {
        juce::ValueTree connection(Identifiers::Connections::Connection::State);
        connection.setProperty(Identifiers::Connections::Connection::MidiChannel, 0, nullptr);
        connection.setProperty(Identifiers::Connections::Connection::MidiCC, 0, nullptr);
        connectionsTree.addChild(connection, i, nullptr);
    }
    stateTree.addChild(connectionsTree, 0, nullptr);
    
    juce::ValueTree calibrationTree (Identifiers::Calibration::State);
    calibrationTree.setProperty(Identifiers::Calibration::BluetoothPoll, false, nullptr);
    calibrationTree.setProperty(Identifiers::Calibration::BluetoothOptions, "", nullptr);
    calibrationTree.setProperty(Identifiers::Calibration::BluetoothSelection, 0, nullptr);
    calibrationTree.setProperty(Identifiers::Calibration::SerialPoll, false, nullptr);
    calibrationTree.setProperty(Identifiers::Calibration::SerialOptions, 0, nullptr);
    calibrationTree.setProperty(Identifiers::Calibration::SerialSelection, 0, nullptr);
    
    stateTree.addChild(calibrationTree, 1, nullptr);
    
    juce::ValueTree swatchesTree (Identifiers::Swatches::State);
    for (int i = 0; i < 8; ++i)
    {
        juce::ValueTree swatch (Identifiers::Swatches::Swatch::State);
        swatch.setProperty(Identifiers::Swatches::Swatch::Index, i, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::Name, "swatch", nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::Enabled, false, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::NfcId, 0, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::InputMin, 0, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::InputMax, 0, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::Transform, 0, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::OutputMin, 0, nullptr);
        swatch.setProperty(Identifiers::Swatches::Swatch::OutputMax, 0, nullptr);
        
        swatchesTree.addChild(swatch, i, nullptr);
    }
    
    stateTree.addChild(swatchesTree, 2, nullptr);
    
    return make_shared<juce::ValueTree>(stateTree);
}


void MainComponent::saveState(bool asPreset)
{
    auto xml = parameters->createXml();
    juce::File saveLocation = stateFile;
    
    if (asPreset)
    {
        std::unique_ptr<juce::FileChooser> chooser = std::make_unique<juce::FileChooser> ("Please select where you want to save...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory));
     
        auto folderChooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectDirectories;
     
        chooser->launchAsync (folderChooserFlags, [&, this] (const juce::FileChooser& chooser)
        {
            juce::File file = chooser.getResult();
            saveLocation = chooser.getResult();
        });
    }
    
    xml->writeTo(saveLocation);
    DBG("Saved XML successfully to: " << stateFile.getFullPathName());
}

void MainComponent::loadState(bool asPreset)
{
    juce::File loadLocation = stateFile;
    
    if (asPreset)
    {
        std::unique_ptr<juce::FileChooser> chooser = std::make_unique<juce::FileChooser> ("Please select the file you want to load...",
                                                   juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
                                                   "*.xml");
     
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
     
        chooser->launchAsync (folderChooserFlags, [&, this] (const juce::FileChooser& chooser)
        {
            juce::File file = chooser.getResult();
            loadLocation = chooser.getResult();
        });
    }
    
    if (auto xml = juce::XmlDocument::parse(loadLocation))
    {
        juce::ValueTree tree = juce::ValueTree::fromXml(*xml);
        parameters = std::make_shared<juce::ValueTree>(tree.createCopy());
    }
}
