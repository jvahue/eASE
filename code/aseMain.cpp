/******************************************************************************
            Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
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
#include <videobuf.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "video.h"
#include "SecComm.h"
#include "CmProcess.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
CmProcess cmProc;

// adrf.exe process control vars
processStatus    adrfProcStatus = processNotActive;
process_handle_t adrfProcHndl;

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
static BOOLEAN CheckCmds(SecComm& secComm);
static void    HandlePowerChange(SecCmds cmd);
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

    UNSIGNED32 *systemTickPtr;
    SecComm secComm;
    UINT32 frames = 0;
    UINT32 lastCmdAt = 0;
    // Grab the system tick pointer
    systemTickPtr = systemTickPointer();

    debug_str_init();

    secComm.Run();
    cmProc.Run();

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
    }
}

//-------------------------------------------------------------------------------------------------
static BOOLEAN CheckCmds(SecComm& secComm)
{
    BOOLEAN cmdSeen = FALSE;
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;

    if (secComm.IsCmdAvailable())
    {
        cmdSeen = TRUE;
        SecRequest request = secComm.m_request;

        debug_str(AseMain, 2, 0, "Last Cmd Id: %d", request.cmdId);

        switch (request.cmdId)
        {
        case ePing:
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            break;

        case eGetSensorNames:
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
            rType = eRspSensors;
            break;

        case ePowerOn:
            // Create the ADRF process to simulate behavior during power on
            //if (NULL == adrfProcHndl)
            //{
            //    adrfProcStatus  = createProcess( "adrf",
            //                                     "adrf-template",
            //                                     0,
            //                                     FALSE,
           //                                      &adrfProcHndl);
           // }
            break;

        case ePowerOff:
            // Kill the ADRF process to simulate behavior during power off
            //adrfProcStatus =  deleteProcess( adrfProcHndl);
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
            cmProc.CheckCmd(secComm);
        }
    }

    return cmdSeen;
}
