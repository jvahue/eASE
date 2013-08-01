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
      void Reset(void);

      // Receiver methods
      // called by MB owner to create and grant access to senders
      BOOLEAN Create (const char* mbName, UINT32 maxMsgSize,UINT32 maxQueDepth);
      BOOLEAN IssueGrant(const char* procName);
      BOOLEAN Receive(void* buff, UINT32 sizeBytes, BOOLEAN bWaitForMessage = FALSE);
      BOOLEAN IsCreated()
      {
        return (m_type == eRecv &&                        // mailbox initialized for receive
                m_hMailBox != NULL &&                     // valid handle obtained
                m_successfulGrantCnt == m_grantListSize); // all granted processes connected
      }

      // Sender methods
      BOOLEAN Connect(const char* procName, const char* mbName);
      BOOLEAN Send   (void* buff, UINT32 sizeBytes, BOOLEAN bBlockOnQueueFull = FALSE);
      BOOLEAN IsConnected()
      {
        // Mailbox init as sender and valid handle obtained
        return (m_type == eSend && m_hMailBox != NULL );
      }

      // Accessor methods
      processStatus  GetProcessStatus(){return m_procStatus;}
      const char*    GetProcessStatusString();

      ipcStatus    GetIpcStatus(){return m_ipcStatus;}
      const char*  GetIpcStatusString();

    protected:
      enum
      {
        eMaxGrantsAllowed  = 16, // Max senders to this mailbox
        eMaxProcNameLen    = 32,  // Max string len of Process name
        eMaxMailboxNameLen = 32
      };

      // Properties
      char  m_procName[eMaxProcNameLen];
      char  m_mailBoxName[eMaxMailboxNameLen];

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

      void AddSender(const char* procName);
      void OpenSenders(void);

    private:

};
#endif // CMAILBOX_H
