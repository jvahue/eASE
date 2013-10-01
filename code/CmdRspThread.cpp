/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:        CmdRspThread.cpp

    Description: The file implements the CmdRspThread

    VERSION
    $Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseCommon.h"

#include "CmdRspThread.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const char* adrfState[] = {
    "Off",
    "On",
    "Ready",
};


/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmdRspThread::CmdRspThread()
    : m_systemTick(0)
    , m_frames(0)
    , m_overrunCount(0)
    , m_updateDisplay(true)
{
}

void CmdRspThread::Process()
{
    UINT32 theLine = 0;

    while (1)
    {
        m_systemTick = GET_SYSTEM_TICK;
        m_frames += 1;
        if (IS_ADRF_ON)
        {
            RunSimulation();
        }
        else
        {
            HandlePowerOff();
        }

        // check if we overran
        if (m_systemTick != GET_SYSTEM_TICK)
        {
            m_overrunCount += 1;
        }

        if (m_updateDisplay)
        {
            theLine = UpdateDisplay(VID_SYS, theLine);
        }

        waitUntilNextPeriod();
    }
}

void CmdRspThread::RunSimulation()
{
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateDisplay
// Description: Display the common proc state info at the top of the screen
//
int CmdRspThread::UpdateDisplay(VID_DEFS who, int theLine)
{
    //debug_str(who, 0, 0,"%s", m_blankLine);
    debug_str(who, theLine, 0,
              "ePySte(%s) ADRF(%s) MS(%s) Script(%s) Frame(%d/%d)",
              IS_CONNECTED ? "Conn" : "NoConn",
              adrfState[m_pCommon->adrfState],
              IS_MS_ONLINE ? "On" : "Off",
              IS_SCRIPT_ACTIVE ? "Run" : "Off",
              m_frames, m_overrunCount
              );

    return theLine;
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
BOOLEAN CmdRspThread::CheckCmd( SecComm& secComm)
{
    return FALSE;
}
