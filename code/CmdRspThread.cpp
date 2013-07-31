/******************************************************************************
            Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
               All Rights Reserved. Proprietary and Confidential.

    File:        CmdRspThread.cpp

    Description: The file implements the CmdRspThread

    VERSION
    $Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

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
{
}

void CmdRspThread::Process()
{
    while (1)
    {
        m_systemTick = GET_SYSTEM_TICK;
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

        waitUntilNextPeriod();
    }
}

void CmdRspThread::RunSimulation()
{
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
BOOLEAN CmdRspThread::CheckCmd( SecComm& secComm)
{
    return FALSE;
}
