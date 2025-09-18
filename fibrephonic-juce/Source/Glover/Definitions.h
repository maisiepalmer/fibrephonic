//
//  Definitions.h
//  Glover - App
//
//  Created by Adam STARK on 21/05/2019.
//  Copyright © 2019 MI.MU. All rights reserved.
//

#ifndef Definitions_h
#define Definitions_h

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
enum Hand
{
    Left,
    Right,
    Both,
    None
};

//=================================================================================
enum ConnectionState
{
    Disconnected,
    ConnectedWifi,
    ConnectedUsb,
    Connecting,
    Discovered
};

//=================================================================================
enum PalmTilt
{
    PalmLeft,
    PalmRight,
    PalmDown,
    PalmUp
};

//=================================================================================
enum Direction
{
    posX, // forwards
    negX, // backwards
    posY, // left
    negY, // right
    posZ, // up
    negZ, // down
    
    NullDirection//none of the above
};

//======================================================================
/** The twenty-six different segments */
enum Segment
{
    // elevated
    ElevatedForwards,
    ElevatedLeftForwards,
    ElevatedLeft,
    ElevatedLeftBackwards,
    ElevatedBackwards,
    ElevatedRightBackwards,
    ElevatedRight,
    ElevatedRightForwards,

    // middle
    MiddleForwards,
    MiddleLeftForwards,
    MiddleLeft,
    MiddleLeftBackwards,
    MiddleBackwards,
    MiddleRightBackwards,
    MiddleRight,
    MiddleRightForwards,

    // lowered
    LoweredForwards,
    LoweredLeftForwards,
    LoweredLeft,
    LoweredLeftBackwards,
    LoweredBackwards,
    LoweredRightBackwards,
    LoweredRight,
    LoweredRightForwards,

    // top & bottom
    Top,
    Bottom,
    
    // none of the above
    NullSegment
};

//=================================================================================
namespace Glover
{
    //=================================================================================
    struct Ids
    {
        // Arrangement
        static const Identifier solo;
        static const Identifier mute;
        static const Identifier selectedAsCurrentMovementSignalSource;
        static const Identifier selectedAsCurrentEventSignalSource;
        
        // Device Properties
        static const Identifier hand;
        
        // Device Inputs
        // ..
        static const Identifier directionIds[6];
        static const Identifier segmentIds[26];
        static const Identifier currentDirection;
        
        static const Identifier pitch;
        static const Identifier roll;
        static const Identifier yaw;
        
        // Device Outputs
        // ..
    };
};

#endif /* Definitions_h */
