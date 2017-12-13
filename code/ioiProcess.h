#ifndef IOIPROCESS_H
#define IOIPROCESS_H

#include <ioiapi.h>

#include "CmdRspThread.h"
#include "File.h"
#include "ioiStatic.h"
#include "MailBox.h"
#include "Parameter.h"
#include "ccdl.h" // this needs to be after CmdRspThread.h so it get video.h

// File: ioiProcess.h

struct SensorSetup {
    UINT16 index;
    UINT8  sgType;
    UINT8  spare;
    UINT32 unitValue;
    FLOAT32 p1;
    FLOAT32 p2;
    FLOAT32 p3;
    FLOAT32 p4;
};

/******************************************************************************
* Description: Implements the ASE thread connecting to the ADRF for testing
*              IOI behavior.
*
*
*/
class IoiProcess : public CmdRspThread
{
public:
    enum IoiConstants {
        eIoiFailDisplay = 10,
        eIoiMaxDisplay = 34,  // MUST BE AN EVEN NUMBER 40 or less
        eMaxPages = 3,
        eMaxQueueDepth = 2,
        eMaxTriggerSize = 128
    };


    IoiProcess();

    // specialization of CmdRspThread
    virtual void Run();
    virtual BOOLEAN CheckCmd( SecComm& secComm);
    virtual int UpdateDisplay(VID_DEFS who, int theLine);

    int GetChanId(void); // 0=B, 1=A
    bool SetChanId(int chanId);
    void WriteChanId();

protected:

    // Methods
    // override the CmdRspThread::RunSimulation
    virtual void RunSimulation();
    virtual void HandlePowerOff();
    // Send the sensor names to ePySte
    void FillSensorNames(INT32 start, SensorNames& m_snsNames) const;
    void ScheduleParameters();

    bool SetSensor(SensorSetup* sensorData, SecComm& secComm);

    bool CollectParamInfo(int paramSetCount, UINT32 paramCount, char* data);
    void InitIoi();

    void UpdateIoi();

    void WriteIoi(Parameter* param );

    void UpdateCCDL();

    int PageIoiStatus(int theLine, bool& nextPage);
    int PageParams(int theLine, bool& nextPage);
    int PageStatic(int theLine, bool& nextPage);

    UINT32 m_paramCount;
    UINT32 m_minParamIndex;
    UINT32 m_maxParamIndex;
    UINT32 m_paramLoopEnd;
    Parameter m_parameters[eAseMaxParams];

    ParamCfg m_paramInfo[eAseMaxParams];
    UINT32 m_paramInfoCount;

    UINT32 m_displayCount;
    UINT32 m_displayIndex[eIoiMaxDisplay];
    UINT32 m_page;
    UINT32 m_paramDetails;









    UINT32 m_scheduledX;
    UINT32 m_remoteX;
    UINT32 m_scheduled;
    UINT32 m_ioiUpdated;
    UINT32 m_loopCount;
    UINT32 m_timeout;     // how many times have we exited IoiUpdate 4 timeout

    UINT32 m_allSlackEnd;   // what was the tick count when we start allSlack mode

    bool m_initParams;
    ioiStatus m_initStatus;

    UINT32 m_paramOpenFailCount;
    UINT32 m_paramCloseFailCount;
    UINT32 m_ioiWriteFailCount;

    ParameterName m_openFailNames[eIoiFailDisplay];
    ParameterName m_closeFailNames[eIoiFailDisplay];

    bool m_sgRun;
    bool m_paramIoRunning;

    // debug timing
    UINT32 m_avgIoiTime;
    UINT32 m_totalParamTime;
    UINT32 m_totalIoiTime;

    // CCDL Stuff
    CCDL    m_ccdl;
    MailBox m_ccdlIn;   // local to remote ADRF - we own this
    MailBox m_ccdlOut;  // remote to local ADRF - ADRF owns this
    char m_localTriggers[eMaxTriggerSize];
    char m_remoteTriggers[eMaxTriggerSize];


    INT32 m_chanId;       // 0=B, 1=A
    File  m_chanIdFile;
    INT32 m_ioiChanId; 
    INT32 m_ioiChanId0; 
    INT32 m_ioiChanId1; 

    UINT32 m_maxProcDuration;
    UINT32 m_restoreDuration;
    UINT32 m_elapsed;
    UINT32 m_peak;

    UINT32 m_dateId;
    UINT32 m_timeId;

    //------------------------------------------
    // Static IOI Data
    StaticIoiContainer m_ioiStatic;
};

#endif
