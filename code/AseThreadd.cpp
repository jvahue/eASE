#include "AseThread.h"
#include <deos.h>
#include <mem.h>

#include "video.h"

/****************************************************************************
 public methods
 ****************************************************************************/
AseThread::AseThread()
    : m_hThread(0)
    , m_state(eNotCreated)
{
}


void AseThread::Run()
{
    if (m_state == eNotCreated)
    {
        Create();
    }
}

//-------------------------------------------------------------------------------------------------
// Function: Create
// Description: Spawn the thread and check for creation
// Parameters:
// name (i): the name of the thread
// tName (i): the template name
// data (i): pointer to data to pass to the thread
//
void AseThread::Create(const CHAR* name, const CHAR* tName)
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

