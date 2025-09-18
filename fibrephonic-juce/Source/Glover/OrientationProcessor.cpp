//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#include "OrientationProcessor.h"
#include "GlobalFunctions.h"

OrientationProcessor::OrientationProcessor (float drumThreshold, float wristFlickThreshold, float slapThreshold) :  currentDirection (NullDirection),
                                                //accelarometerPeakDetector (3.5f),
                                                drumDetector (drumThreshold),
                                                wristFlickDetector (wristFlickThreshold),
                                                slapDetector (slapThreshold),
                                                hand (Hand::Left)
{
    gyroscopePeakDetectors.add (new PeakDetector(800.f, 250));       //Xaxis
    gyroscopePeakDetectors.add (new PeakDetector(300.f,  250));       //Yaxis
    gyroscopePeakDetectors.add (new PeakDetector(300.f,  250));       //Zaxis
}


void OrientationProcessor::addListener(OrientationProcessor::Listener *listenerToAdd)
{
    if (listenerToAdd != nullptr)
    {
        ScopedLock sl(listenerLock);
        listeners.add(listenerToAdd);
    }
}

void OrientationProcessor::removeListener(OrientationProcessor::Listener *listenerToRemove)
{
    if (listenerToRemove != nullptr)
    {
        ScopedLock sl(listenerLock);
        listeners.remove(listenerToRemove);
    }
}

//put this into a class
void OrientationProcessor::orientationRecieved(const RotationMatrix* const rotData)
{
    //Direction newDirectionReading = getDirection (rotData, Xaxis, 40);
    Direction newDirectionReading = static_cast<Direction> (directionProcessor.getDirection (rotData, 40));
    
    if (newDirectionReading != NullDirection && newDirectionReading != currentDirection)
    {
        currentDirection = newDirectionReading;
        ScopedLock sl(listenerLock);
        listeners.call(&Listener::newDirection, this, currentDirection);
    }
    
    Segment segment = directionProcessor.getSegment (rotData, 20.f);
    
    if (segment != Segment::NullSegment && segment != currentSegment)
    {
        currentSegment = segment;
        ScopedLock sl (listenerLock);
        listeners.call (&Listener::newSegment, this, currentSegment);
        //DBG (DirectionProcessor::getSegmentName (currentSegment));
    }
}

Direction OrientationProcessor::getDirection(const RotationMatrix* rotation, const OrientationProcessor::Axis axis, const float coneAngleDegrees)
{
    float theta = coneAngleDegrees / 57.2957795130823f; //deg to rad
    float sintheta = std::sin(theta);
    float costheta = std::cos(theta);
    
    if (rotation->values[0+axis] > (1.f - costheta) && rotation->values[0+axis] < (1.f + costheta) &&
        rotation->values[3+axis] > (0.f - sintheta) && rotation->values[3+axis] < (0.f + sintheta) &&
        rotation->values[6+axis] > (0.f - sintheta) && rotation->values[6+axis] < (0.f + sintheta))
    {
        return posX;
    }
    else if (rotation->values[0+axis] > (-1.f - costheta) && rotation->values[0+axis] < (-1.f + costheta) &&
             rotation->values[3+axis] > (0.f - sintheta) && rotation->values[3+axis] < (0.f + sintheta) &&
             rotation->values[6+axis] > (0.f - sintheta) && rotation->values[6+axis] < (0.f + sintheta))
    {
        return negX;
    }
    else if (rotation->values[0+axis] > (0.f - sintheta) && rotation->values[0+axis] < (0.f + sintheta) &&
             rotation->values[3+axis] > (1.f - costheta) && rotation->values[3+axis] < (1.f + costheta) &&
             rotation->values[6+axis] > (0.f - sintheta) && rotation->values[6+axis] < (0.f + sintheta))
    {
        return posY;
    }
    else if (rotation->values[0+axis] > (0.f - sintheta) && rotation->values[0+axis] < (0.f + sintheta) &&
             rotation->values[3+axis] > (-1.f - costheta) && rotation->values[3+axis] < (-1.f + costheta) &&
             rotation->values[6+axis] > (0.f - sintheta) && rotation->values[6+axis] < (0.f + sintheta))
    {
        return negY;
    }
    else if (rotation->values[0+axis] > (0.f - sintheta) && rotation->values[0+axis] < (0.f + sintheta) &&
             rotation->values[3+axis] > (0.f - sintheta) && rotation->values[3+axis] < (0.f + sintheta) &&
             rotation->values[6+axis] > (1.f - costheta) && rotation->values[6+axis] < (1.f + costheta))
    {
        return posZ;
    }
    else if (rotation->values[0+axis] > (0.f - sintheta) && rotation->values[0+axis] < (0.f + sintheta) &&
             rotation->values[3+axis] > (0.f - sintheta) && rotation->values[3+axis] < (0.f + sintheta) &&
             rotation->values[6+axis] > (-1.f - costheta) && rotation->values[6+axis] < (-1.f + costheta))
    {
        return negZ;
    }
    else
    {
        return NullDirection;
    }
}

void OrientationProcessor::inertialMagneticRecieved (const InertialMagneticData* const sensor)
{
    detectOrientationEvents (sensor);
//    calculateAccelerometerDisplacement (sensor);
//    detectGyroscopePeaks (sensor);
//    calculateGyroscopeDisplacement (sensor);
}

void OrientationProcessor::detectOrientationEvents (const InertialMagneticData* const inertData)
{
#define gyroMagScale(val) 1 - 1/expf (val/2000.f * 6)
    {//WristFlickDetect
        //DBG ("X: " << inertData->gyroscope[Xaxis]);
        float magnitude = 0.f;
        if (hand == Hand::Left)
            magnitude = wristFlickDetector.compute (inertData->gyroscope[Xaxis]);
        else if (hand == Hand::Right)
            magnitude = wristFlickDetector.compute (rightHandInversionFactor * -inertData->gyroscope[Xaxis]);
        
        magnitude *= wristFlickScaleFactor;
        
        if (hand == Hand::Right && magnitude > 0)
            DBG ("MAG: " << magnitude << " | " << gyroMagScale (magnitude));
        
        if (magnitude != 0.f)
        {
            ScopedLock sl(listenerLock);       //max value is 0.9933 and 0.5 -> 0.9179
            listeners.call(&Listener::orientationEvent, this, Xaxis, gyroMagScale (magnitude));
        }
    }
    
    {//DrumhitDetect
        float magnitude = 0.f;
        if (hand == Hand::Left)
            magnitude = drumDetector.compute (-inertData->gyroscope[Zaxis]);
        else if (hand == Hand::Right)
            magnitude = drumDetector.compute (inertData->gyroscope[Zaxis]);
        
        if (magnitude != 0.f)
        {
            ScopedLock sl(listenerLock);       //max value is 0.9933 and 0.5 -> 0.9179
            listeners.call(&Listener::orientationEvent, this, Zaxis, gyroMagScale (magnitude));
        }
    }

    {//SlapDetect
        float magnitude = 0.f;
        
        if (hand == Hand::Left)
            magnitude = slapDetector.compute (inertData->gyroscope[Yaxis]);
        else if (hand == Hand::Right)
            magnitude = slapDetector.compute (rightHandInversionFactor * inertData->gyroscope[Yaxis]);
        
        if (magnitude != 0.f)
        {
            ScopedLock sl(listenerLock);       //max value is 0.9933 and 0.5 -> 0.9179
            listeners.call(&Listener::orientationEvent, this, Yaxis, gyroMagScale (magnitude));
        }
    }
}

//Private
float OrientationProcessor::norm(const float* const accelarometerVec)
{
    return sqrt(accelarometerVec[Xaxis] * accelarometerVec[Xaxis] +
                accelarometerVec[Yaxis] * accelarometerVec[Yaxis] +
                accelarometerVec[Zaxis] * accelarometerVec[Zaxis]);
}

void OrientationProcessor::calculateGyroscopeDisplacement(const InertialMagneticData* inertData)
{
    float gyroDelta[NumAxes];
    gyroDelta[Xaxis] = inertData->gyroscope[Xaxis] * 1.f/Glover::getInertialHandSampleRate();
    gyroDelta[Yaxis] = inertData->gyroscope[Yaxis] * 1.f/Glover::getInertialHandSampleRate();
    gyroDelta[Zaxis] = inertData->gyroscope[Zaxis] * 1.f/Glover::getInertialHandSampleRate();
    
    ScopedLock sl(listenerLock);
    listeners.call(&Listener::gyroscopeDisplacement, this, gyroDelta);
}

void OrientationProcessor::calculateAccelerometerDisplacement(const InertialMagneticData* inertData)
{
    
}

