#include "AseThread.h"
#include <deos.h>

/****************************************************************************
 public methods
 ****************************************************************************/
AseThread::AseThread(int* pCmd)
{
  m_pCmd = pCmd;
}

AseThread::~AseThread()
{}

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
 // Faux-Proc unique processing.
}
