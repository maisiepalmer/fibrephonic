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

#include "../JuceLibraryCode/JuceHeader.h"

namespace Glover
{
    template <typename T>
    static int indexOf (const std::vector<T>& v, T& element)
    {
        auto it = std::find (v.begin(), v.end(), element);
        size_t index = -1;
        if (it != v.end())
            index = std::distance (v.begin(), it);
        
        return static_cast<int> (index);
    }
        
    int getInertialHandSampleRate();
    IPAddress getLocalIpAddress();
    std::vector<IPAddress> getAllLocalIpAddresses();
    IPAddress getLocalIpAddressMatchingRemote (const IPAddress& remote);
    bool isValidIP4Address (const String& ipAddress);
    
    //==============================================================
    bool isValidInteger (const String& s);
    bool isValidFloat (const String s);
}
