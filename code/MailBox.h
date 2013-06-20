#ifndef CMAILBOX_H
#define CMAILBOX_H
/******************************************************************************
              Copyright (C) 2012-2013 Pratt & Whitney Engine Services, Inc.
                 All Rights Reserved. Proprietary and Confidential.

    File:        MailBox.h

    Description:

    VERSION
    $Revision: 3 $  $Date: 3/13/13 2:06p $

******************************************************************************/

#include <deos.h>
#include "alt_stdtypes.h"

class MailBox
{
  public:
    enum MailBoxType
    {
      eUndefined, // Mailbox obj not yet initialized.
      eRecv,      // Mailbox obj created by owner for reading.
      eSend       // Mailbox obj connected by an allowed process for sending
    };


    // Constructor.
    MailBox(void);

    // Initialization
    BOOLEAN Create (const char* procName, UINT32 maxMsgSize,UINT32 maxQueDepth);// Called by MB owner to create and grant access to senders
    BOOLEAN Connect(const char* procName, const char* mbName = NULL);

    //
    INT32 Receive(void* buff, UINT32 sizeBytes, BOOLEAN bWaitForMessage = FALSE);
    INT32 Send   (void* buff, UINT32 sizeBytes, BOOLEAN bBlockOnQueueFull = FALSE);

    // Accessor methods
    INT32   GetProcessStatus(){return (int)m_procStatus;}
    INT32   GetIpcStatus()    {return (int)m_ipcStatus;}
    BOOLEAN IsConnected()     {return (m_hMailBox != NULL && m_type == eSend);}
    BOOLEAN IsCreated()       {return (m_hMailBox != NULL && m_type == eRecv);}

  protected:
    enum
    {
      eMaxGrantsAllowed = 16, // Max senders to this mailbox
      eMaxProcNameLen   = 32  // Max string len of Process name
    };

    // Properties
    char  m_procName[eMaxProcNameLen];
    char  m_mailBoxName[eMaxProcNameLen];

    struct AccessGrantList
    {
      char ProcName[eMaxProcNameLen];
      BOOLEAN bConnected;
    };

    AccessGrantList m_grantList[eMaxGrantsAllowed];
    INT32           m_grantListSize;
    INT32           m_successfulGrantCnt;

    MailBoxType      m_type;

    process_handle_t m_hProcess;
    processStatus    m_procStatus;
    ipcMailboxHandle m_hMailBox;
    ipcStatus        m_ipcStatus;

    void AllowSender(const char* procName);
    void ConnectSenders(void);

  private:

};
#endif // CMAILBOX_H