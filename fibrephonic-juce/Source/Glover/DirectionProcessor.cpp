//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#include "DirectionProcessor.h"
#include "Orientation.h"

const std::array<std::string, 27> segmentNames = {
    "Raised Forwards",
    "Raised Left Forwards",
    "Raised Left",
    "Raised Left Backwards",
    "Raised Backwards",
    "Raised Right Backwards",
    "Raised Right",
    "Raised Right Forwards",

    // middle
    "Forwards [seg]",
    "Left Forwards",
    "Left [seg]",
    "Left Backwards",
    "Backwards [seg]",
    "Right Backwards",
    "Right [seg]",
    "Right Forwards",

    // lowered
    "Lowered Forwards",
    "Lowered Left Forwards",
    "Lowered Left",
    "Lowered Left Backwards",
    "Lowered Backwards",
    "Lowered Right Backwards",
    "Lowered Right",
    "Lowered Right Forwards",

    // top & bottom
    "Up [seg]",
    "Down [seg]",
    
    // none of the above
    "Null Segment"
};

//======================================================================
const String DirectionProcessor::getSegmentName (Segment segment)
{
    if (isPositiveAndBelow (segment, segmentNames.size()))
        return segmentNames[static_cast<int> (segment)];
    
    return "";
}

//======================================================================
DirectionProcessor::DirectionProcessor()
{
    generateBasisVectorsForSixDirections();
    generateBasisVectorsForTwentySixSegments();
}

//======================================================================
Vector DirectionProcessor::calculateSphericalPoint (float polarAngle, float azimuthalAngle)
{
    Vector v;
    v.x = sin (polarAngle) * cos (azimuthalAngle);
    v.y = sin (polarAngle) * sin (azimuthalAngle);
    v.z = cos (polarAngle);
    return v;
}

//======================================================================
Direction DirectionProcessor::getDirection (const RotationMatrix* rotation, float coneAngleDegrees)
{
    float theta = coneAngleDegrees / 57.2957795130823; // deg to rad
    Vector gloveVector (rotation->values[0], rotation->values[3], rotation->values[6]);
    
    for (int i = 0; i < directionalVectors.size(); i++)
    {
        if (gloveVector.angleTo (directionalVectors[i]) < theta)
            return static_cast<Direction> (i);
    }
    
    return Direction::NullDirection;
}

//======================================================================
Segment DirectionProcessor::getSegment (const RotationMatrix* rotation, float coneAngleDegrees)
{
    float theta = coneAngleDegrees / 57.2957795130823; // deg to rad
    Vector gloveVector (rotation->values[0], rotation->values[3], rotation->values[6]);
    
    for (int i = 0; i < segmentVectors.size(); i++)
    {
        if (gloveVector.angleTo (segmentVectors[i]) < theta)
            return static_cast<Segment> (i);
    }
    
    return Segment::NullSegment;
}

//======================================================================
void DirectionProcessor::generateBasisVectorsForSixDirections()
{
    directionalVectors.clear();
    
    // these are hardcoded so the ordering of directions
    // matches our older direction calculation code
    
    // forwards
    directionalVectors.push_back (calculateSphericalPoint (MathConstants<float>::halfPi, 0.f));
    
    // backwards
    directionalVectors.push_back (calculateSphericalPoint (MathConstants<float>::halfPi, MathConstants<float>::pi));
    
    // left
    directionalVectors.push_back (calculateSphericalPoint (MathConstants<float>::halfPi, MathConstants<float>::halfPi));
    
    // right
    directionalVectors.push_back (calculateSphericalPoint (MathConstants<float>::halfPi, 3 * MathConstants<float>::halfPi));
    
    // up
    directionalVectors.push_back (calculateSphericalPoint (0.f, 0.f));
    
    // down
    directionalVectors.push_back (calculateSphericalPoint (MathConstants<float>::pi, 0.f));
}

//======================================================================
void DirectionProcessor::generateBasisVectorsForTwentySixSegments()
{
    segmentVectors.clear();
    
    // polar position is the "vertical" position or "row" of points on the sphere
    // 1 = top row
    // 2 = middle row
    // 3 = bottom row
    for (int polarPosition = 1; polarPosition <= 3; polarPosition++)
    {
        float polarAngle = static_cast<float> (polarPosition) * MathConstants<float>::pi / 4.f;
        
        for (int i = 0; i < 8; i++)
        {
            float azimuthalAngle = static_cast<float> (i) * MathConstants<float>::pi / 4.f;
            segmentVectors.push_back (calculateSphericalPoint (polarAngle, azimuthalAngle));
        }
    }
    
    // up
    segmentVectors.push_back (calculateSphericalPoint (0.f, 0.f));
    
    // down
    segmentVectors.push_back (calculateSphericalPoint (MathConstants<float>::pi, 0.f));
}
