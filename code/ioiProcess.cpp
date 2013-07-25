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
 protected methods for FxProc
 ****************************************************************************/

void IoiProcess::Run()
{
    // create alias for this process because adrf will be granting write access
    // to CMProcess, not ASE

    processStatus ps = createProcessAlias( "ioi");

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("IoiProcess", "StdThreadTemplate");
}


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
        waitUntilNextPeriod();
    }
}

BOOLEAN IoiProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eSetSensorValue:
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

