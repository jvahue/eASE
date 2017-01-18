/******************************************************************************
Copyright (C) 2013-2017 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        AseThread.cpp

Description: This file implements the base ASE Thread class, all ASE threads are derived from
this class

VERSION
$Revision: $  $Date: $

******************************************************************************/
#include <deos.h>
#include <mem.h>

#include "AseCommon.h"

#include "AseThread.h"

/****************************************************************************
public methods
****************************************************************************/
AseThread::AseThread()
: m_hThread(0)
, m_state(eNotCreated)
, m_pCommon(NULL)
{
}

//---------------------------------------------------------------------------------------------
// Function: Launch
// Description: Spawn the thread and check for creation
// Parameters:
// name (i): the name of the thread
// tName (i): the template name
//
void AseThread::Launch(const CHAR* name, const CHAR* tName)
{
    threadStatus ts;

    ts = createThread(name, tName, ThreadFunc, (DWORD)this, &m_hThread);
    if (ts != threadSuccess)
    {
        m_state = eError;
    }
    else
    {
        m_state = eRun;
    }
}

AseThread::AseThreadState AseThread::GetRunState()
{
    return m_state;
}
