#ifndef SecComm_h
#define SecComm_h
/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
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
#include "AseThread.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
enum SecCmds {
//--------------------------- AseMain Serviced 1 - 199 -----------------------
    eRunScript      = 1,
    eScriptDone     = 2,
    eShutdown       = 3,
    ePing           = 4,
    ePowerOn        = 5,
    ePowerOff       = 6,
    eMsState        = 7,
    eDisplayState   = 8,  // this is directed at a specific Process

// Ase Enums 101-199

//---------------------------- CMProcess 200 - 399 ----------------------------
    eReadStream     = 200,
    eClearStream    = 201,
    eWriteStream    = 202,
    ePutFile        = 203,  // put a file onto the UUT in a partition
    eGetFile        = 204,  // get a file from the UUT in a partition
    eSetCfgFileName = 205,
    eStartReconfig  = 206,
    eGetReconfigSts = 207,
    eLogFileReady   = 208,
    eLogFileCrc     = 209,
    eFileExists     = 210,

// CmProc Reconfig Controls
    eCmFileNameDelay = 250,
    eCmRecfgAckDelay = 251,
    eGetRcfCount     = 252,
    eDeleteFile      = 253,
    eCmLatchWait     = 254,

// CmProc Enums 301-399
    eCmPartCfg      = 301,  // the Cfg file partition Id
    eCmPartLog      = 302,  // the Log file partition Id

//-------------------------------- IOI 400 - 599 ------------------------------
    eGetSensorNames = 400,
    eGetSensorValue = 401,
    eSetSensorValue = 402,   // TBD: this should be deleted as only eSetSensorSG is used
    eSetSensorSG    = 403,
    eResetSG        = 404,
    eRunSG          = 405,
    eHoldSG         = 406,
    eSetSdi         = 407,
    eSetSsm         = 408,
    eSetLabel       = 409,
    eSendParamData  = 410,
    eInitParamData  = 411,
    eDisplayParam   = 412,
    eResetIoi       = 413,
    eParamState     = 414,
    eParamIoState   = 415,


//-------------------------------- unallocated --------------------------------
    eStartLogging   = 55560,
    eStopLogging    = 55570,
    eLoadCfg        = 55580,
    eClearCfg       = 55590,
    ePowerToggle    = 55125,
    eChannelPause   = 55130,
    eChannelResume  = 55140,
    eSetPfen        = 55150,
    ePowerInt       = 55160,
    eSetBarker      = 55170,
    eSet429BitRate  = 55180,
    eSet429Amp      = 55190,
    eSetQarPause    = 55230,
    eSetQarData     = 55240,

    eSetFileStream  = 55650,
    eSetBaudRate    = 55660,
    eSetFfd         = 55670,

    eSampleAin      = 51000,
    eReadAvgAin     = 51010,
    eReadPeakAin    = 51020,
    eReadDin        = 51030,
    eReadDinOr      = 51040,
    eClearDinOr     = 51050,
    eSelectAntenna  = 51060,

};

///////////////////////////////////////////////////////////////////////////////////////////////
enum SecSystemConstants {
    eSecNumberOfSensors = 125,         // SEC/IOC number of sensors
    eSecErrorMsgSize    = 128,         // SEC/IOC max error Message
    eSecStreamSize      = eAseStreamSize,   // the size of input stream data
    eSecCharDataSize    = eAseCharDataSize, // SEC/IOC max filename size
    eSecPortNumber      = 51423,       // socket port connection on local host
    eSecAseH1           = 0x05EC2A5E,  // Header marker1 Sec->Ase
    eSecAseH2           = 0xABCD1234u, // Header marker2 Sec->Ase
    eAseSecH1           = 0x0A5E25EC,  // Header marker1 Ase->Sec
    eAseSecH2           = 0x1234ABCD,  // Header marker2 Ase->Sec
    eHeaderError        = 254,         // The header content was not correct
    eChecksumError      = 255          // Failed the checksum
};

enum ResponseType {
    eRspWait,
    eRspNormal,
    eRspSensors,
};


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
    INT32   variableId;      // general purpose data item see command for specifics
    UINT32  sigGenId;
    UINT32  resetRequest;
    UINT32  clearCfgRequest;
    UINT32  videoDisplay;

    FLOAT32 value;
    FLOAT32 param1;
    FLOAT32 param2;
    FLOAT32 param3;
    FLOAT32 param4;

    UINT32  resetAll;
    UINT32  charDataSize;
    CHAR    charData[eSecCharDataSize];
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
    CHAR    streamData[eSecStreamSize];
    CHAR    errorMsg[eSecErrorMsgSize];
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
    UINT32 pBaseIndex;
    UINT32 maxIndex;
    ParameterName names[eSecNumberOfSensors];
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

    // this is true when a new request comes in and while it is being serviced
    BOOLEAN IsConnected() const {
        return m_connState == eConnConnected;
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

    virtual void Run();      // spawn a receiver thread for the commands

    void ErrorMsg( char* format, ...);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // PUBLIC DATA
    BOOLEAN m_isValid;      // was all of the socket/thread creation ok
    BOOLEAN m_connected;    // are we connected
    UINT16 m_port;          // what port are we on - default = 54321
    UINT32 m_lastSequence;  // the last command sequence number
    BOOLEAN forceConnectionClosed;  // connection has been idle too long close and reopen
    BOOLEAN isRxing;

    SecRequest  m_request;  // the current or last cmd received
    SecRequest  m_bufRqst;  // a buffer for the in coming cmd

    ResponseType m_rspType; // What is the status of the response
    IocResponse m_response; // structure to return cmd results to PySTE
    SensorNames m_snsNames; // structure to return the sensor names to PySTE

protected:
    virtual void Process();  // The receiver thread processing

private:
    void OpenConnection();
    INT32  SetupTransport(clientConnectionHandleType &connectionHandle, CHAR* connectionId);
    void CheckCmd(const char* buffer, const int size);
    void ResetConn();
    UINT32 Checksum( void* ptr, int size);
    void SendResponse();
    void SendAny(const char* data, UINT32 size);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // PRIVATE DATA
    UINT32 m_cmdRequest;     // how many requests have come in
    UINT32 m_cmdServiced;    // how many cmds have been completely serviced
    CHAR* m_cmdHandlerName;  // Handler gives a pointer to their name
    CHAR  m_errMsg[eSecErrorMsgSize]; // holds any error message when the SecComm object is invalid
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

#endif
