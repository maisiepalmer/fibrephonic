//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#ifndef H_DRUMDETECTOR
#define H_DRUMDETECTOR

#include <JuceHeader.h>

#include "CircularBuffer.h"
#include "GlobalFunctions.h"

/**
 Class to examine a stream of floats for peaks with appropriate debouncing and so fourth. 
 
 (in the following description ascent and decent can be swapped and this class will still work i.e. @thresh_ can 
 be both positive and negative)
 This class uses a specialied heuristic for identifying fluctuations in the inerial measurement 
 data indicative of a drum gesture. The method works by observing a descent in the Z axis gyro reading followed 
 by subsequent ascent. The descent is identified when the value crosses the @onThresh @thresh_ value. When the 
 gyro value then ascends above the @offThresh the drum is triggered. 
 
 The offThresh moves to remain 200 degrees per second below the peak of the descent and is then reset to the @onThresh 
 when the drum is triggered.
 
 When the drum is triggered a 20ms count down timer is initalised - when this timer reaches zero the detector 
 is armed again. If the threshold is retriggered within this 20ms (likely due to recoil readings of the gyro, 
 as observed in plots) the timer is reset for another 20ms. This produces a stepped varibale length count down 
 timer which produces a debouncing mechanism which extends its debounce time only when necessary.
 */
class DrumDetector
{
public:
    /**
     Constructor
     */
    DrumDetector (const float thresh_, const float sampleRate_ = Glover::getInertialHandSampleRate())
     :  onThresh (thresh_),
        offThresh (thresh_),
        beatPending(false),
        /*beatInhibit(false),*/
        countDownTimer(0),
        sampleRate (sampleRate_)
    {
        CircularBuffer256<float> buf;
    }
    
    /**
     Destructor
     */
    ~DrumDetector()
    {
        
    }
    
    /**
     Takes an input sample and when a peak is detected returns true.
     */
    float compute (const float input)
    {
        if (countDownTimer > 0) 
            --countDownTimer;
        
//        if (beatInhibit == false) 
//        {
            if ( isThreshExceeded(input) )
            { 
                if ( countDownTimer == 0 ) 
                {
                    beatPending = true;
                    buffer.add(input);
                }
                else 
                {
                    countDownTimer = roundToInt (0.02 * sampleRate);
                }
            }
            else if(beatPending == true)
            {
                float velocity = getMaxMagnitude();
                beatPending = false;
                offThresh = onThresh;
                buffer.clear();
  //              beatInhibit = true;
                countDownTimer = roundToInt (0.02 * sampleRate);
                return velocity;
            }
//        }
//        else if ( crossZero(input) )
//        {
//            std::cout << "inhibit off: " << input << "\n";
//            beatInhibit = false;
//        }
        
        return 0.f;
    }
    	
private:
    
    bool isThreshExceeded (float input)
    {
        if (beatPending == false)
        {
            return onThresh > 0.f ? input > onThresh : input < onThresh;
        }
        else
        {
            if (onThresh > 0.f)
            {
                if (input >  offThresh + 200) 
                {
                    offThresh = input - 200;
                }
                return input > offThresh;
            }
            else 
            {
                if (input < offThresh - 200) 
                {
                    offThresh = input + 200;
                }
                return input < offThresh;
            }

        }
    }
    
    float getMaxMagnitude()
    {
        unsigned char pos;
        float magnitude;
        
        if (onThresh > 0.f ) 
        {
             buffer.getMaxAndPosition (&magnitude, &pos);
        }
        else 
        {
             buffer.getMinAndPosition (&magnitude, &pos);
        }
        
        return fabsf (magnitude);
    }
    
    bool crossZero(float input)
    {
        return onThresh > 0.f ? input < 0.f : input > 0.f;
    }
    
    CircularBuffer256<float> buffer;
    float onThresh;
    float offThresh;
    bool beatPending;
//    bool beatInhibit; //debounce
    int countDownTimer;
    const float sampleRate;
};


#endif //H_DRUMDETECTOR
