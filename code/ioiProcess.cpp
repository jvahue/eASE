#include <deos.h>
#include <mem.h>
#include <string.h>
#include <stdio.h>

#include "ioiProcess.h"
#include "video.h"

static const CHAR adrfProcessName[] = "adrf";

IoiProcess::IoiProcess()
{
}

/****************************************************************************
 public methods for IoIProcess
****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Function: Run
// Description: Prepare to run and then launch the process
//
void IoiProcess::Run()
{
    // create alias for this process because some process needs to source the 
    // ioi data

    processStatus ps = createProcessAlias( "ioi");
    // TBD: may want this to act as io cross-channel also
    
    // TODO: create all of the IOI items

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("IoiProcess", "StdThreadTemplate");
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Update all of the "output" parameters from the ioi process
//
BOOLEAN IoiProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eSetSensorValue:
        // TBD: This option should be removed as ePySte will convert a call to SetSensor('x', 33.3) 
        //      into eSetSensorSG with the type set to manual
        break;

    case eSetSensorSG:
        break;

    case eResetSG:
        break;

    case eRunSG:
        break;

    case eHoldSG:
        break;

    default:
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("ioiProc");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
}

/****************************************************************************
 protected methods for IoIProcess
****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Function: RunSimulation
// Description: Processing done to simulate the ioi process
//
void IoiProcess::RunSimulation()
{
    UNSIGNED32* pSystemTickTime;
    UNSIGNED32  nextRequestTime;
    UNSIGNED32  nowTime;
    UNSIGNED32  interval = 100;  // 100 X 10 millisecs/tick = 1 sec interval

    // Grab the system tick pointer
    pSystemTickTime = systemTickPointer();


    while (1)
    {
        UpdateIoi();
        
        waitUntilNextPeriod();
    }
}

//-------------------------------------------------------------------------------------------------
// Function: InitIoi
// Description: Processing done to simulate the ioi process
//
void IoiProcess::InitIoi()
{
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateIoi
// Description: Update all of the "output" parameters from the ioi process
//
void IoiProcess::UpdateIoi()
{
}


