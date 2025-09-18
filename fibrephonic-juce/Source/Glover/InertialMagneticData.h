//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#ifndef H_INERTIALMAGNETICDATA
#define H_INERTIALMAGNETICDATA

#include <JuceHeader.h>
//#warning "Refactor this to use pointers to fixed size arrays, type checking is very loose currently"

/**
 Small class with public memebers for quaternion data.
 Quartenions are 4 elements. This class will also do euler and rotaion matrix calculations too
 */
class InertialMagneticData
{
public:
    /**
     enum containing the array sizes for the arrays
     */
    enum DataSizes
    {
        GyroscopeSize = 3,
        AccelerometerSize = 3,
        MagnetometerSize = 3,
        BarometerSize = 1,
        
        InertialMagneticDataSize = GyroscopeSize + AccelerometerSize + MagnetometerSize + BarometerSize,
        
        X = 0, Y, Z
    };
    
    /**
     
     */
    InertialMagneticData ()
    {
        std::fill (gyroscope, gyroscope + GyroscopeSize, 0.f);
        std::fill (accelerometer, accelerometer + AccelerometerSize, 0.f);
        std::fill (magnetometer, magnetometer + MagnetometerSize, 0.f);
    }
    
    /**
     Constructor
     */
    InertialMagneticData (const float *gyroscope_, const float *accelerometer_, const float *magnetometer_)
    {
        std::memcpy (gyroscope, gyroscope_, GyroscopeSize*sizeof (float));
        std::memcpy (accelerometer, accelerometer_, AccelerometerSize*sizeof (float));
        std::memcpy (magnetometer, magnetometer_, MagnetometerSize*sizeof (float));
    }
    
    InertialMagneticData (float gyroX,  float gyroY,  float gyroZ,
                          float accelX, float accelY, float accelZ,
                          float magX,   float magY,   float magZ)
    {
        gyroscope[X] = gyroX; gyroscope[Y] = gyroY; gyroscope[Z] = gyroZ;
        accelerometer[X] = accelX; accelerometer[Y] = accelY; accelerometer[Z] = accelZ;
        magnetometer[X] = magX; magnetometer[Y] = magY; magnetometer[Z] = magZ;
    }
    
    /**
     Constructor
     */
    InertialMagneticData (const float *data)
    {
        std::memcpy (gyroscope, data, GyroscopeSize * sizeof (float));
        std::memcpy (accelerometer, data + GyroscopeSize, AccelerometerSize * sizeof (float));
        std::memcpy (magnetometer, data + GyroscopeSize + AccelerometerSize, MagnetometerSize * sizeof (float));
    }
    
    float gyroscope[GyroscopeSize];
    float accelerometer[AccelerometerSize];
    float magnetometer[MagnetometerSize];
private:
    JUCE_LEAK_DETECTOR (InertialMagneticData);
};


#endif /* H_INERTIALMAGNETICDATA */
