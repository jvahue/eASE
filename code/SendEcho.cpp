#include "SendEcho.h"
#include <deos.h>
#include <mem.h>

#include "video.h"

/****************************************************************************
 protected methods for FxProc
 ****************************************************************************/

void FxProc::Create()
{
    AseThread::Create("FxProc", "FxProcThreadTemplate");
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
    // Create my recv mailbox, grant any ASE thread to send.
    m_recvBox.Create("SendProcMB", sizeof(m_recvBuf),2);
    m_recvBox.GrantProcess("ASE");

    // Connect send box for sending to the EchoProc mailbox
    m_sendBox.Connect("ASE","EchoProcMB");

    // Create the thread for this object
    AseThread::Create("SendProc", "FxProcThreadTemplate");
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
    // Create my mailbox, allow any ASE thread to send.
    m_recvBox.Create("EchoProcMB",sizeof(m_recvBuf),2);
    m_recvBox.GrantProcess("ASE");

    // Connect to the SendProc mailbox
    m_sendBox.Connect("ASE","SendProcMB");

    // Create the thread for this object

    AseThread::Create("EchoProc", "FxProcThreadTemplate");
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