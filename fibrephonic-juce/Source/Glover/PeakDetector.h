//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#ifndef H_PEAKDETECTOR
#define H_PEAKDETECTOR

#include <JuceHeader.h>

/**
 Class to examing a stream of floats for peaks with appropriate debouncing and so fourth.
 */
class PeakDetector
{
public:
    /**
     Constructor
     */
    PeakDetector(const float thresh_, int debounceTime_ = 100) : debounce (0), debounceTime(debounceTime_), thresh (std::fabs(thresh_))
    {
        
    }
    
    /**
     Destructor
     */
    ~PeakDetector()
    {
        
    }
    
    /**
     Takes an input sample and when a peak is detected returns true.
     */
    bool compute (const float input)
    {
        if (crossThresh(input) > 0.f && debounce == 0)
        {
            debounce = Time::getMillisecondCounterHiRes();
            return true;
        }
        else if(debounce != 0)
        {
            long long elapsed = Time::getMillisecondCounterHiRes() - debounce;
            if (elapsed > debounceTime) 
            {
                debounce = 0;
            }
        }
        return false;
    }
    	
private:
    
    bool crossThresh(float input)
    {
        return (input > thresh || input < -thresh);
    }
    
    long long debounce;
    long long debounceTime;
    float thresh;
};


#endif //H_PEAKDETECTOR
