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
#include "File.h"

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
    const UINT32 MAX_IDLE_FRAMES = 15 * systemTickTimeInHz;

    UINT32 i;
    UINT32 start;
    UINT32 td;
    SecComm secComm;
    UINT32 frames = 0;
    UINT32 cmdIdle = 0;
    UINT32 lastCmdAt = 0;

    memset( &aseCommon, 0, sizeof(aseCommon));

    // default to MS being online
    aseCommon.bMsOnline = true;

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    debug_str_init();

    // Initially create the adrf to start it running.
    adrfProcStatus  = createProcess( adrfName, adrfTmplName, 0, TRUE, &adrfProcHndl);

    aseCommon.adrfState = (processSuccess == adrfProcStatus) ? eAdrfOn : eAdrfOff;

    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; i < MAX_CMD_RSP; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }

    videoRedirect = AseMain;

    // overhead of timing
    start = HsTimer();
    td = HsTimeDiff(start);
    td = HsTimeDiff(start);

    debug_str(AseMain, 3, 0, "Last Cmd Id: 0");

    // The main thread goes into an infinite loop.
    while (1)
    {
        // call the base class to get the first row
        cmdRspThreads[0]->CmdRspThread::UpdateDisplay(AseMain, 0);

        // Write the system tick value to video memory.
        debug_str(AseMain, 1, 0, "SecComm(%s) %d - %d : %s",
                  secComm.GetSocketInfo(),
                  frames, td,
                  version);

        debug_str(AseMain, 2, 0, "Rx(%d) Tx(%d) IsRx: %s CloseConn: %s Idle Time: %4d/%d",
                  secComm.GetRxCount(),
                  secComm.GetTxCount(),
                  secComm.isRxing ? "Yes" : "No ",
                  secComm.forceConnectionClosed ? "Yes" : "No",
                  cmdIdle+1,
                  MAX_IDLE_FRAMES
                  );

        debug_str(AseMain, 4, 0, "%s", secComm.GetErrorMsg());
        debug_str(AseMain, 5, 0, "CmProc: %d Ioi: %d", cmProc.m_frames, ioiProc.m_frames);

        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();
        frames += 1;

        // Any new cmds seen
        if (CheckCmds( secComm))
        {
            lastCmdAt = frames;
        }
        else
        {
            // no timeout if we are running a script
            cmdIdle = frames - lastCmdAt;
            if (cmdIdle > MAX_IDLE_FRAMES)
            {
                secComm.forceConnectionClosed = TRUE;
                lastCmdAt = frames;
            }
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

        videoRedirect = (VID_DEFS)request.videoDisplay;

        debug_str(AseMain, 3, 0, "Last Cmd Id: %d        ", request.cmdId);

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
                adrfProcStatus = createProcess( "adrf", "adrf-template", 0, TRUE, &adrfProcHndl);
                debug_str(AseMain, 5, 0, "PowerOn: Create process %s returned: %d",
                                                                 adrfName,
                                                                 adrfProcStatus);
                // Update the global-shared data block
                aseCommon.adrfState = (processSuccess == adrfProcStatus) ? eAdrfOn : eAdrfOff;
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
            aseCommon.adrfState = eAdrfOff;

            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eMsState:
            // Kill the ADRF process to simulate behavior during power off
            if (request.variableId == 0 || request.variableId == 1)
            {
                aseCommon.bMsOnline = request.variableId == 1;
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("Ms State Error: Accept 0,1 - got %d", request.variableId);
                secComm.m_response.successful = FALSE;
            }
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

//-------------------------------------------------------------------------------------------------
