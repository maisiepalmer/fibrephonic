//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#ifndef H_ORIENTATIONPROCESSOR
#define H_ORIENTATIONPROCESSOR

#include "PeakDetector.h"
#include "CircularBuffer.h"
#include "DrumDetector.h"
#include "Orientation.h"
#include "InertialMagneticData.h"
#include <atomic>
#include "Definitions.h"
#include "DirectionProcessor.h"

/** Orientation processor class */
class OrientationProcessor
{ 
public:
    
    /** Axes Enum */
    enum Axis
    {
        Xaxis = 0,
        Yaxis = 1,
        Zaxis = 2,
        
        NumAxes
    };
    
    /** Enum for state changes */
    enum State
    {
		NotRunning=0,
		Running,
		
		NumStates
	};
    
	/** Constructor */
    OrientationProcessor (float drumThreshold = 50.f, float wristFlickThreshold = 200.f, float slapThreshold = 50.f);
    
    /** Destructor */
    ~OrientationProcessor() {}
    
    void setInversionForRightHandedWristFlicks (bool invert)
    {
        // horrid hacks for NGIMU wrist flick...
        rightHandInversionFactor = invert ? -1.f : 1.f;
        wristFlickScaleFactor = invert ? 0.25f : 1.f;
    }
    
    /** Listener class for postures */
    class Listener
    {
    public:
        virtual ~Listener() {}
        /** Called when one of the hands falls into one of the envelopes north, south, east, west, up, down*/
        virtual void newDirection (const OrientationProcessor* source, const Direction direction) = 0;
        virtual void newSegment (const OrientationProcessor* source, const Segment segment) = 0;
        virtual void orientationEvent (const OrientationProcessor* source, OrientationProcessor::Axis axis, float magnitude) = 0;
        /** Called whenever a gyroscope packet is recieved indicating an estimation of the rotaional displacement 
         since the last sample*/
        virtual void gyroscopeDisplacement(const OrientationProcessor* source, const float gyroDelta[OrientationProcessor::NumAxes]) = 0;
    };
    
    /** Adds a listener to recieve Orientation data */
	void addListener(OrientationProcessor::Listener *listenerToAdd);
	
    /** Removes a listener */
	void removeListener(OrientationProcessor::Listener *listenerToRemove);
    
    void orientationRecieved (const RotationMatrix* const rotData);
    void inertialMagneticRecieved (const InertialMagneticData* const sensor);
    
    void setHand (int h) { hand = h; }

private:
    Direction getDirection (const RotationMatrix* rotationMatrix, const OrientationProcessor::Axis axis, const float coneAngleDegrees);
    
    void detectOrientationEvents (const InertialMagneticData* const inertData);         //peak detect
    void calculateAccelerometerDisplacement (const InertialMagneticData* inertData);    //deltaValues for accelerometer
    float norm (const float* const accelarometerVec);                                   //norm of vector
    void calculateGyroscopeDisplacement (const InertialMagneticData* inertData);        //deltaValues for gyroscope
    
    Atomic<int> sensorPosition;
    Direction currentDirection;
    Segment currentSegment;
        
    DrumDetector drumDetector;          //Z
    DrumDetector wristFlickDetector;    //X
    DrumDetector slapDetector;          //Y
    
    OwnedArray<PeakDetector> gyroscopePeakDetectors;
    
    CriticalSection listenerLock;
    ListenerList<Listener> listeners;
    
    DirectionProcessor directionProcessor;
    
    std::atomic<int> hand;
    
    float rightHandInversionFactor {1.f};
    float wristFlickScaleFactor {1.f};
};

#endif //H_ORIENTATIONPROCESSOR
