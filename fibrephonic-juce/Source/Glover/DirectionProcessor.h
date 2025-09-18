//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "Definitions.h" 
#include "Vector.h"

//======================================================================
struct RotationMatrix;

//======================================================================
class DirectionProcessor
{
public:
    
    //======================================================================
    /** Calculates the point on a sphere of unit radius
     *  @param polarAngle the polar angle in the range [0, PI] where 0 is due north and PI is south
     *  @param azimuthalAngle the azimuth in the range [0, 2 * PI] where 0 can be considered 'forwards'
     */
    static Vector calculateSphericalPoint (float polarAngle, float azimuthalAngle);
    
    static const String getSegmentName (Segment segment);
        
    //======================================================================
    /** Constructor */
    DirectionProcessor();
        
    Direction getDirection (const RotationMatrix* rotation, float coneAngleDegrees);
    Segment getSegment (const RotationMatrix* rotation, float coneAngleDegrees);
    
    
private:
    
    
    void generateBasisVectorsForSixDirections();
    void generateBasisVectorsForTwentySixSegments();
    
    std::vector<Vector> directionalVectors;
    std::vector<Vector> segmentVectors;
};
