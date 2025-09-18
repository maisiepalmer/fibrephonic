//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#ifndef H_Quaternion
#define H_Quaternion

#include <JuceHeader.h>
#include <array>
#include <cmath>

/** Orientation Structures */
struct Euler
{
    /** Enum for euler angle elemets */
    enum EulerConstants
    {
        Phi = 0,
        Theta,
        Psi,
        
        EulerSize
    };
    std::array<float, EulerSize> values;
};

/**
 Class for storing a Quternsion which includes some useful conversion and arithmetic functions
 
 The data is stored in a static array.
 
 Small class with public memebers for quaternion data.
 Quartenions are 4 elements. This class will also do euler and rotaion matrix calculations too
 
 */
struct Quaternion
{
    /** enum containing the array sizes for the arrays */
    enum QuaternionConstants
    {
        W = 0,
        X,
        Y,
        Z,
        
        QuaternionSize
    };
    /** Constructor */
    Quaternion();
    
    /** Constructor */
    Quaternion (std::array<float, QuaternionSize>& q);
    
    /** Constructor */
    Quaternion (const float q0, const float q1, const float q2, const float q3);
    
    /** Converts data to XYZ Euler angles (in degrees). */
    Euler toEulerAngles() const;
    
    //    /**
    //     Converts to rotation matrix.
    //     Elements 0 to 2 represent columns 1 to 3 of row 1.
    //     Elements 3 to 5 represent columns 1 to 3 of row 2.
    //     Elements 6 to 8 represent columns 1 to 3 of row 3.
    //     */
    //    void convertToRotationMatrix (float rotationMatrix[Quaternion::RotationMatrixSize]) const;
    
    /** Convert to conjugate */
    Quaternion getConjugate() const;
    
    /** Normalise */
    void normalise();
    
    /** returns the result of @a * @b */
    static Quaternion quaternionMultiply (const Quaternion a, const Quaternion b);
    
    std::array<float, QuaternionSize> values;
};

struct RotationMatrix
{
    /**
     Enum for rotation matrix indicies
     */
    enum RotationMatrixConstants
    {
        R11 = 0, R12, R13,
        R21, R22, R23,
        R31, R32, R33,
        
        RotationMatrixSize,
        
        XX = 0, XY, XZ,
        YX, YY, YZ,
        ZX, ZY, ZZ,
        
        X = 0, Y, Z
    };
    
    /** Constructor */
    RotationMatrix();
    
    /** Constructor */
    RotationMatrix (const float initial[RotationMatrixSize]);
    
    /** Constructor */
    RotationMatrix (float r11, float r12, float r13,
                    float r21, float r22, float r23,
                    float r31, float r32, float r33);
    
    /** Constructor Converts to rotation matrix.
     Elements 0 to 2 represent columns 1 to 3 of row 1.
     Elements 3 to 5 represent columns 1 to 3 of row 2.
     Elements 6 to 8 represent columns 1 to 3 of row 3.
     */
    RotationMatrix (const Quaternion& q);
    
    /** returns the transpose of this matrix */
    RotationMatrix getTranspose ();
    
    /** returns dot product of this with arg */
    RotationMatrix dot (RotationMatrix b);
    std::array<float, RotationMatrixSize> dot (std::array<float, RotationMatrixSize>& b);
    Array<float> dot (const Array<float>& b);
    
    static std::array<float, 3> rotationMultiply (const std::array<float, 3>& v, const RotationMatrix& a);
    static RotationMatrix rotationMultiply (const RotationMatrix& a, const RotationMatrix& b);
    
    static RotationMatrix eulerAnglesToRotationMatrix (float phi, float theta, float psi);
    
    static RotationMatrix axisAngleToRotationMatrix (Array<float> axisVector, float angleInDegrees);
    /** Converts data to XYZ Euler angles. */
    void convertToEulerAngles (Euler& e) const;
    
    std::array<float, RotationMatrixSize> values;
    
    static const std::array<float, RotationMatrixSize> identityMatrix;
};

#endif //H_Quaternion
