/******************************************************************************
Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
All Rights Reserved. Proprietary and Confidential.

File:        SecComm.h

Description: This file holds structures and classes used in communication
between PySTE and the ASE application.

VERSION
$Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <socketapi.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "alt_stdtypes.h"
#include "AseThread.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
enum SecCmds {
// AseMain Serviced 1 - 100
    eRunScript      = 1,
    eScriptDone     = 2,
    eShutdown       = 3,
    ePing           = 4,

// CMProcess 200 - 300

// IOI 400 - 500
    eSetSensorValue = 10,
    eSetSensorSG    = 20,
    eResetSG        = 30,
    eRunSG          = 40,
    eHoldSG         = 50,
    eStartLogging   = 60,
    eStopLogging    = 70,
    eLoadCfg        = 80,
    eClearCfg       = 90,
    ePowerOn        = 110,
    ePowerOff       = 120,
    ePowerToggle    = 125,
    eChannelPause   = 130,
    eChannelResume  = 140,
    eSetPfen        = 150,
    ePowerInt       = 160,
    eSetBarker      = 170,
    eSet429BitRate  = 180,
    eSet429Amp      = 190,
    eSetSdi         = 200,
    eSetSsm         = 210,
    eSetLabel       = 220,
    eSetQarPause    = 230,
    eSetQarData     = 240,

    eGetSensorValue = 100,
    eReadStream     = 500,
    eWriteStream    = 600,
    eSetFileStream  = 650,
    eSetBaudRate    = 660,
    eSetFfd         = 670,

    eSampleAin      = 1000,
    eReadAvgAin     = 1010,
    eReadPeakAin    = 1020,
    eReadDin        = 1030,
    eReadDinOr      = 1040,
    eClearDinOr     = 1050,
    eSelectAntenna  = 1060,

    eGetSensorNames = 10000,
};

///////////////////////////////////////////////////////////////////////////////////////////////
enum AseSystemConstants {
    eAseSensorNameSize  = 32,         // SEC/IOC size of a sensor name (UUT uses 32)
    eAseNumberOfSensors = 125,        // SEC/IOC number of sensors
    eAseErrorMsgSize    = 128,        // SEC/IOC max error Message
    eAseStreamSize      = 3500,       // the size of input stream data
    eAseCharDataSize    = 2048,       // SEC/IOC max filename size
    eAsePortNumber      = 51423,      // socket port connection on local host
    eSecAseH1           = 0x05EC2A5E, // Header marker1 Sec->Ase
    eSecAseH2           = 0xABCD1234, // Header marker2 Sec->Ase
    eAseSecH1           = 0x0A5E25EC, // Header marker1 Ase->Sec
    eAseSecH2           = 0x1234ABCD, // Header marker2 Ase->Sec
};

///////////////////////////////////////////////////////////////////////////////////////////////
// Signal Generator Types
enum SigGenEnum {
    eSGmanual,     // no signal generator
    eSGramp,       // ramp from low to high, then back to low and repeat (min, max, time(s))
    eSGrampHold,   // ramp to a value and hold at max
    eSGtriangle,   // ramp up/down (min, max, time(s))
    eSGsine,       // sine wave starting at 0 deg, (freq(Hz), amplitude)
    eSG1Shot,      // one shot a signal (start, oneShotValue, frameIndex)
    eSGnShot,      // n-shot (baseline, nValue, frameIndex, nFrames)
    eSGpwm,        // PWM between two values (value1, value2) varying duty cycle
    eSGrandom,     // random values uniform dist (min, max)
    eMaxSensorMode
};

enum ResponseType {
    eRspWait,
    eRspNormal,
    eRspSensors,
};

typedef char SENSORNAME[eAseSensorNameSize];

///////////////////////////////////////////////////////////////////////////////////////////////
// These three must match up with the structure defined in Python see SecIocPayloads.py to  
// ensure consistency
struct SecRequest
{
    UINT32  header1;
    UINT32  header2;
    UINT32  size;
    UINT32  sequence;

    // ------------- Start Payload --------------
    UINT32  cmdId;           // the cmd identifier
    INT32   variableID;      // general purpose data item see command for specifics
    UINT32  sigGenId;
    UINT32  resetRequest;
    UINT32  clearCfgRequest;

    FLOAT32 value;
    FLOAT32 param1;
    FLOAT32 param2;
    FLOAT32 param3;
    FLOAT32 param4;

    UINT32  resetAll;
    UINT32  charDataSize;
    CHAR    charData[eAseCharDataSize];
    // -------------- End Payload ---------------

    UINT32  checksum;
};

///////////////////////////////////////////////////////////////////////////////////////////////
struct IocResponse
{
    UINT32  header1;
    UINT32  header2;
    UINT32  size;
    UINT32  sequence;

    // ------------- Start Payload --------------
    UINT32  successful;
    UINT32  cmdId;

    FLOAT32 value;

    UINT32  streamSize;
    CHAR    streamData[eAseStreamSize];
    CHAR    errorMsg[eAseErrorMsgSize];
    // -------------- End Payload ---------------

    UINT32  checksum;
};

///////////////////////////////////////////////////////////////////////////////////////////////
struct SensorNames
{
    UINT32  header1;
    UINT32  header2;
    UINT32  size;
    UINT32  sequence;

    // ------------- Start Payload --------------
    SENSORNAME names[eAseNumberOfSensors];
    // -------------- End Payload ---------------

    UINT32  checksum;
};


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
class SecComm : public AseThread
{
public:
    enum ConnState {
        eConnNoSocket,
        eConnWait,
        eConnNotConnected,
        eConnConnected,
    };

    SecComm();

    // keep track of cmds requested and serviced
    void IncCmdRequest() {
        m_cmdRequest += 1;
        m_rspType = eRspWait;
    }

    // called when the handler has completed processing of the cmd request
    void IncCmdServiced(ResponseType rsp=eRspNormal) {
        m_cmdServiced += 1;
        m_rspType = rsp;
        m_cmdHandlerName = NULL;
    }

    // this is true when a new request comes in and while it is being serviced
    BOOLEAN IsCmdAvailable() const {
        return m_cmdRequest != m_cmdServiced;
    }

    // this is true when a new request comes in and while it is being serviced
    BOOLEAN IsValid() const {
        return m_isValid;
    }

    // a thread handling the command set this for error reporting
    void SetHandler(CHAR* const handlerName) {
        m_cmdHandlerName = handlerName;
    }; 

    const CHAR* GetHandler( const CHAR* handlerName) const {
        return m_cmdHandlerName;
    }
    
    const UINT32 GetCmdId() const {
        return m_request.cmdId;
    }

    const CHAR* GetErrorMsg() const {
        return m_errMsg;
    }

    UINT32 GetRxCount() const {
        return m_rxCount;
    }

    UINT32 GetTxCount() const {
        return m_txCount;
    }

    const CHAR* GetSocketInfo();

    ///////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC DATA
    BOOLEAN m_isValid;      // was all of the socket/thread creation ok
    BOOLEAN m_connected;    // are we connected
    UINT16 m_port;          // what port are we on - default = 54321
    UINT32 m_lastSequence;  // the last command sequence number
    BOOLEAN forceConnectionClosed;  // connection has been idle too long close and reopen

    SecRequest  m_request;  // the current or last cmd received
    SecRequest  m_bufRqst;  // a buffer for the in coming cmd

    ResponseType m_rspType; // What is the status of the response
    IocResponse m_response; // structure to return cmd results to PySTE
    SensorNames m_snsNames; // structure to return the sensor names to PySTE

protected:
    virtual void Create();   // spawn a reciever thread for the commands
    virtual void Process();  // The receiver thread processing

private:
    void OpenConnection();
    INT32  SetupTransport(clientConnectionHandleType &connectionHandle, CHAR* connectionId);
    UINT32 Checksum( void* ptr, int size);
    void SendResponse();
    void SendAny(const char* data, UINT32 size);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // PRIVATE DATA
    UINT32 m_cmdRequest;     // how many requests have come in
    UINT32 m_cmdServiced;    // how many cmds have been completely serviced
    CHAR* m_cmdHandlerName;  // Handler gives a pointer to their name
    CHAR  m_errMsg[128];     // holds any error message when the SecComm object is invalid
    UINT32 m_rxCount;        // how many bytes have come in
    UINT32 m_txCount;        // how many bytes have gone out
    ConnState m_connState;   // connection state

    // Socket Connection data
    clientConnectionHandleType m_pysteHandle;
    int         m_socket;
    int         m_clientSocket;
    UINT32      m_acceptCount;
    sockaddr_in m_socketAddr;

    CHAR m_ipPort[80];

};

