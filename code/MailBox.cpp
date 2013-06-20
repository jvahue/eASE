#define CMAILBOX
/******************************************************************************
              Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
                 All Rights Reserved. Proprietary and Confidential.

    File:        CMailBox.cpp

    Description:

    VERSION
    $Revision:  $  $Date: 13-02-26 9:32a $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/


/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include <videobuf.h>
#include <string.h>

#include "Mailbox.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define BYTES_TO_DWORDS(x) ((x / sizeof(UINT32))+1)
//#define MAILBOX_DEBUG

#ifdef MAILBOX_DEBUG
#include "video.h"
#define DEBUG_STR(...) debug_str(1,__VA_ARGS__)
#define DEBUG_STR0(...) debug_str(0,__VA_ARGS__)
#define DEBUG_STR2(...) debug_str(2,__VA_ARGS__)
CHAR* ps_to_str(processStatus ps);
CHAR* is_to_str(ipcStatus is);

#else
#define DEBUG_STR(...)
#define DEBUG_STR0(...)
#define DEBUG_STR2(...)
#endif

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/


/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************
 * Function:    Mailbox
 *
 * Description:  MailBox class constructor
 *
 * Parameters:   none
 *
 * Returns:      None
 *
 * Notes:
 *
 ****************************************************************************/
MailBox::MailBox(void)
  :m_hProcess(NULL)
  ,m_procStatus(processInvalidHandle)
  ,m_hMailBox(NULL)
  ,m_ipcStatus(ipcNoSuchProduceItem)
  ,m_type(eUndefined)
  ,m_grantListSize(0)
  ,m_successfulGrantCnt(0)
{
  // Init the access grant status list.
  // this is only used by creators(owners)
  // to manage grants to processes which will write to my mailbox.
  for (int i = 0; i < eMaxGrantsAllowed; ++i)
  {
    m_grantList[i].ProcName[0] = '\0';
    m_grantList[i].bConnected  = FALSE;
  }
}

/******************************************************************************************
 *  Public methods
 *****************************************************************************************/

/*****************************************************************************
 * Function:    Create
 *
 * Description:  Creates a Mail Box object for receiving messages sent from
 *               processes which have been granted access via a subsequent call
 *               to "Connect" on this mailbox object.
 *
 * Parameters:   procName - string containing the name of the owning process
 *               maxMsgSize
 *
 * Returns:      True if success, otherwise false
 *
 * Notes:
 *
 ****************************************************************************/
BOOLEAN MailBox::Create(const char* procName, UINT32 maxMsgSize,UINT32 maxQueDepth)
{
  BOOLEAN result = FALSE;

  strncpy(m_procName, procName,  sizeof(eMaxProcNameLen));

  m_ipcStatus = createMailbox(m_procName, maxMsgSize, maxQueDepth, &m_hMailBox);

  DEBUG_STR0("Call to createMailbox %s returned %s\n", m_mailBoxName, is_to_str(m_ipcStatus));

  if(m_ipcStatus == ipcValid)
  {
    result = TRUE;
    m_type = eRecv;
  }
  return result;
}

/*****************************************************************************
 * Function:    Connect
 *
 * Description:  Connect to a Mailbox for sending, OR...
 *               grants access to a process to sending msgs to mailbox
 *               created by this process via a preceding 'create' call.
 *
 * Parameters:   procName - string containing the name of the process which owns
 *                          the mailbox.
 *               mbName   - string containing the name of the mailbox.
 *
 * Returns:      True if success, otherwise false
 *
 * Notes:
 *
 ****************************************************************************/
BOOLEAN MailBox::Connect(const char* procName, const char* mbName)
{
  BOOLEAN result = TRUE;

  // If this mailbox is being created by the owner, then grant write/send access to the
  //  specified process.

  if(0 != strlen(procName))
  {
    result = FALSE;
  }

  switch (m_type)
  {
    // If mailbox handle is set and 'this' is a mailbox owner(eRecv),
    // grant access for the specified sender process.
    case eRecv:
      if (m_hMailBox != NULL)
      {
        AllowSender(procName);
      }
      break;

    case eUndefined:
      // We are setting up a sender mailbox obj...
      strncpy(m_procName,    procName, sizeof(m_procName));
      strncpy(m_mailBoxName, mbName,   sizeof(m_mailBoxName));
      m_type = eSend;
      //Deliberate fallthrough...

    case eSend:
      // Get Process handle for the mailbox owner
      m_procStatus = getProcessHandle(m_procName, &m_hProcess);
      if( m_procStatus == processSuccess)
      {
        //Get mailbox handle/attach as sender.
        m_ipcStatus = getMailboxHandle(m_mailBoxName, m_hProcess, &m_hMailBox);
        if(m_ipcStatus != ipcValid)
        {
          m_hMailBox = NULL;
          DEBUG_STR0("Call to getMailboxHandle %s returned %s\n",
                                 m_mailBoxName, is_to_str(m_ipcStatus));
        }
      }
      else
      {
        // getProcessHandle failed, report.
        m_hProcess = NULL;
        DEBUG_STR0("Call to getProcessHandle %s returned %s\n", m_procName,
                         ps_to_str(m_procStatus));
      }

      if((m_ipcStatus != ipcValid) || (m_procStatus != processSuccess))
      {
        result = FALSE;
      }
      break;

  default:
    break;
  }// switch on mailbox type
  return result;
}

/*****************************************************************************
 * Function:    Receive
 *
 * Description: Call to read msgs on my mailbox
 *
 * Parameters: buff            - pointer to memory area to receive incoming msg
 *             sizeBytes       - the number of bytes of input
 *             bWaitForMessage - flag signaling whether call should block until
 *                               msg available. Default is FALSE
 * Returns:    ipcStatus of the call
 *
 * Notes:
 *
 ****************************************************************************/
INT32 MailBox::Receive(void* buff, UINT32 sizeBytes, BOOLEAN bWaitForMessage)
{
  if (m_type == eRecv)
  {
    // If there were any sender processes which could not be granted access,
    // try to connect them each time we do a read.
    if (m_successfulGrantCnt < m_grantListSize )
    {
      ConnectSenders();
    }

    // Read the mailbox
    m_ipcStatus = receiveMessage(m_hMailBox,
                                 buff,
                                 BYTES_TO_DWORDS(sizeBytes),
                                 bWaitForMessage);
  }
  else
  {
    // Only the creator/owner should be doing reads!
    m_ipcStatus = ipcInvalid;
  }

  return (INT32)m_ipcStatus;
}

/*****************************************************************************
 * Function:    Send
 *
 * Description: Call to send msg to the  associated mailbox
 *
 * Parameters: buff            - pointer to memory area containing msg to send
 *             sizeBytes       - the number of bytes of output
 *             bWaitForMessage - flag signalling whether call should block until
 *                               out-queue has space available to send msg.
 *                                Default is FALSE(i.e. return immediately
 *                                with 'ipcQueueFull'
 * Returns:    ipcStatus of the call
 *
 * Notes:
 *
 ****************************************************************************/
INT32 MailBox::Send(void* buff, UINT32 sizeBytes, BOOLEAN bBlockOnQueueFull)
{
  if ( !IsConnected() )
  {
    Connect(m_procName, m_mailBoxName);
  }

  if (IsConnected())
  {
    m_ipcStatus = sendMessage(m_hMailBox, buff, BYTES_TO_DWORDS(sizeBytes), bBlockOnQueueFull);
  }

  return (INT32)m_ipcStatus;
}

/***********************************************************************************
 *  Protected methods
 **********************************************************************************/


/*****************************************************************************
 * Function:    AllowSender
 *
 * Description:  Connect/grant send-access to the process 'procName'
 *
 * Parameters:   procName - string containing the name of the owning process
 *
 * Returns:      True if success, otherwise false
 *
 * Notes:
 *
 ****************************************************************************/
void MailBox::AllowSender(const char* procName)
{
  // Add the allowed-sender to the process list.
  // NOTE: NOT CHECKING FOR DUPLICATE ATTEMPTS!!
  strncpy(m_grantList[m_grantListSize].ProcName, procName, eMaxProcNameLen);
  m_grantListSize++;

  // Try to grant/connect this and any un-connected senders.
  ConnectSenders();

  return;
}

/*****************************************************************************
 * Function:    ConnectSenders
 *
 * Description:  Connect/grant other processes access to my mailbox.
 *               This method is called by the MB owner to check the grant list
 *               for processes which have not been connected and attempts
 *               to get handle and grant auth.
 *               This method is called during setup and also during Read call
 *
 * Parameters:   None
 *
 * Returns:      None
 *
 * Notes:
 *
 ****************************************************************************/
void MailBox::ConnectSenders(void)
{
  process_handle_t procHandle;
  processStatus    procStatus;
  ipcMailboxHandle mailBoxHndl;
  ipcStatus        ipcStat;
  INT32 i;

  for (i = 0; (i < m_grantListSize) &&
              (m_successfulGrantCnt < m_grantListSize );
              ++i)
  {
    // For each allowed process not yet connected, find the process and grant
    // access to my mailbox
    if ( !m_grantList[i].bConnected )
    {
      // Get a handle to the sender process
      procStatus = getProcessHandle(m_grantList[i].ProcName, &procHandle);

      if(procStatus == processSuccess)
      {
        ipcStat = grantMailboxAccess(m_hMailBox, procHandle, FALSE);
        if (ipcStat == ipcValid)
        {
          m_grantList[i].bConnected = TRUE;
          m_successfulGrantCnt++;
        }
        DEBUG_STR0("grantMailboxAccess to %s:  %s\n", procName, is_to_str(ipcStatus));
      }
      else
      {
        DEBUG_STR0("getProcessHandle %s is %s\n", procName, ps_to_str(ps));
      }
    }// Process not connected
  }// for grant list table.
}
