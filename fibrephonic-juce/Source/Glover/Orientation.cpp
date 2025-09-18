//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#include "Orientation.h"

//Quaternion===========================================================
Quaternion::Quaternion()
{
    values.fill (0.f);
    values[0] = 1.f;
}

Quaternion::Quaternion (std::array<float, QuaternionSize>& q)
{
    values = q;
    normalise();
}

Quaternion::Quaternion (const float q0, const float q1, const float q2, const float q3)
{
    values[0] = q0;
    values[1] = q1;
    values[2] = q2;
    values[3] = q3;
    normalise();
}

Euler Quaternion::toEulerAngles() const
{
    float phi = (float)std::atan2(2 * (values[2] * values[3] - values[0] * values[1]), 2 * values[0] * values[0] - 1 + 2 * values[3] * values[3]);
    float theta = (float)-std::atan((2.0 * (values[1] * values[3] + values[0] * values[2])) / std::sqrt(1.0 - std::pow((2.0 * values[1] * values[3] + 2.0 * values[0] * values[2]), 2.0)));
    float psi = (float)std::atan2(2 * (values[1] * values[2] - values[0] * values[3]), 2 * values[0] * values[0] - 1 + 2 * values[1] * values[1]);
    
    Euler e;
    e.values[Euler::Phi] = radiansToDegrees(phi);
    e.values[Euler::Theta] = radiansToDegrees(theta);
    e.values[Euler::Psi] = radiansToDegrees(psi);
    
    return e;
}

//void Quaternion::convertToRotationMatrix (float rotationMatrix[Quaternion::RotationMatrixSize]) const
//{
//    rotationMatrix[R11] = 2 * values[0] * values[0] - 1 + 2 * values[1] * values[1];
//    rotationMatrix[R12] = 2 * (values[1] * values[2] + values[0] * values[3]);
//    rotationMatrix[R13] = 2 * (values[1] * values[3] - values[0] * values[2]);
//    rotationMatrix[R21] = 2 * (values[1] * values[2] - values[0] * values[3]);
//    rotationMatrix[R22] = 2 * values[0] * values[0] - 1 + 2 * values[2] * values[2];
//    rotationMatrix[R23] = 2 * (values[2] * values[3] + values[0] * values[1]);
//    rotationMatrix[R31] = 2 * (values[1] * values[3] + values[0] * values[2]);
//    rotationMatrix[R32] = 2 * (values[2] * values[3] - values[0] * values[1]);
//    rotationMatrix[R33] = 2 * values[0] * values[0] - 1 + 2 * values[3] * values[3];
//}

Quaternion Quaternion::getConjugate() const
{
    Quaternion result;
    result.values[0] = values[0];
    result.values[1] = -values[1];
    result.values[2] = -values[2];
    result.values[3] = -values[3];
    
    return result;
}

/**
 Normalise
 */
void Quaternion::normalise()
{
    float mag = sqrtf(values[0] * values[0] +
                      values[1] * values[1] +
                      values[2] * values[2] +
                      values[3] * values[3]);
    
    values[0] /= mag;
    values[1] /= mag;
    values[2] /= mag;
    values[3] /= mag;
    
}

Quaternion Quaternion::quaternionMultiply(const Quaternion a, const Quaternion b)
{
    Quaternion result;
    result.values[0] = a.values[0]*b.values[0] - a.values[1]*b.values[1] - a.values[2]*b.values[2] - a.values[3]*b.values[3];
    result.values[1] = a.values[0]*b.values[1] + a.values[1]*b.values[0] + a.values[2]*b.values[3] - a.values[3]*b.values[2];
    result.values[2] = a.values[0]*b.values[2] - a.values[1]*b.values[3] + a.values[2]*b.values[0] + a.values[3]*b.values[1];
    result.values[3] = a.values[0]*b.values[3] + a.values[1]*b.values[2] - a.values[2]*b.values[1] + a.values[3]*b.values[0];
    
    return result;
}

//RotationMatrix===========================================================
const std::array<float, RotationMatrix::RotationMatrixSize> RotationMatrix::identityMatrix = {1 ,0 , 0, 0, 1, 0, 0, 0, 1};

RotationMatrix::RotationMatrix() : values (identityMatrix)
{
    
}

RotationMatrix::RotationMatrix (const float initial[RotationMatrixSize])
{
    values[R11] = initial[R11];
    values[R12] = initial[R12];
    values[R13] = initial[R13];
    values[R21] = initial[R21];
    values[R22] = initial[R22];
    values[R23] = initial[R23];
    values[R31] = initial[R31];
    values[R32] = initial[R32];
    values[R33] = initial[R33];
}

RotationMatrix::RotationMatrix (float r11, float r12, float r13,
                                float r21, float r22, float r23,
                                float r31, float r32, float r33)
{
    values[R11] = r11;
    values[R12] = r12;
    values[R13] = r13;
    values[R21] = r21;
    values[R22] = r22;
    values[R23] = r23;
    values[R31] = r31;
    values[R32] = r32;
    values[R33] = r33;
}

/** Constructor Converts to rotation matrix.
 Elements 0 to 2 represent columns 1 to 3 of row 1.
 Elements 3 to 5 represent columns 1 to 3 of row 2.
 Elements 6 to 8 represent columns 1 to 3 of row 3.
 */
RotationMatrix::RotationMatrix (const Quaternion& q)
{
    values[R11] = 2 * q.values[0]  * q.values[0] - 1 + 2 * q.values[1] * q.values[1];
    values[R12] = 2 * (q.values[1] * q.values[2] + q.values[0] * q.values[3]);
    values[R13] = 2 * (q.values[1] * q.values[3] - q.values[0] * q.values[2]);
    values[R21] = 2 * (q.values[1] * q.values[2] - q.values[0] * q.values[3]);
    values[R22] = 2 * q.values[0]  * q.values[0] - 1 + 2 * q.values[2] * q.values[2];
    values[R23] = 2 * (q.values[2] * q.values[3] + q.values[0] * q.values[1]);
    values[R31] = 2 * (q.values[1] * q.values[3] + q.values[0] * q.values[2]);
    values[R32] = 2 * (q.values[2] * q.values[3] - q.values[0] * q.values[1]);
    values[R33] = 2 * q.values[0]  * q.values[0] - 1 + 2 * q.values[3] * q.values[3];
}

/** returns the transpose of this matrix */
RotationMatrix RotationMatrix::getTranspose ()
{
    //x11 = a11xb11 + a12xb21 + a13xb31
    float transpose[RotationMatrixSize] = { values[0], values[3], values[6],
                                            values[1], values[4], values[7],
                                            values[2], values[5], values[8]  };
    return RotationMatrix (transpose);
}

RotationMatrix RotationMatrix::dot (RotationMatrix b)
{
    return {values[0]*b.values[0] + values[1]*b.values[3] + values[2]*b.values[6],
            values[0]*b.values[1] + values[1]*b.values[4] + values[2]*b.values[7],
            values[0]*b.values[2] + values[1]*b.values[5] + values[2]*b.values[8],
        
            values[3]*b.values[0] + values[4]*b.values[3] + values[5]*b.values[6],
            values[3]*b.values[1] + values[4]*b.values[4] + values[5]*b.values[7],
            values[3]*b.values[2] + values[4]*b.values[5] + values[5]*b.values[8],
        
            values[6]*b.values[0] + values[7]*b.values[3] + values[8]*b.values[6],
            values[6]*b.values[1] + values[7]*b.values[4] + values[8]*b.values[7],
            values[6]*b.values[2] + values[7]*b.values[5] + values[8]*b.values[8]};
}

std::array<float, RotationMatrix::RotationMatrixSize> RotationMatrix::dot (std::array<float, RotationMatrixSize>& b)
{
    jassert (b.size() == 3);
    
    return {values[0] * b[0] + values[1] * b[1] + values[2] * b[2],
            values[3] * b[0] + values[4] * b[1] + values[5] * b[2],
            values[6] * b[0] + values[7] * b[1] + values[8] * b[2]};
}

Array<float> RotationMatrix::dot (const Array<float>& b)
{
    jassert (b.size() == 3);
    
    return {values[0] * b[0] + values[1] * b[1] + values[2] * b[2],
        values[3] * b[0] + values[4] * b[1] + values[5] * b[2],
        values[6] * b[0] + values[7] * b[1] + values[8] * b[2]};
}

RotationMatrix RotationMatrix::rotationMultiply (const RotationMatrix& a, const RotationMatrix& b)
{
    float result[RotationMatrixSize] =
    {   a.values[0]*b.values[0] + a.values[1]*b.values[3] + a.values[2]*b.values[6],
        a.values[0]*b.values[1] + a.values[1]*b.values[4] + a.values[2]*b.values[7],
        a.values[0]*b.values[2] + a.values[1]*b.values[5] + a.values[2]*b.values[8],
        
        a.values[3]*b.values[0] + a.values[4]*b.values[3] + a.values[5]*b.values[6],
        a.values[3]*b.values[1] + a.values[4]*b.values[4] + a.values[5]*b.values[7],
        a.values[3]*b.values[2] + a.values[4]*b.values[5] + a.values[5]*b.values[8],
        
        a.values[6]*b.values[0] + a.values[7]*b.values[3] + a.values[8]*b.values[6],
        a.values[6]*b.values[1] + a.values[7]*b.values[4] + a.values[8]*b.values[7],
        a.values[6]*b.values[2] + a.values[7]*b.values[5] + a.values[8]*b.values[8]  };
    
    return RotationMatrix (result);
}

std::array<float, 3> RotationMatrix::rotationMultiply (const std::array<float, 3>& v, const RotationMatrix& a)
{
    std::array<float, 3> array =
    {   v[X]*a.values[XX] + v[Y]*a.values[XY] + v[Z]*a.values[XZ],
        v[X]*a.values[YX] + v[Y]*a.values[YY] + v[Z]*a.values[YZ],
        v[X]*a.values[ZX] + v[Y]*a.values[ZY] + v[Z]*a.values[ZZ] };
    
    return array;
}

RotationMatrix RotationMatrix::eulerAnglesToRotationMatrix (float phi, float theta, float psi)
{
    float matrix[RotationMatrixSize];
    
    matrix[R11] = cosf(psi)*cosf(theta);
    matrix[R12] = -sinf(psi)*cosf(phi) + cosf(psi)*sinf(theta)*sinf(phi);
    matrix[R13] = sinf(psi)*sinf(phi) + cosf(psi)*sinf(theta)*cosf(phi);
    
    matrix[R21] = sinf(psi)*cosf(theta);
    matrix[R22] = cosf(psi)*cosf(phi) + sinf(psi)*sinf(theta)*sinf(phi);
    matrix[R23] = -cosf(psi)*sinf(phi) + sinf(psi)*sinf(theta)*cosf(phi);
    
    matrix[R31] = -sinf(theta);
    matrix[R32] = cosf(theta)*sinf(phi);
    matrix[R33] = cosf(theta)*cosf(phi);
    
    return RotationMatrix (matrix);
}

RotationMatrix RotationMatrix::axisAngleToRotationMatrix (Array<float> axisList, float angleInDegrees)
{
    const float angle = degreesToRadians (angleInDegrees);
    
    const float* axisVector = axisList.begin();
    float kx = axisVector[0];
    float ky = axisVector[1];
    float kz = axisVector[2];
    float cT = cos (angle);
    float sT = sin (angle);
    float vT = 1 - cos (angle);
    
    float R11 = kx * kx * vT + cT;
    float R12 = kx * ky * vT - kz * sT;
    float R13 = kx * kz * vT + ky * sT;
    float R21 = kx * ky * vT + kz * sT;
    float R22 = ky * ky * vT + cT;
    float R23 = ky * kz * vT - kx * sT;
    float R31 = kx * kz * vT - ky * sT;
    float R32 = ky * kz * vT + kx * sT;
    float R33 = kz * kz * vT + cT;
    
    return {R11,  R12,  R13,
            R21,  R22,  R23,
            R31,  R32,  R33};
}

void RotationMatrix::convertToEulerAngles (Euler& e) const
{
    e.values[Euler::Phi] = atan2f (values[R32], values[R33]);
    e.values[Euler::Theta] =  -atanf (values[R31] / sqrt (1 - values[R31]*values[R31]));
    e.values[Euler::Psi] = atan2f (values[R21], values[R11]);
}


