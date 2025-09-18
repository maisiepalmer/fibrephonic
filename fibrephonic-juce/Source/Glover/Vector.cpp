//
//  Vector.cpp
//  Glover - App
//
//  Created by Adam Stark on 25/03/2021.
//  Copyright © 2021 MiMU. All rights reserved.
//

#include "Vector.h"
#include <JuceHeader.h>

//======================================================================
float Vector::getL2Norm() const
{
    return sqrt (x * x + y * y + z * z);
}

//======================================================================
Vector Vector::getUnitVector() const
{
    float norm = getL2Norm();
    
    if (norm > 0.f)
    {
        return Vector {x / norm, y / norm, z / norm};
    }
    else
    {
        // the unit vector only exists for non zero vectors
        jassertfalse;
        return Vector();
    }
}

//======================================================================
float Vector::angleTo (const Vector& other)
{
    auto vector1UnitVector = this->getUnitVector();
    auto vector2UnitVector = other.getUnitVector();
    float dotProduct = vector1UnitVector.dotProduct (vector2UnitVector);
    return acos (jlimit (-1.f, 1.f, dotProduct));
}

//======================================================================
float Vector::dotProduct (const Vector& other)
{
    return x * other.x + y * other.y + z * other.z;
}
