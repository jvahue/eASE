#include "AseThread.h"
#include <deos.h>
#include <mem.h>

#include "video.h"

/****************************************************************************
 public methods
 ****************************************************************************/
AseThread::AseThread()
    : m_hThread(0)
    , m_pCommon(NULL)
    , m_state(eNotCreated)
    , m_overrunCount(0)
{
}


//-------------------------------------------------------------------------------------------------
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

