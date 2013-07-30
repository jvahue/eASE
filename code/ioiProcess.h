#ifndef IOIPROCESS_H
#define IOIPROCESS_H

#include "CmdRspThread.h"
#include "SecComm.h"
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
        eIoiMaxParams = 3000,
        eIoiMaxDisplay = 20,
    };
    
    IoiProcess();
    virtual void Run();
    virtual BOOLEAN CheckCmd( SecComm& secComm);

protected:

     // Methods
    virtual void RunSimulation(); // override the CmdRspThread::RunSimulation
    void FillSensorNames(INT32 start, SensorNames& m_snsNames);       // Send the sensor names to ePySte
    
    void InitIoi();
    void UpdateIoi();
    
    UINT32 m_paramCount;
    UINT32 m_maxParamIndex;
    Parameter m_parameters[eIoiMaxParams];

    UINT32 m_displayCount;
    UINT32 m_displayIndex[eIoiMaxDisplay];

    UINT32 m_frames;
    UINT32 m_scheduled;
    UINT32 m_updated;
};

#endif
