//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#include "CircularBuffer.h"

//==============================================================================
#ifdef GLOVER_UNIT_TESTS
class CircularBufferUnitTests  : public UnitTest
{
public:
    
    //==============================================================================
    CircularBufferUnitTests()
    :   UnitTest ("CircularBuffer testing")
    {
        
    }
    
    //==============================================================================
    void runTest() override
    {
        //------------------------------------------------------------------
        beginTest ("Initialisation");
        
        CircularBuffer<float, 6> buffer;
        
        expect (buffer.size() == 6);
        
        for (int i = 0; i < buffer.size(); i++)
            expectEquals (buffer[i], 0.f);
        
        //------------------------------------------------------------------
        beginTest ("Adding Values to Back");
        
        buffer.addToBack (24.f);
        checkContains (buffer, {0.f, 0.f, 0.f, 0.f, 0.f, 24.f});
        expectEquals (buffer.getSum(), 24.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), 0.f);
        expectEquals (buffer.getMaxInRange (0, 2), 0.f);
        expectEquals (buffer.getMean(), 4.);
        
        buffer.addToBack (20.f);
        checkContains (buffer, {0.f, 0.f, 0.f, 0.f, 24.f, 20.f});
        expectEquals (buffer.getSum(), 44.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), 0.f);
        expectEquals (buffer.getMaxInRange (0, 2), 0.f);
        expectWithinAbsoluteError (buffer.getMean(), 44. / 6., 0.00001);
        
        buffer.addToBack (5.f);
        checkContains (buffer, {0.f, 0.f, 0.f, 24.f, 20.f, 5.f});
        expectEquals (buffer.getSum(), 49.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), 0.f);
        expectEquals (buffer.getMaxInRange (0, 2), 0.f);
        expectWithinAbsoluteError (buffer.getMean(), 49. / 6., 0.00001);
        
        buffer.addToBack (-10.f);
        checkContains (buffer, {0.f, 0.f, 24.f, 20.f, 5.f, -10.f});
        expectEquals (buffer.getSum(), 39.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 24.f);
        expectWithinAbsoluteError (buffer.getMean(), 39. / 6., 0.00001);
        
        buffer.addToBack (15.f);
        checkContains (buffer, {0.f, 24.f, 20.f, 5.f, -10.f, 15.f});
        expectEquals (buffer.getSum(), 54.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 24.f);
        expectWithinAbsoluteError (buffer.getMean(), 54. / 6., 0.00001);
        
        buffer.addToBack (2.f);
        checkContains (buffer, {24.f, 20.f, 5.f, -10.f, 15.f, 2.f});
        expectEquals (buffer.getSum(), 56.f);
        expectEquals (buffer.getMaxValue(), 24.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 24.f);
        expectWithinAbsoluteError (buffer.getMean(), 56. / 6., 0.00001);
        
        buffer.addToBack (12.f);
        checkContains (buffer, {20.f, 5.f, -10.f, 15.f, 2.f, 12.f});
        expectEquals (buffer.getSum(), 44.f);
        expectEquals (buffer.getMaxValue(), 20.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 20.f);
        expectWithinAbsoluteError (buffer.getMean(), 44. / 6., 0.00001);
        
        buffer.addToBack (1.f);
        checkContains (buffer, {5.f, -10.f, 15.f, 2.f, 12.f, 1.f});
        expectEquals (buffer.getSum(), 25.f);
        expectEquals (buffer.getMaxValue(), 15.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 15.f);
        expectWithinAbsoluteError (buffer.getMean(), 25. / 6., 0.00001);
        
        //------------------------------------------------------------------
        beginTest ("Adding Values to Front");
        
        buffer.addToFront (100.f);
        checkContains (buffer, {100.f, 5.f, -10.f, 15.f, 2.f, 12.f});
        expectEquals (buffer.getSum(), 124.f);
        expectEquals (buffer.getMaxValue(), 100.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 100.f);
        expectWithinAbsoluteError (buffer.getMean(), 124. / 6., 0.00001);
        
        buffer.addToFront (25.f);
        checkContains (buffer, {25.f, 100.f, 5.f, -10.f, 15.f, 2.f});
        expectEquals (buffer.getSum(), 137.f);
        expectEquals (buffer.getMaxValue(), 100.f);
        expectEquals (buffer.getMinValue(), -10.f);
        expectEquals (buffer.getMaxInRange (0, 2), 100.f);
        expectWithinAbsoluteError (buffer.getMean(), 137. / 6., 0.00001);
        
        //------------------------------------------------------------------
        beginTest ("Fill With and Clear");
        
        buffer.fillWith (23.f);
        checkContains (buffer, {23.f, 23.f, 23.f, 23.f, 23.f, 23.f});
        expectEquals (buffer.getSum(), 138.f);
        expectEquals (buffer.getMaxValue(), 23.f);
        expectEquals (buffer.getMinValue(), 23.f);
        expectEquals (buffer.getMaxInRange (0, 2), 23.f);
        expectWithinAbsoluteError (buffer.getMean(), 23., 0.00001);
        
        buffer.clear();
        checkContains (buffer, {0.f, 0.f, 0.f, 0.f, 0.f, 0.f});
        expectEquals (buffer.getSum(), 0.f);
        expectEquals (buffer.getMaxValue(), 0.f);
        expectEquals (buffer.getMinValue(), 0.f);
        expectEquals (buffer.getMaxInRange (0, 2), 0.f);
        expectEquals (buffer.getMean(), 0.);
    }
    
    void checkContains (CircularBuffer<float, 6>& buffer, const std::array<float, 6>& v)
    {
        for (int i = 0; i < buffer.size(); i++)
            expectEquals (buffer[i], v[i]);
    }
};

static CircularBufferUnitTests circularBufferUnitTests;

#endif
