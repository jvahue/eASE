//
// File: AseThread.h

// Description: Implements the socket connection for the low level driver code
// This is the client side of the socket
//
// The AseSocket creates two threads that receive and transmit data.  These threads
// fill in data within this object.  The main functions are:
// Create - Spawn threads
// Connect - attempt to get a connection
// Read - read data from the thread - standard command msgs from the PySTE are supported
// Send - send data
// Close - close the connection
//
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
