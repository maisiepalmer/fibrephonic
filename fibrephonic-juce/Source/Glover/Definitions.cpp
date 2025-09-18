//
//  Definitions.cpp
//  Glover - App
//
//  Created by Adam STARK on 21/05/2019.
//  Copyright © 2019 MI.MU. All rights reserved.
//

#include "Definitions.h"

//=================================================================================
namespace Glover
{
    //=================================================================================
    // Arrangement
    const Identifier Glover::Ids::solo ("Solo");
    const Identifier Glover::Ids::mute ("Mute");
    const Identifier Glover::Ids::selectedAsCurrentMovementSignalSource ("SelectedAsCurrentMovementSignalSource");
    const Identifier Glover::Ids::selectedAsCurrentEventSignalSource ("SelectedAsCurrentEventSignalSource");
    
    //=================================================================================
    // Device Properties
    const Identifier Glover::Ids::hand ("Hand");
    
    const Identifier Glover::Ids::directionIds[] = {"Forwards", "Backwards", "Left", "Right", "Up", "Down"};
    
    const Identifier Glover::Ids::segmentIds[] = {
        "RaisedForwards",
        "RaisedLeftForwards",
        "RaisedLeft",
        "RaisedLeftBackwards",
        "RaisedBackwards",
        "RaisedRightBackwards",
        "RaisedRight",
        "RaisedRightForwards",

        // middle
        "MiddleForwards",
        "MiddleLeftForwards",
        "MiddleLeft",
        "MiddleLeftBackwards",
        "MiddleBackwards",
        "MiddleRightBackwards",
        "MiddleRight",
        "MiddleRightForwards",

        // lowered
        "LoweredForwards",
        "LoweredLeftForwards",
        "LoweredLeft",
        "LoweredLeftBackwards",
        "LoweredBackwards",
        "LoweredRightBackwards",
        "LoweredRight",
        "LoweredRightForwards",

        // top & bottom
        "Top",
        "Bottom"
    };
    
    const Identifier Glover::Ids::currentDirection ("CurrentDirection");
    
    const Identifier Glover::Ids::pitch ("Pitch");;
    const Identifier Glover::Ids::roll ("Roll");;
    const Identifier Glover::Ids::yaw ("Yaw");;
}

