/*
 *  SerialPort.h
 *  Created by Tom Mitchell
 *  Copyright Tom Mitchell
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <thread>
#include <mutex>
#include <atomic>

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#include <fcntl.h>
#endif

#if defined(_WIN64) || defined(_WIN32)
typedef HANDLE SerialDescriptor;
#else
typedef int SerialDescriptor;
#endif

template <class ListenerClass>
class BasicListenerList
{
public:
    BasicListenerList() = default;
    ~BasicListenerList() = default;
    
    void add (ListenerClass* listenerToAdd)
    {
        if (listenerToAdd != nullptr && std::find (listeners.begin(), listeners.end(), listenerToAdd) == listeners.end())
            listeners.push_back (listenerToAdd);
//        else
//            jassertfalse;  // Listeners can't be null pointers!
    }
    void remove (ListenerClass* listenerToRemove)
    {
//        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!
        auto it = std::find (listeners.begin(), listeners.end(), listenerToRemove);
        if(it != listeners.end())
            listeners.erase (it);
    }
    
    template <typename Callback>
    void call (Callback&& callback)
    {
        for (auto& l : listeners)
            callback (*l);
    }
    
private:
    std::vector<ListenerClass*> listeners;
};

/**
 Class to manage serial port connections.

 Todo: 
 - Remove all the commented code
 - Do something with the errormessages - they can't be returned currently
 - Test fine control of the serial port settings!
 - When reading enable timeout, bufferlength or frame termination to control when callbacks are made
 - Add control for nonstandard baud rates
 */

class SerialPort
{
public:
    /** 
        Populates and returns @std::vector<std::string> with all availabe serial ports on the machine.
        @return A @std::vector<std::string> containing device names, if this is empty then there are no devices
        or something went wrong
     */
    static std::vector<std::string> getDevicePathList();
    
    /** Baudrate constants for serial port settings */
    enum BaudRate
    {
        #if defined(_WIN64) || defined(_WIN32)
        Baud110		= CBR_110,    
        Baud300		= CBR_300,    
        Baud600		= CBR_600,    
        Baud1200    = CBR_1200,
        Baud2400    = CBR_2400,
        Baud4800    = CBR_4800,
        Baud9600    = CBR_9600,
        Baud14400   = CBR_14400,
        Baud19200   = CBR_19200,
        Baud38400   = CBR_38400,
        Baud56000   = CBR_56000,  
        Baud57600   = CBR_57600,  
        Baud115200	= CBR_115200,
        Baud128000  = CBR_128000, 
        Baud256000  = CBR_256000
        #else
        Baud50		= B50,
        Baud75		= B75,
        Baud110		= B110,
        Baud134		= B134,
        Baud150		= B150,
        Baud200		= B200,
        Baud300		= B300,
        Baud600		= B600,
        Baud1200	= B1200,
        Baud1800	= B1800,
        Baud2400	= B2400,
        Baud4800	= B4800,
        Baud7200	= B7200,
        Baud9600	= B9600,
        Baud14400	= B14400,
        Baud19200	= B19200,
        Baud28800	= B28800,
        Baud38400	= B38400,
        Baud57600	= B57600,
        Baud76800	= B76800,
        Baud115200	= B115200,
        Baud230400	= B230400
        #endif
    };
    
    /** Read / write constants for serial port settings */
    enum ReadWriteAccess
    {
        #if defined(_WIN64) || defined(_WIN32) //JUCE_WINDOWS
        ReadOnly        = GENERIC_READ,
        WriteOnly       = GENERIC_WRITE,
        ReadAndWrite	= GENERIC_READ | GENERIC_WRITE
        #else
        ReadOnly        = O_RDONLY,
        WriteOnly       = O_WRONLY,
        ReadAndWrite    = O_RDWR
        #endif
    };
    
    /** Databits constants for serial port settings */
    enum DataBits
    {
        #if defined(_WIN64) || defined(_WIN32) //JUCE_WINDOWS
        DataBits5 = 5,
        DataBits6 = 6,
        DataBits7 = 7,
        DataBits8 = 8
        #else
        DataBits5 = CS5,
        DataBits6 = CS6,
        DataBits7 = CS7,
        DataBits8 = CS8
        #endif
    };
    
    /** Flow control constants for serial port settings */
    enum FlowControl
    {
        NoFlowControl = 0,
        EnableHardwareFlowControl = 1,
        EnableSoftwareFlowContorl = 2,
        EnableBothSoftwareAndHardwareFlowControl = EnableHardwareFlowControl | EnableSoftwareFlowContorl
    };

    /** Parity constants for serial port settings */
    enum StopBits
    {
        #if defined(_WIN64) || defined(_WIN32) //JUCE_WINDOWS
        OneStopBit = ONESTOPBIT,
        TwoStopBit = TWOSTOPBITS
        #else
        OneStopBit,
        TwoStopBit
        #endif
    };
    
    /** Parity constants for serial port settings */
    enum Parity
    {
        #if defined(_WIN64) || defined(_WIN32) //JUCE_WINDOWS
        NoParity = NOPARITY,
        EvenParity = EVENPARITY,
        OddParity = ODDPARITY
        #else
        NoParity,
        EvenParity,
        OddParity
        #endif
    };
    
    /** for error handling */
    class ErrorStatus
    {
    public:
        static ErrorStatus ok() noexcept         { return ErrorStatus(); }
        static ErrorStatus fail (const std::string& errorMessage) noexcept { return ErrorStatus (errorMessage.empty() ? "Unknown Error" : errorMessage);}
        bool wasOk() noexcept       {return errorMessage.empty();}
        bool failed() noexcept     {return ! errorMessage.empty();}
        const std::string& getErrorMessage() const noexcept {return errorMessage;}
    private:
        ErrorStatus() = default;
        ErrorStatus (const std::string& em) : errorMessage (em) {};
        std::string errorMessage;
    };
    
    /** struct for fine control of serial port settings, use defaultSettings for the defaults... */
    struct SerialPortSettings
    {
        SerialPortSettings (const std::string&  portName_ = "",
                            BaudRate            br        = Baud115200,
                            ReadWriteAccess     rwa       = ReadAndWrite,
                            DataBits            db        = DataBits8,
                            FlowControl         fc        = NoFlowControl,
                            StopBits            sb        = OneStopBit,
                            Parity              pty       = NoParity)
         :  portName (portName_), 
            readwriteAccess (rwa), 
            baudRate (br), 
            dataBits (db), 
            flowControl (fc), 
            stopBits (sb),
            parity (pty),
            #if defined(_WIN64) || defined(_WIN32) //JUCE_WINDOWS
            descriptor (INVALID_HANDLE_VALUE)
            #else
            descriptor (-1)
            #endif
        {
            
        }
        
        //Settings
        inline const std::string&   getPortName()           const {return portName;}
        inline ReadWriteAccess      getReadWriteAccess()    const {return readwriteAccess;}
        inline BaudRate             getBaudRate()           const {return baudRate;}
        inline DataBits             getDataBits()           const {return dataBits;}
        inline FlowControl          getFlowControl()        const {return flowControl;}
        inline StopBits             getStopBits()           const {return stopBits;}
        inline Parity               getParity()             const {return parity;}
    
        inline void setDescriptor (SerialDescriptor value) {descriptor = value;}
        inline SerialDescriptor getDescriptor () const {return descriptor;}
        private:
            std::string       portName;
            ReadWriteAccess   readwriteAccess;
            BaudRate          baudRate;
            DataBits          dataBits;
            FlowControl       flowControl;
            StopBits          stopBits;
            Parity            parity;
            SerialDescriptor descriptor;
    };

    /** Constructor */
    SerialPort(); 

    /** Destructor */
    ~SerialPort();

    /**
     Syncronously opens a port with the provided @SerialPortSettings.
     This function blocks until the attempt to open the port is complete in the serial port thread and returns a bool to indicate whether it worked
     
     Multiple calls can be made for opening several ports simultaniously. There is only one searial thread that services them all.
     
     @param settings  settings for the port to be opened
     @return true if successful and false otherwise
     */
    bool openPort (const SerialPort::SerialPortSettings& settings);

    /**
     Asyncronously opens a port with the provided @SerialPortSettings.
     This function returns imediately opening happens in the serial port thread
     
     Multiple calls can be made for opening several ports simultaniously. There is only one searial thread that services them all.
     
     @param settings  settings for the port to be opened
     */
    void openPortAsync (const SerialPort::SerialPortSettings& settings);

    /**
     Closes the named port, if this is the only port with the read mode flag set the read thread will be stopped.

     @returns a fail message if anything goes wrong.
     */
    void closePort (const std::string& nameOfPortToClose);

    /**
     Closes all ports, the read thread stopped.
     
     @returns a fail message if anything goes wrong.
     */
    void closeAllPorts();
    
    /** Returns true if there is a port open with the read flag set */
    bool isReading() const;
    
    /**
     Asyncronously writes the data on the specified port.
     This function returns imediately opening happens in the serial port thread
     */
    void writeToPort (const std::string& nameOfPortToWriteTo, const char* const data, size_t dataSize);
    
	
    ////////////////Listeners
	/**
     Class for a listener to this serial port connection to inherit. Only one listener at present.
     */
	class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener()                                     {}
		
        /** Called when the a data packet is recieved. */
        //virtual void serialDataRecieved (const int portIndex, const uint8 byte) = 0;
        virtual void serialDataRecieved (const std::string& portName, const char* bytes, size_t size) = 0;
        /** Called when the connection state changes if this is due to an error then errorMessage will contain a description,
         if there is no error then errorMessage will be empty */
        virtual void serialConnectionStateChanged (const std::string& portName, bool isConnected, const std::string& errorMessage) = 0;
        /** Called when the connection timesout */
        virtual void serialPortTimeout() = 0;
    };
	
    /** adds the listener to recieve bytes as they are recieved. Only one at a time at the moment. */
	void addListener (SerialPort::Listener* const listenerToAdd);
	
    /** removes the listener so that it will no londger recieve messages. */
	void removeListener (SerialPort::Listener* const listenerToRemove);
    
private:
    /** Sets up a port ready for reading and writing this should only be called from the serial thread */
    ErrorStatus initialisePort (SerialPort::SerialPortSettings& settings);
    /** Sets up a port ready for reading and writing this should only be called from the serial thread */
    ErrorStatus releasePort (SerialPort::SerialPortSettings& settings);
    /** read from any open serial ports
        @returns true if bytes were read */
    bool readInputs();
    
    /** struct for queue items when writing data */
    struct WriteData
    {
        WriteData () = default;
        WriteData (const std::string& portName, const char* d, size_t dataSize)
        :  portToWriteTo (portName), data (d, d + dataSize) {}
        std::string portToWriteTo;
        std::vector<char> data;
    };
    
    /** write any pending serial messages */
    void writeQueuedMessages();
    /** Initialise ports queued in portsToOpen and move them to the openPortSettings array and update internal state */
    void configurePortsToOpen();
    /** release the ports queued in portsToClose and remove them to the openPortSettings array and update internal state*/
    void configurePortsToClose();

    /** recalculates imprtant parameters for all ports - should be called internally whenever a port is opened or close */
    void configureGlobalSettings();
    
    //Thread function
	void run();
    
    /////////////
    std::thread thread;
    std::atomic<bool> threadShouldExit = true;
    std::atomic<bool> threadIsRunning = false;
    std::mutex lock;                                    //used to synconrise access for all members accessible on multiple threads
    std::condition_variable waiter;
    std::vector<SerialPortSettings> openPortSettings;    //stores details for all open ports
    std::vector<SerialPortSettings> portsToOpen;         //producer storage used for port open requests
    std::vector<std::string> portsToClose;               //producer storage used for port close requests
    std::vector<WriteData> dataToWrite;                  //queue for data to be written in the serial thread
    
    std::array<char, 4096> buffer;
    
    bool readState  = false;
    int descriptorMax = -1;                              //required by select() 'IX only
    //////////
    BasicListenerList<Listener> listeners;
	//ListenerList<Listener> listeners; //for our only listener
	
	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerialPort);
};
