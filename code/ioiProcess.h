#ifndef IOIPROCESS_H
#define IOIPROCESS_H

#include "CmdRspThread.h"
#include "SecComm.h"

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
        IoiProcess();
        virtual void Run();
        virtual BOOLEAN CheckCmd( SecComm& secComm);

    protected:

         // Methods
        virtual void RunSimulation(); // override the CmdRspThread::Simulation
};


#endif
