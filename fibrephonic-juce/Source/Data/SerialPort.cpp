/*
 *  tlib_SerialPort.cpp
 *  Created by Tom Mitchell
 *
 */

#include "SerialPort.h"

#include <cassert>

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#else
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#endif

//#warning refactor to return a vector of std::strings
std::vector<std::string> SerialPort::getDevicePathList()
{
    std::vector<std::string> devicePathList;
    devicePathList.clear();
	#if defined(_WIN64) || defined(_WIN32)
	//This uses the NT kernel QueryDosDevice API adapted from http://www.naughter.com/enumser.html
    OSVERSIONINFOEX osvi;
    memset (&osvi, 0, sizeof (osvi));
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    osvi.dwPlatformId = VER_PLATFORM_WIN32_NT;
    ULONGLONG dwlConditionMask = 0;
    VER_SET_CONDITION (dwlConditionMask, VER_PLATFORMID, VER_EQUAL);
    if (VerifyVersionInfo (&osvi, VER_PLATFORMID, dwlConditionMask))
    {
        std::vector<char> buf (4096);//start with this
        int charsRead;
        int attempt = 1;
        do
        {
            charsRead = QueryDosDevice (NULL, static_cast<LPTSTR> (buf.data()), buf.size());
            if (charsRead == 0)
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_INSUFFICIENT_BUFFER)
                {
                    ++attempt;
                    if (attempt == 10) //stop infinate buffer enlargement
                        return devicePathList;
                    buf.resize (buf.size() * attempt); //Expand the buffer and loop around again 
                }
                else
                {
                    return devicePathList;
                }
            }
        }
        while (charsRead == 0);
   
        LPTSTR deviceNamesPointer = static_cast<LPTSTR> (buf.data());
        int offset = 0;
        while (deviceNamesPointer[offset] != '\0')
        {
            std::string device (&(deviceNamesPointer[offset]));
            if (auto index = device.find_first_of ("0123456789"); index != std::string::npos)
            {
                auto result = std::stoi (device.substr (index));
                if (device.find ("COM", 0) == 0 && result > 0)
                    devicePathList.push_back(device);
            }
            //Go to next device name
            offset += device.length() + 1;
        }    
    }
	#else
    io_iterator_t serialPortIterator;
    
    CFMutableDictionaryRef classesToMatch;
    
    classesToMatch = IOServiceMatching (kIOSerialBSDServiceValue);
    if (classesToMatch == 0)
        return devicePathList;
    
    CFDictionarySetValue (classesToMatch, CFSTR (kIOSerialBSDTypeKey), CFSTR (kIOSerialBSDAllTypes));
    
    kern_return_t kernResult; 
    kernResult = IOServiceGetMatchingServices (kIOMasterPortDefault, classesToMatch, &serialPortIterator);    
    if (kernResult != KERN_SUCCESS)
        return devicePathList;
    
    io_object_t	deviceService;
    
    // Loop untill we have all the devices
    while ((deviceService = IOIteratorNext (serialPortIterator)))
    {
		// Get the callout device's path (/dev/cu.xxxxx).         
		CFTypeRef bsdPathAsCFString = IORegistryEntryCreateCFProperty ( deviceService,
                                                                       CFSTR(kIOCalloutDeviceKey),
                                                                       kCFAllocatorDefault, 0);
        if (bsdPathAsCFString == NULL)
            return devicePathList;
        
        Boolean result;
        char bsdPath[MAXPATHLEN] = {'\0'};
        
        // Convert the path from a CFString to a C (NUL-terminated) string for use with the POSIX open() call.
        result = CFStringGetCString ((CFStringRef)bsdPathAsCFString,
                                     bsdPath,
                                     MAXPATHLEN, 
                                     kCFStringEncodingUTF8);
        CFRelease (bsdPathAsCFString);
        
        if (result == false)
            return devicePathList;
        
        devicePathList.push_back (bsdPath);
        //printf ("Device found with BSD path: %s\n", bsdPath);
        
        // Release the io_service_t now that we are done with it.
		(void) IOObjectRelease (deviceService);
    }
    
    IOObjectRelease(serialPortIterator);	// Release the iterator.
	#endif

    return devicePathList;
}

SerialPort::SerialPort()
{
    thread = std::thread (&SerialPort::run, this);
}

SerialPort::~SerialPort()
{
    //if any ports are open then close them!
    closeAllPorts();
    threadShouldExit = true;
    thread.join();
}

bool SerialPort::openPort (const SerialPort::SerialPortSettings& settings)
{
    if (auto dpl = getDevicePathList();
        std::find (dpl.begin(), dpl.end(), settings.getPortName()) == dpl.end())
    {
        const std::string errorMessage ("Serial port error: Port does not exist: " + settings.getPortName());
        std::lock_guard<std::mutex> sl (lock);
        listeners.call ([&settings, &errorMessage](Listener& l)
                        {
                            l.serialConnectionStateChanged (settings.getPortName(), false, errorMessage);
                        });

        return false;
    }
    std::unique_lock<std::mutex> ul (lock);
    portsToOpen.push_back (settings);
    
    //wait for upto 100 ms - should only take a couple of ms!
    if (! waiter.wait_for (ul, std::chrono::milliseconds (100), [this](){return portsToOpen.size() == 0;}))
        std::cout << "Port open timeout - serial port open request not serviced within 100 ms, there is a problem with the serial port thread" << std::endl;
    
    const bool wasSuccessful = isReading();
    const std::string errorMessage (wasSuccessful ? "" : "Serial port error: failed to open port: " + settings.getPortName());

    listeners.call ([&settings, &wasSuccessful, &errorMessage](Listener& l)
                    {
                        l.serialConnectionStateChanged (settings.getPortName(), wasSuccessful, errorMessage);
                    });

    return wasSuccessful;
}

void SerialPort::openPortAsync (const SerialPort::SerialPortSettings& settings)
{
    if (auto dpl = getDevicePathList();
        std::find (dpl.begin(), dpl.end(), settings.getPortName()) == dpl.end())
    {
        //bad port name
        assert (false); //this is a bit strong
        return;
    }

    std::lock_guard<std::mutex> sl (lock);
    portsToOpen.push_back (settings);
}

void SerialPort::closePort (const std::string& nameOfPortToClose)
{
    std::lock_guard<std::mutex> sl (lock);
    //for (SerialPortSettings port : openPortSettings) //VS10 doesn't support these
	for (int i = 0; i < openPortSettings.size(); ++i)
    {
        const SerialPortSettings& port (openPortSettings[i]);
        if (port.getPortName() == nameOfPortToClose)
        {
            portsToClose.push_back (nameOfPortToClose);
            return;
        }
    }
}

void SerialPort::closeAllPorts()
{
    std::lock_guard<std::mutex> sl (lock);
    
    for (int i = 0; i < openPortSettings.size(); ++i)
    {
		SerialPortSettings& port (openPortSettings[i]);
        portsToClose.push_back (port.getPortName());
    }
}

SerialPort::ErrorStatus SerialPort::initialisePort (SerialPort::SerialPortSettings& settings)
{
    #if defined(_WIN64) || defined(_WIN32)
    std::string portName ("\\\\.\\");
    portName += settings.getPortName();
    settings.setDescriptor (CreateFile (portName.c_str(), // this strange arrangement means this works when the com number has more than one digit value e.g. COM10 and above https://support.microsoft.com/en-us/kb/115831
                            settings.getReadWriteAccess(),
                            0,
                            0,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            0));

    if (settings.getDescriptor() == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        switch (error)
        {
            case ERROR_FILE_NOT_FOUND : return SerialPort::ErrorStatus::fail ("SerialPort Error opening port: serial port does not exist"); break;
            case ERROR_ACCESS_DENIED  : return SerialPort::ErrorStatus::fail ("SerialPort Error opening port: serial port already in use"); break;
            default: return SerialPort::ErrorStatus::fail ("SerialPort Error opening port"); break;
        }
    }
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof (dcbSerialParams);

    if ( ! GetCommState (settings.getDescriptor(), &dcbSerialParams)) 
    {
        releasePort (settings);
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to open port: can't set port settings (timeouts)");
    }

    dcbSerialParams.BaudRate = settings.getBaudRate();
    dcbSerialParams.ByteSize = (char)settings.getDataBits();
    dcbSerialParams.StopBits = (char)settings.getStopBits();
    dcbSerialParams.Parity = (char)settings.getParity();
    if ( ! SetCommState (settings.getDescriptor(), &dcbSerialParams))
    {
        releasePort (settings);
        return SerialPort::ErrorStatus::fail ("SerialPortError: error setting serial port settings");
    }

    /*
        Setting ReadIntervalTimeout to MAXDWORD and both ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier 
        to zero will cause any read operations to return immediately with whatever characters are in the buffer 
        (ie, have already been received), even if there aren’t any.
    */
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = MAXDWORD;//read timeout between characters in milliseconds
    timeouts.ReadTotalTimeoutConstant = 0;  //read timeout in full
    timeouts.ReadTotalTimeoutMultiplier = 0;//additional time?
 
    timeouts.WriteTotalTimeoutConstant = 0;	
    timeouts.WriteTotalTimeoutMultiplier = 0;
 
    if( ! SetCommTimeouts (settings.getDescriptor(), &timeouts))
    {
        releasePort (settings);
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to open port: can't set port settings (timeouts)");
    }

    #else
    settings.setDescriptor (::open (settings.getPortName().c_str(), settings.getReadWriteAccess() | O_NOCTTY));
    if (settings.getDescriptor() == -1) //did this fail?
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to open port");
    //---------
    //    // don't allow multiple opens
    //    if (ioctl(descriptor, TIOCEXCL) == -1)
    //    {
    //        //DBG("SerialPort::open : ioctl error, non critical");
    //        closePort();
    //        return OpenPortError;
    //    }
    
    //
    //    if (fcntl(descriptor, F_SETFL, 0) == -1)
    //    {
    //        //DBG("SerialPort::open : fcntl error");
    //		closePort();
    //        return OpenPortError;
    //    }
    
    //    struct termios options;
    //    memset(&options, 0, sizeof(struct termios));
    //    if(tcgetattr(portDescriptor, &options) == -1)	//Get the current options for the port...
    //    {
    //        //DBG("SerialPort::open : can't get port settings to set timeouts");
    //        closePort();
    //        return OpenPortError;
    //    }
 
    struct termios options;
    memset (&options, 0, sizeof(struct termios));

    //non canocal, 0.5 second timeout, read returns as soon as any data is recieved
    cfmakeraw (&options);
    options.c_cc[VMIN] = 1;     //wait for atleast 1 character - test to see if this can be set to 0 for instant return?
    options.c_cc[VTIME] = 5;    //timeout after 5 tenths of a second
    options.c_cflag |= CREAD;   //enable reciever (daft) - maybe only set this when the Read flag is set in the settings
    options.c_cflag |= CLOCAL;  //don't monitor modem control lines - set local mode

    //baud and bits
    cfsetspeed (&options, settings.getBaudRate());
    //cfsetispeed(&options, baudRate);	//set the in speed
    //cfsetospeed(&options, baudRate);	//set the out speed

    //NUMBER OF BITS
    options.c_cflag |= settings.getDataBits();

    //NO PARITY
    //options.c_cflag &= ~PARENB;				//Turn parity off (8N1):
    //EVEN PARITY
    //options.c_cflag |= PARENB;
    //ODD PARITY
    //options.c_cflag |= PARENB;
    //options.c_cflag |= PARODD;

    //STOP
    //options.c_cflag &= ~CSTOPB;         //disable 2 stop bits

    //character size mask??
    //options.c_cflag &= ~CSIZE;

    //FLOWCONTROL XONXOFF:
    //options.c_iflag |= IXON;
    //options.c_iflag |= IXOFF;

    //FLOWCONTROL_HARDWARE:
    //options.c_cflag |= CCTS_OFLOW;
    //options.c_cflag |= CRTS_IFLOW;
    //options.c_cflag |= CRTSCTS;    // enable hardware flow control
    options.c_cflag &= ~CRTSCTS;	//dissable hardware flow control

    //options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	//recieve unprocessed raw input
    //options.c_iflag &= ~(IXON | IXOFF | IXANY);	// enabled software flow control
    //options.c_iflag &= ~(IXON | IXOFF | IXANY);	//dissable software flow control

    //set to non-standard baudRate
    if (tcsetattr (settings.getDescriptor(), TCSAFLUSH, &options) == -1)
    {
        releasePort (settings);
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to open port: can't set port settings (timeouts)");
    }

    //----------------
    //    if (tcsetattr(descriptors[1], TCSAFLUSH, &options) == -1)
    //    {
    //        //DBG("SerialPort::open : can't set port settings (timeouts)");
    //		closePort();
    //        return OpenPortError;
    //    }
    //    if (tcsetattr(descriptors[2], TCSAFLUSH, &options) == -1)
    //    {
    //        //DBG("SerialPort::open : can't set port settings (timeouts)");
    //		closePort();
    //        return OpenPortError;
    //    }
    //----------------

    if (tcsetattr (settings.getDescriptor(), TCSANOW, &options) == -1)
    {
        std::cout << "SerialPort::open : can't set port settings (timeouts)" << std::endl;
        releasePort (settings);
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to open port");
    }

    //-------------
    //    if (tcsetattr(descriptors[1],TCSANOW,&options) == -1)
    //    {
    //        //DBG("SerialPort::open : can't set port settings (timeouts)");
    //		closePort();
    //        return OpenPortError;
    //    }
    //    if (tcsetattr(descriptors[2],TCSANOW,&options) == -1)
    //    {
    //        //DBG("SerialPort::open : can't set port settings (timeouts)");
    //		closePort();
    //        return OpenPortError;
    //    }
    //-------------

    //    descriptorMax = -1;
    //    for (int i = 0; i > descriptors.size(); i++)
    //    {
    //        if (descriptors[i] > descriptorMax)
    //            descriptorMax = descriptors[i];
    //    }

    //set non standard baud rate
    //    speed_t nonStandardBaud = 250000;
    //    if (ioctl(descriptors.getLast(), IOSSIOSPEED, &nonStandardBaud, 1) == -1)
    //    {
    //        std::cout << "SerialPort::open : can't set non-standard speed";
    //        closePort(descriptors.size()-1);
    //        return OpenPortError;
    //    }
    #endif
    return SerialPort::ErrorStatus::ok();//success
}

SerialPort::ErrorStatus SerialPort::releasePort (SerialPort::SerialPortSettings& settings)
{
    #if defined(_WIN64) || defined(_WIN32)
    if (CloseHandle (settings.getDescriptor()) == 0)
    #else
    if (::close (settings.getDescriptor()) == -1)					// Close the Serial Port
    #endif
        return SerialPort::ErrorStatus::fail ("SerialPortError: Unable to close port");

    return SerialPort::ErrorStatus::ok();
}

bool SerialPort::isReading() const
{ 
    return threadIsRunning;
}

void SerialPort::writeToPort (const std::string& nameOfPortToWriteTo, const char* data, size_t dataSize)
{
    std::lock_guard<std::mutex> sl (lock);
    dataToWrite.push_back (WriteData (nameOfPortToWriteTo, data, dataSize));
}

void SerialPort::addListener (SerialPort::Listener* const listenerToAdd)
{
    if (listenerToAdd != nullptr) 
    {
        std::lock_guard<std::mutex> sl (lock);
        listeners.add (listenerToAdd);
    }
}

void SerialPort::removeListener (SerialPort::Listener* const listenerToRemove)
{
    if (listenerToRemove != nullptr) 
    {
        std::lock_guard<std::mutex> sl (lock);
        listeners.remove (listenerToRemove);
    }
}

void SerialPort::run()
{
    threadIsRunning = true;
    threadShouldExit = false;
    while ( ! threadShouldExit)
    {
        if ( ! readInputs()) //returns true if bytes were read so pause if not
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //process any queued requests
        configurePortsToOpen();
        configurePortsToClose();
        writeQueuedMessages();
    }
    threadIsRunning = false;
}

bool SerialPort::readInputs()
{
    auto error = SerialPort::ErrorStatus::ok();
    auto dataWasRead = false;
    #if defined(_WIN64) || defined(_WIN32)
    for (SerialPortSettings& port : openPortSettings)
    {
        DWORD numberOfBytesRead = 0;
        if( ! ReadFile (port.getDescriptor(), buffer.data(), buffer.size(), &numberOfBytesRead, NULL))
        {
            error = SerialPort::ErrorStatus::fail ("SerialPortError: Error reading from the serial input");
        }
        else
        {
            if (numberOfBytesRead)
            {
                std::lock_guard<std::mutex> sl(lock);
                listeners.call ([&port, data = buffer.data(), &numberOfBytesRead](Listener& l)
                {
                    l.serialDataRecieved (port.getPortName(), data, numberOfBytesRead);
                });
                dataWasRead = true;
            }
        }
    }

    #else //__APPLE__
    
    if (openPortSettings.size() > 0)
    {
        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1000;
        fd_set  descriptorSet;
        FD_ZERO (&descriptorSet);
        for (SerialPortSettings port : openPortSettings)
            FD_SET (port.getDescriptor(), &descriptorSet);
        if (select (descriptorMax, &descriptorSet, NULL, NULL, &timeout) < 0) //select error
        {
            error = SerialPort::ErrorStatus::fail ("SerialPortError: An error occurred, waiting for port activity (select() error)");
        }
        else //select worked fine
        {
            for (SerialPortSettings& port : openPortSettings)
            {
                // If we have input then read it
                if (FD_ISSET (port.getDescriptor(), &descriptorSet))
                {
                    auto numberOfBytesRead = read (port.getDescriptor(), buffer.data(), buffer.size());
                    if (numberOfBytesRead <= 0) //if it didn't work
                    {
                        error = SerialPort::ErrorStatus::fail ("SerialPortError: Error reading from the serial input");
                    }
                    else
                    {
                        std::lock_guard<std::mutex> sl (lock);
                        listeners.call ([&port, data = buffer.data(), &numberOfBytesRead](Listener& l)
                                        {
                                            l.serialDataRecieved (port.getPortName(), data, numberOfBytesRead);
                                        });
                    }
                }
            }
        }
    }
    #endif
    //do something with the error?
    if (error.failed())
    {
        std::cout << error.getErrorMessage();
    }
    return dataWasRead;
}

void SerialPort::writeQueuedMessages()
{
    auto error = SerialPort::ErrorStatus::ok();
    std::unique_lock<std::mutex> ul (lock);
    while (dataToWrite.size())
    {
        WriteData writeData = dataToWrite.front();
        
        for (int i = 0; i < openPortSettings.size(); i++)
        {
            SerialPortSettings port = openPortSettings[i];
            if (port.getPortName() == writeData.portToWriteTo)
            {
                const std::vector<char>& buf (writeData.data);

                #if defined(_WIN64) || defined(_WIN32)
                unsigned long bytesWritten;
                if (WriteFile (port.getDescriptor(), buf.data(), buf.size(), &bytesWritten, NULL) == false)//failed
                {
                    error = SerialPort::ErrorStatus::fail ("SerialPortError: Writing to the Serial Port Failed");
                }
                else if (bytesWritten != buf.size())
                {
                    error = SerialPort::ErrorStatus::fail ("SerialPortError: Writing to the Serial Port Failed");
                }
                #else
                if (write (port.getDescriptor(), buf.data(), buf.size()) != buf.size())
                    error = SerialPort::ErrorStatus::fail ("SerialPortError: Writing to the Serial Port Failed");
                #endif
             }
        }
        dataToWrite.erase (dataToWrite.begin());
    }
    ul.unlock();
    if (error.failed())
    {
        std::cout << "Serial Port Write Error: " << error.getErrorMessage() << std::endl;
        std::cout << "Closing serial port(s)" << std::endl;
        closeAllPorts();
    }
}

void SerialPort::configurePortsToOpen()
{
    std::unique_lock<std::mutex> ul (lock);
    bool doneSomething {portsToOpen.size() > 0};
    while (portsToOpen.size())
    {
        SerialPortSettings port = portsToOpen.back();
        if (initialisePort (port).wasOk())
        {
            openPortSettings.push_back (port);
            configureGlobalSettings();
            //no need to lock as we are already locked above
            listeners.call ([&port](Listener& l)
                            {
                                l.serialConnectionStateChanged (port.getPortName(), true, "");
                            });
        }
        else
        {
            listeners.call ([&port](Listener& l)
                            {
                                l.serialConnectionStateChanged (port.getPortName(), false, "Serial error: failed to open port: " + port.getPortName());
                            });
            std::cout << "Serial error: failed to open port: " << port.getPortName();
        }
        portsToOpen.pop_back();
    }
    //if we've opened some ports then notify possible waiters
    if (doneSomething)
    {
        ul.unlock();
        waiter.notify_one();
    }
}

void SerialPort::configurePortsToClose()
{
    std::lock_guard<std::mutex> sl (lock);
    while (portsToClose.size())
    {
        std::string portNameToClose = portsToClose.back();
        
        for (int i = 0; i < openPortSettings.size(); i++)
        {
            SerialPortSettings port = openPortSettings[i];
            if (port.getPortName() == portNameToClose)
            {
                releasePort (port);
                openPortSettings.erase (openPortSettings.begin() + i);
                configureGlobalSettings();
                listeners.call ([&port](Listener& l)
                                {
                                    l.serialConnectionStateChanged (port.getPortName(), false, "");
                                });
                break;
            }
        }
        portsToClose.pop_back();
    }
}

void SerialPort::configureGlobalSettings()
{   
    #if __APPLE__
    descriptorMax = -1;
    for (SerialPortSettings port : openPortSettings)
    {
        if (port.getDescriptor() > descriptorMax)
            descriptorMax = port.getDescriptor();
    }
    descriptorMax += 1;
    #endif
    
    readState = false;
    for (int i = 0; i < openPortSettings.size(); i++)
    {
        SerialPortSettings& port = openPortSettings[i];
        if (port.getReadWriteAccess() == ReadOnly || port.getReadWriteAccess() == ReadAndWrite)
            readState = true;
    }
}