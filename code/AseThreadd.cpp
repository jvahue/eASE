#include "AseThread.h"
#include <deos.h>
#include <mem.h>

#include "video.h"

/****************************************************************************
 public methods
 ****************************************************************************/
AseThread::AseThread()
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
AseThread::Create(const CHAR* name, const CHAR* tName, void* data)
{
    threadStatus ts;

    ts = createThread(name, tName, ThreadFunc, (DWORD)data, &m_hThread);
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

/****************************************************************************
 protected methods for FxProc
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

/****************************************************************************
 protected methods for SendProc
 ****************************************************************************/

void SendProc::Create()
{
    threadStatus ts;


    // Create my recv mailbox, grant any ASE thread to send.
    m_recvBox.Create("SendProcMB", sizeof(m_recvBuf),2);
    m_recvBox.GrantProcess("ASE");

    // Connect send box for sending to the EchoProc mailbox
    m_sendBox.Connect("ASE","EchoProcMB");

    ts = createThread("SendProc", "FxProcThreadTemplate", ThreadFunc,
                                       (DWORD)this, &m_hThread);
}


void SendProc::Process()
{
    UNSIGNED32 interval = 50;
    m_ticks  = 0;
    m_token = 0;
    m_bAcked = TRUE;
    m_lastTick = 0;

    while (1)
    {
        m_ticks += 1;
        if(m_bAcked == TRUE)
        {
            if (m_ticks > m_lastTick + interval)
            {
                m_lastTick   = m_ticks;

                m_sendBuf[0] = ++m_token;
                if (ipcValid == m_sendBox.Send((void*)m_sendBuf, sizeof(m_sendBuf)) )
                {
                    // sent ok, wait for ack
                    m_bAcked = FALSE;
                }
            }
        }
        else // we are waiting for response.. check if recv box has the
         // echo of the sent token
        {
            if(m_recvBox.IsCreated() &&
               ipcValid == m_recvBox.Receive(m_recvBuf, sizeof(m_recvBuf)) &&
               m_recvBuf[0] == m_token )
            {
                // signal acked was received.
                m_bAcked = TRUE;
            }
        }

    waitUntilNextPeriod();
    }//while forever
}

/****************************************************************************
 protected methods for EchoProc
 ****************************************************************************/

void EchoProc::Create()
{
    threadStatus ts;

    // Create my mailbox, allow any ASE thread to send.
    m_recvBox.Create("EchoProcMB",sizeof(m_recvBuf),2);
    m_recvBox.GrantProcess("ASE");

    // Connect to the SendProc mailbox
    m_sendBox.Connect("ASE","SendProcMB");

    ts = createThread("EchoProc", "FxProcThreadTemplate", ThreadFunc,
                                            (DWORD)this, &m_hThread);
}

void EchoProc::Process()
{
    m_token = 0;
    memset(m_recvBuf,0,sizeof(m_recvBuf));
        while (1)
        {
            if ( m_recvBox.IsCreated() &&
                 ipcValid == m_recvBox.Receive(m_recvBuf,sizeof(m_recvBuf)))
            {
                // if new value received, echo it back so sender will send a new value
                if (m_recvBuf[0] ==  m_token + 1 )
                {
                    m_token      = m_recvBuf[0];
                    m_sendBuf[0] = m_token;
                    m_sendBox.Send(m_sendBuf, sizeof(m_sendBuf));
                }
            }
            waitUntilNextPeriod();
        }
}
