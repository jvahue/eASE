#include "AseThread.h"
#include <deos.h>

#include "video.h"

/****************************************************************************
 public methods
 ****************************************************************************/
AseThread::AseThread()
{
  //m_pCmd = pCmd;
}


void AseThread::Run()
{
  if (m_state == eNotCreated)
  {
  	Create();
  }
}
void AseThread::Suspend()
{

}
void AseThread::Stop()
{

}


AseThread::AseThreadState AseThread::GetRunState()
{
  return m_state;
}

/****************************************************************************
 protected methods
 ****************************************************************************/


void FxProc::Create()
{
  threadStatus ts;
  ts = createThread("FxProc", "FxProcThreadTemplate", ThreadFunc,
                                       (DWORD)this, &m_hThread);
}


void FxProc::Process()
{
    while (1)
    {
        m_ticks += 1;
        waitUntilNextPeriod();
    }
}
