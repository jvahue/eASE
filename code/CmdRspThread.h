#ifndef CmdRspThread_h
#define CmdRspThread_h
/******************************************************************************
            Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
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
#include "alt_stdTypes.h"
#include "AseThread.h"
#include "SecComm.h"

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
    CmdRspThread();

    //virtual void Run();
    //virtual void Create();

    virtual BOOLEAN CheckCmd( SecComm& secComm);

protected:
    virtual void Process();
    // Normally does not need to be overridden
    virtual void RunSimulation();
    // This is the real work horse
    virtual void HandlePowerOff() {}

};

#endif

