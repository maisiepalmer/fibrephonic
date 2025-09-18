//
//  Vector.hpp
//  Glover - App
//
//  Created by Adam Stark on 25/03/2021.
//  Copyright © 2021 MiMU. All rights reserved.
//

#pragma once

//======================================================================
struct Vector
{
    Vector (float x_, float y_, float z_) : x (x_), y (y_), z (z_) {}
    Vector() : x (0.f), y (0.f), z (0.f) {}
    
    /** @Returns the L2 norm of this vector */
    float getL2Norm() const;
    
    /** @Returns the unit vector of this vector */
    Vector getUnitVector() const;
    
    /** @Returns the angle in radians between this one and another vector
    */
    float angleTo (const Vector& other);
    
    /** @Returns the dot product between this vector and another vector */
    float dotProduct (const Vector& other);
        
    float x {0.f};
    float y {0.f};
    float z {0.f};
};
