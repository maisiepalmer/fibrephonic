/*
  ==============================================================================

    Identifiers.h
    Created: 20 Aug 2025 11:26:11am
    Author:  Maisie Palmer

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

//==============================================================================
/**
 *  A namespace that contains the identifiers for all values in the value tree.
 */
namespace Identifiers
{
    struct EntireState
    {
        static const inline juce::Identifier State {"state"};
    };

    struct Calibration
    {
        static const inline juce::Identifier State {"calibration"};
        static const inline juce::Identifier BluetoothPoll {"bluetooth_poll"};
        static const inline juce::Identifier BluetoothOptions {"bluetooth_options"};
        static const inline juce::Identifier BluetoothSelection {"bluetooth_selection"};
        static const inline juce::Identifier SerialPoll {"serial_poll"};
        static const inline juce::Identifier SerialOptions {"serial_options"};
        static const inline juce::Identifier SerialSelection {"serial_selection"};
    };

    struct Connections
    {
        static const inline juce::Identifier State {"connections"};
        
        struct Connection
        {
            static const inline juce::Identifier State {"connection"};
            static const inline juce::Identifier Index {"connection_index"};
            static const inline juce::Identifier MidiChannel {"midi_channel"};
            static const inline juce::Identifier MidiCC {"midi_cc"};
        };
    };

    struct Swatches
    {
        static const inline juce::Identifier State {"swatches"};
        
        struct Swatch
        {
            static const inline juce::Identifier State {"swatch"};
            static const inline juce::Identifier Index {"index"};
            static const inline juce::Identifier Name {"name"};
            static const inline juce::Identifier Enabled {"enabled"};
            static const inline juce::Identifier NfcId {"nfc_id"};
            static const inline juce::Identifier InputMin {"input_min"};
            static const inline juce::Identifier InputMax {"input_max"};
            static const inline juce::Identifier Transform {"transform"};
            static const inline juce::Identifier OutputMin {"output_min"};
            static const inline juce::Identifier OutputMax {"output_max"};
        };
    };
};
