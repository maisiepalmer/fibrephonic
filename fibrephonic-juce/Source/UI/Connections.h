/*
  ==============================================================================

    Connections.h
    Created: 19 Aug 2025 9:45:43am
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Connections  : public juce::Component
{
public:
    Connections()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~Connections() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    // dynamic array of rows!
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connections)
};
