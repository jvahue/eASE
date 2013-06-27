//
// File: SendEcho.h
/*
* Description: Implements a Send/Echo Thread pair for testing mailbox IPC
* The SendProc thread creates a send and listen mailbox and send an incrementing
* token to the EchoProc thread when it determines that the previous  token has
*  been successfully returned.
*  The EchoProc reads for a token from the SendProc and verifies it
*  contains the previous token value + 1. If so it echos the value back to
*  SendProc and stores it for comparison against the next value.
*/

// Includes OS
//

// Includes PWC
#include "alt_stdtypes.h"
#include "procapi.h"

#include "AseThread.h"
#include "MailBox.h"

/****************************************************************************
 public methods
 ****************************************************************************/

/**********************************************************************/
class SendProc : public AseThread
{
    public:
        virtual void Create();  // override the AseThread::Create
        UNSIGNED32 m_ticks;
        UNSIGNED32 m_lastTick;
        UNSIGNED32 m_token;

        BOOLEAN m_bAcked;
        UNSIGNED32 m_sent;

        MailBox m_recvBox;
        MailBox m_sendBox;

        UINT32 m_recvBuf[64];
        UINT32 m_sendBuf[64];

    protected:
        // Methods
        virtual void Process(); // override the AseThread::Process
};
/**********************************************************************/
class EchoProc : public AseThread
{
    public:
        virtual void Create();  // override the AseThread::Create
        UNSIGNED32 m_token;

        MailBox m_recvBox;
        MailBox m_sendBox;
        UINT32 m_recvBuf[64];
        UINT32 m_sendBuf[64];

    protected:
        // Methods
        virtual void Process(); // override the AseThread::Process
};


/**********************************************************************/
class FxProc : public AseThread
{
    protected:
      // Methods
      virtual void Process(); // override the AseThread::Process

  public:
      virtual void Create();  // override the AseThread::Create
      UNSIGNED32 m_ticks;
};
