#ifndef IOIPROCESS_H
#define IOIPROCESS_H

#include <ioiapi.h>

#include "CmdRspThread.h"
#include "File.h"
#include "SecComm.h"
#include "Parameter.h"

// File: ioiProcess.h

/******************************************************************************
* Description: Implements the ASE thread connecting to the ADRF for testing
*              IOI behavior.
*
*
*/
struct ParamCfg {
    UINT32 index;
    UINT32 masterId;
    ParameterName name;
    UINT32 rateHz;
    PARAM_FMT_ENUM fmt;
    UINT32 gpa;
    UINT32 gpb;
    UINT32 gpc;
    UINT32 scale;
};

class IoiProcess : public CmdRspThread
{
public:
    enum IoiConstants {
        eIoiFailDisplay = 5,
        eIoiMaxDisplay = 10,
    };
    
    enum IoiState {
        eIoiStateInit,
        eIoiStateInitFail,
        eIoiState
    };

    IoiProcess();

    // specialization of CmdRspThread
    virtual void Run();
    virtual void HandlePowerOff();
    virtual BOOLEAN CheckCmd( SecComm& secComm);

protected:

     // Methods
    virtual void RunSimulation(); // override the CmdRspThread::RunSimulation
    void FillSensorNames(INT32 start, SensorNames& m_snsNames);       // Send the sensor names to ePySte
    void ScheduleParameters();

    bool CollectParamInfo(int paramSetCount, UINT32 paramCount, char* data);
    void InitIoi();
    
    void UpdateIoi();
    void UpdateCCDL();

    virtual void UpdateDisplay(VID_DEFS who);

    UINT32 m_paramCount;
    UINT32 m_maxParamIndex;
    UINT32 m_paramLoopEnd;
    Parameter m_parameters[eAseMaxParams];

    ParamCfg m_paramInfo[eAseMaxParams];
    UINT32 m_paramInfoCount;

    UINT32 m_displayCount;
    UINT32 m_displayIndex[eIoiMaxDisplay];

    UINT32 m_frames;
    UINT32 m_scheduled;
    UINT32 m_updated;

    ioiStatus m_initStatus;

    UINT32 m_ioiOpenFailCount;
    UINT32 m_ioiCloseFailCount;
    UINT32 m_ioiWriteFailCount;

    ParameterName m_openFailNames[eIoiFailDisplay];
    ParameterName m_closeFailNames[eIoiFailDisplay];

    bool m_sgRun;

    //File m_paramCfg;
};

#endif
