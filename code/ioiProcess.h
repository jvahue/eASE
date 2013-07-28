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
    };
    
    IoiProcess();
    virtual void Run();
    virtual BOOLEAN CheckCmd( SecComm& secComm);

protected:

     // Methods
    virtual void RunSimulation(); // override the CmdRspThread::RunSimulation
    
    void InitIoi();
    void UpdateIoi();
    
    UINT32 m_paramCount;
    Parameter m_parameters[eIoiMaxParams];
};

#endif
