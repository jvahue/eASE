/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:        aseMain.cpp

    Description: The main ASE process

    VERSION
    $Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <string.h>
#include <videobuf.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "video.h"
#include "SecComm.h"
#include "CmProcess.h"
#include "ioiProcess.h"
#include "AseCommon.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define MAX_CMD_RSP 2

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
CmProcess cmProc;
IoiProcess ioiProc;

// adrf.exe process control vars
const char adrfName[] = "adrf";
const char adrfTmplName[] = "adrf-template";
processStatus    adrfProcStatus = processNotActive;
process_handle_t adrfProcHndl = NULL;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
AseCommon aseCommon;

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
CmdRspThread* cmdRspThreads[MAX_CMD_RSP] = {
  &cmProc,
  &ioiProc
};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
processStatus CreateAdrfProcess();

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/
// Function: main
// Description: The function main() implements this process' main thread
// (sometimes referred to as its primary thread).  That is, when Deos
// automatically creates the main thread, main() will execute.
// Declare some VideoStream objects that will allow this thread to output text to the target's
// video memory (essentially, a printf-style debugging aid).  For each VideoStream object,
// we specify four parameters (y, x, numY, numX), where:
// y=starting row, x=starting column, numY=number of rows, and numX=number of columns
//VideoStream videoOutTitle(14, 0, 1, 50);   // here is where we'll output the title string
//VideoStream videoOut1(20, 40, 1, 40);     // here is where we'll output the system tick value
int main(void)
{
    // These variables are used to hold values we want to output to video memory.
    const UNSIGNED32 systemTickTimeInHz = 1000000 / systemTickInMicroseconds();
    const UINT32 MAX_IDLE_FRAMES = (5 * 60) * systemTickTimeInHz;

    UINT32 i;

    SecComm secComm;
    UINT32 frames = 0;
    UINT32 lastCmdAt = 0;

    memset( &aseCommon, 0, sizeof(aseCommon));

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    debug_str_init();

    // Initially create the adrf to start it running.
    adrfProcStatus  = createProcess( adrfName, adrfTmplName, 0, TRUE, &adrfProcHndl);
    debug_str(AseMain, 5, 0, "Initial Create of adrf returned: %d", adrfProcStatus);

    aseCommon.bPowerOnState = (processSuccess == adrfProcStatus) ? TRUE : FALSE;

    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; i < MAX_CMD_RSP; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }

    debug_str(AseMain, 2, 0, "Last Cmd Id: 0");

    // The main thread goes into an infinite loop.
    while (1)
    {
        // Write the system tick value to video memory.
        debug_str(AseMain, 0, 0, "SecComm(%s) %d",
                  secComm.GetSocketInfo(),
                  frames);

        debug_str(AseMain, 1, 0, "Rx(%d) Tx(%d) %s",
                  secComm.GetRxCount(),
                  secComm.GetTxCount(),
                  secComm.GetErrorMsg());

        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();
        frames += 1;

        // Any new cmds seen
        if (CheckCmds( secComm))
        {
            lastCmdAt = frames;
        }
        else if ((frames - lastCmdAt) > MAX_IDLE_FRAMES)
        {
            secComm.forceConnectionClosed = TRUE;
            lastCmdAt = frames;
        }

        aseCommon.bConnected = secComm.IsConnected();
    }
}

//-------------------------------------------------------------------------------------------------
static BOOLEAN CheckCmds(SecComm& secComm)
{
    UINT32 i;
    BOOLEAN cmdSeen = FALSE;
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;

    if (secComm.IsCmdAvailable())
    {
        cmdSeen = TRUE;
        SecRequest request = secComm.m_request;

        debug_str(AseMain, 2, 0, "Last Cmd Id: %d        ", request.cmdId);

        switch (request.cmdId)
        {
        case eRunScript:
            aseCommon.bScriptRunning = TRUE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eScriptDone:
            aseCommon.bScriptRunning = FALSE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eShutdown:
            aseCommon.bScriptRunning = FALSE;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePing:
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOn:
            // Create the ADRF process to simulate behavior during power on
            if ( adrfProcHndl == NULL)
            {
                createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
                debug_str(AseMain, 5, 0, "PowerOn: Create process %s returned: %d",
                                                                 adrfName,
                                                                 adrfProcStatus);
                // Update the global-shared data block
                aseCommon.bPowerOnState = TRUE;
            }
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePowerOff:
            // Kill the ADRF process to simulate behavior during power off
            if (adrfProcHndl != NULL)
            {
                adrfProcStatus = deleteProcess( adrfProcHndl);
                debug_str(AseMain, 5, 0, "PowerOff: Delete process %s returned: %d",
                                                   adrfName,
                                                   adrfProcStatus);
                adrfProcStatus =  processNotActive;
                adrfProcHndl   = NULL;
            }
            // Update the global-shared data block
            aseCommon.bPowerOnState = FALSE;

            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        default:
            break;
        }

        if (serviced)
        {
            secComm.SetHandler("AseMain");
            secComm.IncCmdServiced(rType);
        }
        else
        {
            // Run all of the cmd response threads
            for (i=0; i < MAX_CMD_RSP; ++i)
            {
                if (cmdRspThreads[i]->CheckCmd(secComm))
                {
                    break;
                }
            }
        }
    }

    return cmdSeen;
}

processStatus CreateAdrfProcess()
{
    return createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
}
