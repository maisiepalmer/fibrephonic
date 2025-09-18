//=======================================================================
/**
 * Copyright (c) 2010 - 2018 Mi.mu Gloves Limited
 * 	
 * All Rights Reserved.
 *
 * http://www.mimugloves.com
 */
//======================================================================

#include "GlobalFunctions.h"


namespace Glover
{

#if JUCE_WINDOWS
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <iostream>
#include <vector>

    // Link with Iphlpapi.lib
#pragma comment (lib, "IPHLPAPI.lib")
    bool findAllActiveIPAddresses (Array<IPAddress>& results)
    {
       
        const int BufferSize = 15000;
        const int ReadAttempts = 3;

        /* Declare and initialize variables */

        DWORD dwRetVal = 0;

        LPVOID lpMsgBuf = NULL;

        PIP_ADAPTER_ADDRESSES pAddresses = NULL;
        ULONG outBufLen = BufferSize;

        PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;

        std::vector<unsigned char> buffer(BufferSize);

        int attempts = 0;

        do
        {
            pAddresses = (IP_ADAPTER_ADDRESSES*)buffer.data();
            if (pAddresses == NULL)
            {
                DBG ("Vector allocation failed for IP_ADAPTER_ADDRESSES struct");
                return false;
            }

            dwRetVal = GetAdaptersAddresses (AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);

            attempts++;

            if (dwRetVal == ERROR_BUFFER_OVERFLOW)
            {
                pAddresses = NULL;
                buffer.resize ((attempts + 1L) * BufferSize, 0);
            }

        } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (attempts < ReadAttempts));

        if (dwRetVal == NO_ERROR)
        {
            // If successful, output some information from the data we received
            pCurrAddresses = pAddresses;
            while (pCurrAddresses)
            {
                //DBG ("\tFriendly name: " << pCurrAddresses->FriendlyName);
                //DBG("\tOperStatus: " << pCurrAddresses->OperStatus);

                if (pCurrAddresses->OperStatus == IfOperStatusUp)
                {
                    pUnicast = pCurrAddresses->FirstUnicastAddress;
                    if (pUnicast != NULL) {
                        for (int i = 0; pUnicast != NULL; i++)
                        {
                            SOCKADDR* pSockAddr = pUnicast->Address.lpSockaddr;
                            switch (pSockAddr->sa_family)
                            {
                                case AF_INET:
                                {
                                    sockaddr_in* pInAddr = (sockaddr_in*)pSockAddr;
                                    //unsigned char* array = (unsigned char*)&pInAddr->sin_addr.s_addr;
                                    //DBG ("\tIP Address: " << (int)array[0] << "."
                                    //                      << (int)array[1] << "."
                                    //                      << (int)array[2] << "."
                                    //                      << (int)array[3]);
                                    results.addIfNotAlreadyThere (IPAddress ((uint8*)&pInAddr->sin_addr.s_addr, false));

                                    break;
                                }
                            }

                            pUnicast = pUnicast->Next;
                        }
                    }
                    else
                    {
                        DBG("\tNo Unicast Addresses\n");
                    }
                }
                pCurrAddresses = pCurrAddresses->Next;
            }
        }
        else
        {
            DBG ("Call to GetAdaptersAddresses failed with error: " << dwRetVal);
            if (dwRetVal == ERROR_NO_DATA)
                DBG ("\tNo addresses were found for the requested parameters\n");
            else 
            {

                if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    // Default language
                    (LPTSTR)&lpMsgBuf, 0, NULL))
                {
                    DBG ("\tError: " << (wchar_t*)lpMsgBuf);
                    LocalFree(lpMsgBuf);

                    return false;
                }
            }
        }
    }
#endif
    IPAddress getLocalIpAddress()
    {
        Array<IPAddress> results;
#ifdef JUCE_MAC
        IPAddress::findAllAddresses (results);
#elif JUCE_WINDOWS
        findAllActiveIPAddresses (results);
#endif
        for (int i = 0; i < results.size(); i++)
        {
            if (results.getReference (i).toString().startsWith ("192.") 
                || results.getReference (i).toString().startsWith ("10.")
                || results.getReference(i).toString().startsWith("169."))
                return results.getReference (i);
        }
        
        return IPAddress();
    }
    
    bool addressesHaveMatchingFirstThreeBytes (const IPAddress& ip1, const IPAddress& ip2)
    {
        return ip1.address[0] == ip2.address[0] && ip1.address[1] == ip2.address[1] && ip1.address[2] == ip2.address[2];
    }
    
    IPAddress getLocalIpAddressMatchingRemote (const IPAddress& remoteAddress)
    {
        Array<IPAddress> results;
#ifdef JUCE_MAC
        IPAddress::findAllAddresses (results);
#elif JUCE_WINDOWS
        findAllActiveIPAddresses (results);
#endif
        for (int i = 0; i < results.size(); i++)
        {
            if (addressesHaveMatchingFirstThreeBytes (results[i], remoteAddress))
                return results.getReference (i);
        }
        
        return IPAddress();
    }

    std::vector<IPAddress> getAllLocalIpAddresses()
    {
        std::vector<IPAddress> ipAddresses;
        Array<IPAddress> results;
#ifdef JUCE_MAC
        IPAddress::findAllAddresses (results);
#elif JUCE_WINDOWS
        findAllActiveIPAddresses (results);
#endif
        for (auto& ip : results)
        {
            if (ip != IPAddress::local())
                ipAddresses.push_back (ip);
        }
        return ipAddresses;
    }
    
    bool isValidIP4Address (const String& ipAddress)
    {
        if (ipAddress.containsOnly ("0123456789."))
        {
            auto tokens = StringArray::fromTokens (ipAddress, ".", "");
            
            if (tokens.size() == 4)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (tokens[i] == String())
                        return false;
                    
                    if (! tokens[i].containsOnly ("0123456789"))
                        return false;
                    
                    int value = tokens[i].getIntValue();
                    
                    if (! isPositiveAndBelow (value, 256))
                        return false;
                }
                
                return true;
            }
        }
        
        return false;
    }
    
    int getInertialHandSampleRate()
    {
        return 128;
    }
    
    bool isValidInteger (const String& s)
    {
        if (s.contains ("-") && s.length() > 1 && s.substring (1).containsOnly ("0123456789"))
            return true;
        else if (s.containsOnly ("0123456789"))
            return true;
        else
            return false;
    }
    
    bool isValidFloat (const String s)
    {
        if (s.isEmpty())
            return false;
        
        // check that if there is a minus sign, it is the first character only, else return false
        if (s.containsChar ('-') && s.lastIndexOfChar ('-') != 0)
            return false;
                
        return s.containsOnly ("0123456789.-") && s.containsChar ('.') && s.indexOfChar ('.') == s.lastIndexOfChar ('.');
    }
}
