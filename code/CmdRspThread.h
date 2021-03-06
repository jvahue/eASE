#ifndef CmdRspThread_h
#define CmdRspThread_h
/******************************************************************************
Copyright (C) 2013 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        CmdRspThread.h

Description: This file implements the base class for threads that can
respond to PySte commands

VERSION
$Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseThread.h"
#include "SecComm.h"
#include "video.h"

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
class CmdRspThread : public AseThread
{
public:
    enum CmdRspConstants {
        eFirstDisplayRow = 1, // change this value to take more rows at top
    };
    CmdRspThread();

    //virtual void Run();
    //virtual void Create();

    virtual BOOLEAN CheckCmd( SecComm& secComm);
    virtual int UpdateDisplay(VID_DEFS who, int theLine);

    UINT32  m_frames;
protected:
    virtual void Process();           // Normally does not need to be overridden    
    virtual void RunSimulation();     // This is the real work horse    
    virtual void HandlePowerOff() {}  // default to no action for power off

    UINT32  m_elapsedProc;
    UINT32  m_elapsedProcZ1;
    UINT32  m_elapsedDisp;
    UINT32  m_maxDuration;
    UINT32  m_systemTick;
    UINT32  m_overrunCount;
    bool    m_updateDisplay;
};

#endif

