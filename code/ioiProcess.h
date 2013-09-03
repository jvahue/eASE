#ifndef IOIPROCESS_H
#define IOIPROCESS_H

#include <ioiapi.h>

#include "CmdRspThread.h"
//#include "File.h"
//#include "SecComm.h"
#include "Parameter.h"

// File: ioiProcess.h

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
        eIoiFailDisplay = 5,
        eIoiMaxDisplay = 10
    };
    
    //enum IoiState {
    //    eIoiStateInit,
    //    eIoiStateInitFail,
    //    eIoiState
    //};

    IoiProcess();

    // specialization of CmdRspThread
    virtual void Run();
    virtual BOOLEAN CheckCmd( SecComm& secComm);

protected:

     // Methods
    // override the CmdRspThread::RunSimulation
    virtual void RunSimulation(); 
    virtual void HandlePowerOff();
    // Send the sensor names to ePySte
    void FillSensorNames(INT32 start, SensorNames& m_snsNames) const;   
    void ScheduleParameters();

    bool CollectParamInfo(int paramSetCount, UINT32 paramCount, char* data);
    void InitIoi();
    
    void UpdateIoi();
    void UpdateCCDL();

    virtual int UpdateDisplay(int theLine);

    UINT32 m_paramCount;
    UINT32 m_maxParamIndex;
    UINT32 m_paramLoopEnd;
    Parameter m_parameters[eAseMaxParams];

    ParamCfg m_paramInfo[eAseMaxParams];
    UINT32 m_paramInfoCount;

    UINT32 m_displayCount;
    UINT32 m_displayIndex[eIoiMaxDisplay];

    UINT32 m_scheduled;
    UINT32 m_updated;

    ioiStatus m_initStatus;

    UINT32 m_ioiOpenFailCount;
    UINT32 m_ioiCloseFailCount;
    UINT32 m_ioiWriteFailCount;

    ParameterName m_openFailNames[eIoiFailDisplay];
    ParameterName m_closeFailNames[eIoiFailDisplay];

    bool m_sgRun;

    // debug timing
    UINT32 m_avgIoiTime;
    UINT32 m_totalParamTime;
    UINT32 m_totalIoiTime;


    //File m_paramCfg;
};

#endif
