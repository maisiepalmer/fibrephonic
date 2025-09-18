//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#pragma once

#include <JuceHeader.h>
#include <array>
#include <limits>
#include <algorithm>

//======================================================================
/** A fast 256 element circular buffer that overflows a uchar to avoid end of buffer checks */
template <class Type>
class CircularBuffer256
{
public:
    enum Constants 
    {
        Size = 256
    };
    
    /** Constructor */
    CircularBuffer256() : bufferPosition (0)
    {
        clear();
    }
    
    /** Destructor */
    ~CircularBuffer256() {}
    
    /** Clear */
    void clear()
    {
        buffer.fill ((Type)0);
    }
    
    /** writes a value to the circular buffer */
    void add (Type sample)
    {
        buffer[bufferPosition++] = sample;
    }
    
    Type getMaxValue() const
    {
        return *std::max_element (buffer.begin(), buffer.end());
    }
    /** offsets the value by the corrent position so it can be read using a bit like a static array */
    Type getValue (int index) const
    {
        return buffer[(bufferPosition + index) % Size];
    }
    
    void getMaxAndPosition (Type* maxValue, unsigned char* index)
    {
        typename std::array<Type, Size>::iterator max = std::max_element (buffer.begin(), buffer.end());
        *maxValue = *max;
        *index = max - buffer.begin();
    }
    
    void getMinAndPosition (Type* minValue, unsigned char* index)
    {
        typename std::array<Type, Size>::iterator min = std::min_element (buffer.begin(), buffer.end());
        *minValue = *min;
        *index =  min - buffer.begin();
    }
    
private:
    std::array<Type, Size> buffer;
    unsigned char bufferPosition;
};


//======================================================================
/** A general purpose circular buffer that can be set to an arbitrary size */
template <typename T, int Size>
class CircularBuffer
{
public:
    
    //======================================================================
    /** Constructor */
    CircularBuffer()
    {
        fillWith (0);
    }
    
    CircularBuffer (const CircularBuffer &other)
    {
        for (int i = 0; i < size(); i++)
            (*this)[i] = other[i];
    }
        
    T& operator[] (int i)
    {
        int index = (bufferStart + i) % buffer.size();
        return buffer[index];
    }
    
    /** @Returns the number of elements in the buffer */
    size_t size() const
    {
        return Size;
    }
    
    /** Removes all elements from the buffer */
    void clear()
    {
        fillWith (0);
        bufferStart = 0;
    }
    
    /** Fills the buffer with a given value, up to the MaxCapacity */
    void fillWith (T value)
    {
        buffer.fill (value);
        bufferStart = 0;
    }
    
    //======================================================================
    /** Adds a single element to the back of the buffer. The buffer will
     * drop the first element from the start of the buffer to make space
     */
    void addToBack (T value)
    {
        buffer[bufferStart] = value;
        bufferStart = (bufferStart + 1) % buffer.size();
    }
    
    /** Adds a single element to the front of the buffer. The buffer will
     * drop the last element from the back of the buffer to make space
     */
    void addToFront (T value)
    {
        bufferStart = wrapToBufferRange (bufferStart - 1);
        buffer[bufferStart] = value;
    }
    
    /** @Returns the sum of the buffer */
    T getSum()
    {
        return std::accumulate (buffer.begin(), buffer.end(), 0.);
    }
    
    /** @Returns the mean of the buffer */
    double getMean()
    {
        return static_cast<double> (std::accumulate (buffer.begin(), buffer.end(), 0.)) / static_cast<double> (buffer.size());
    }
    
    /** @Returns the maximum value in the buffer */
    T getMaxValue()
    {
        return *std::max_element (buffer.begin(), buffer.end());
    }
    
    /** @Returns the minimum value in the buffer */
    T getMinValue()
    {
        return *std::min_element (buffer.begin(), buffer.end());
    }
    
    /** @Returns the maximum value in the buffer within the inclusive range of [startIndex, endIndex] */
    T getMaxInRange (int startIndex, int endIndex)
    {
        T maxValue = 0;
        
        for (int i = startIndex; i <= endIndex; i++)
        {
            T value = (*this)[i];
            if (value > maxValue)
                maxValue = value;
        }
        
        return maxValue;
    }
            
private:
    
    int wrapToBufferRange (int v)
    {
        while (v < 0)
            v += buffer.size();
        
        while (v >= buffer.size())
            v -= buffer.size();
        
        return v;
    }
    
    int bufferStart = 0;
    std::array<T, Size> buffer;
};
