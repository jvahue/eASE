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

LINUX_TM_FMT nextTime;
UINT32 _10msec;

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
AseCommon aseCommon;

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
CmdRspThread* cmdRspThreads[] = {
  &cmProc,
  &ioiProc,
  NULL
};

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
static void SetTime(SecRequest& request);

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

    _10msec = 0;

    memset( &aseCommon, 0, sizeof(aseCommon));
    memset( &nextTime, 0, sizeof(nextTime));

    // default time 
    aseCommon.time.tm_year = 2013;
    aseCommon.time.tm_mon  = 7;
    aseCommon.time.tm_mday = 26;

    nextTime.tm_year = 2013;
    nextTime.tm_mon  = 7;
    nextTime.tm_mday = 27;
    
    // default to MS being online
    aseCommon.bMsOnline = true;

    // Grab the system tick pointer, all threads/tasks should use GET_SYSTEM_TICK
    aseCommon.systemTickPtr = systemTickPointer();

    debug_str_init();
    videoRedirect = AseMain;

    // Initially create the adrf to start it running.
    adrfProcStatus  = createProcess( adrfName, adrfTmplName, 0, TRUE, &adrfProcHndl);
    debug_str(AseMain, 6, 0, "Initial Create of adrf returned: %d", adrfProcStatus);

    aseCommon.adrfState = (processSuccess == adrfProcStatus) ? eAdrfOn : eAdrfOff;

    secComm.Run();

    // Run all of the cmd response threads
    for (i=0; cmdRspThreads[i] != NULL; ++i)
    {
        cmdRspThreads[i]->Run(&aseCommon);
    }

    // overhead of timing
    start = HsTimer();
    td = HsTimeDiff(start);
    td = HsTimeDiff(start);

    // see CheckCmds - where this is updated
    debug_str(AseMain, 5, 0, "Last Cmd Id: 0");

    // The main thread goes into an infinite loop.
    while (1)
    {
        // call the base class to get the first row
        cmdRspThreads[0]->CmdRspThread::UpdateDisplay(AseMain, 0);

        debug_str(AseMain, 1, 0, "ASE Version: %s Now: %04d/%02d/%02d %02d:%02d:%02d.%0.3d",
                  version,
                  aseCommon.time.tm_year,
                  aseCommon.time.tm_mon,   // month    0..11
                  aseCommon.time.tm_mday,  // day of the month  1..31
                  aseCommon.time.tm_hour,  // hours    0..23
                  aseCommon.time.tm_min,   // minutes  0..59
                  aseCommon.time.tm_sec,   // seconds  0..59
                  _10msec
                  );

        // Write the system tick value to video memory.
        debug_str(AseMain, 2, 0, "SecComm(%s) %d - %d",
                  secComm.GetSocketInfo(),
                  frames, td);

        debug_str(AseMain, 3, 0, "Rx(%d) Tx(%d) IsRx: %s CloseConn: %s Idle Time: %4d/%d",
                  secComm.GetRxCount(),
                  secComm.GetTxCount(),
                  secComm.isRxing ? "Yes" : "No ",
                  secComm.forceConnectionClosed ? "Yes" : "No",
                  cmdIdle+1,
                  MAX_IDLE_FRAMES
                  );

        debug_str(AseMain, 4, 0, "%s", secComm.GetErrorMsg());

        // Yield the CPU and wait until the next period to run again.
        waitUntilNextPeriod();
        
        // keep time
        _10msec += 10;
        if (_10msec >= 1000)
        {
            _10msec = 0;
            aseCommon.time.tm_sec += 1;
            if (aseCommon.time.tm_sec >= 60)
            {
                aseCommon.time.tm_sec = 0;
                aseCommon.time.tm_min += 1;
                if (aseCommon.time.tm_min >= 60)
                {
                    aseCommon.time.tm_min = 0;
                    aseCommon.time.tm_hour += 1;
                    if (aseCommon.time.tm_hour >= 24)
                    {
                        aseCommon.time.tm_hour = 0;
                        aseCommon.time.tm_year = nextTime.tm_year;
                        aseCommon.time.tm_mon = nextTime.tm_mon;
                        aseCommon.time.tm_mday = nextTime.tm_mday;
                    }
                }
            }
        }

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

        debug_str(AseMain, 5, 0, "Last Cmd Id: %d        ", request.cmdId);

        switch (request.cmdId)
        {
        case eRunScript:
            aseCommon.bScriptRunning = TRUE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eScriptDone:
            aseCommon.bScriptRunning = FALSE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eShutdown:
            aseCommon.bScriptRunning = FALSE;
            SetTime(request);
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case ePing:
            // ping carries the current time and next time
            SetTime(request);
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
            SetTime(request);
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

            SetTime(request);
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
static void SetTime(SecRequest& request)
{
    aseCommon.time.tm_year = request.variableId;  // year from 1900
    aseCommon.time.tm_mon  = request.sigGenId;   // month    0..11
    aseCommon.time.tm_mday = request.resetRequest;  // day of the month  1..31
    aseCommon.time.tm_hour = request.clearCfgRequest;  // hours    0..23
    aseCommon.time.tm_min  = int(request.value);   // minutes  0..59
    aseCommon.time.tm_sec  = int(request.param1);   // seconds  0..59

    nextTime.tm_year = int(request.param2);
    nextTime.tm_mon  = int(request.param3);
    nextTime.tm_mday = int(request.param4);

    _10msec = 0;
}
