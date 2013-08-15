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

{
    memset(m_blankLine, 0x20, sizeof(m_blankLine));
    m_blankLine[80] = '\0';

}

void CmdRspThread::Process()
{
    while (1)
    {
        m_systemTick = GET_SYSTEM_TICK;
        m_frames += 1;
        if (IS_POWER_ON)
        {
            RunSimulation();
        }
        else
        {
            HandlePowerOff();
        }

        // check if we overran
        if (m_systemTick < GET_SYSTEM_TICK)
        {
            m_overrunCount += 1;
        }

        UpdateDisplay();

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
void CmdRspThread::UpdateDisplay(VID_DEFS who)
{
    debug_str(who, 1, 0,"%s", m_blankLine);
    debug_str(who, 1, 0, "ePySte: %s ADRF: %s MS: %s Script: %s Frame: %d",
              IS_CONNECTED ? "Conn  " : "NoConn",
              IS_POWER_ON ? "On" : "Off",
              IS_MS_ONLINE ? "On" : "Off",
              IS_SCRIPT_ACTIVE ? "Run" : "Off",
              m_frames
              );
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
BOOLEAN CmdRspThread::CheckCmd( SecComm& secComm)
{
    return FALSE;
}
