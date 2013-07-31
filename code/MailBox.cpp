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
#include "video.h"

#include "Mailbox.h"
#include "AseCommon.h"

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
 * Function:    is_to_str
 *
 * Description: Convert a ipcStatus return value to a descriptive string
 *
 * Parameters:  [in] is: ipcStatus return value
 *
 * Returns:     CHAR* null terminated string describing is
 *
 * Notes:
 *
 ****************************************************************************/
const char* is_to_str(ipcStatus is)
{
    const char* is_strs[] = {
    "ipcValid",
    "ipcInvalidThread",
    "ipcInvalid",
    "ipcNoSuchProduceItem",
    "ipcInvalidSize",
    "ipcNoPreviousProducerToCopy",
    "ipcNoSuchConsumeItem",
    "ipcCouldNotAttachToProducer",
    "ipcQuotaExceeded",
    "ipcNoSuchMemoryObject",
    "ipcInsufficientVirtualAddressSpace",
    "ipcNoDataFromProducer",
    "ipcInvalidMessage",
    "ipcInvalidMessageLength",
    "ipcStaleMessage",
    "ipcOutOfMemory",
    "ipcInsufficientPrivilege",
    "ipcInvalidHandle",
    "ipcInvalidAccessType",
    "ipcInvalidProcessHandle",
    "ipcInvalidMemoryObjectHandle",
    "ipcInvalidMailboxHandle",
    "ipcInvalidEnvelopeHandle",
    "ipcEnvelopeInMailbox",
    "ipcQueueFull",
    "ipcNoMessage",
    "ipcEnvelopeQuotaExceeded",
    "ipcHandleMismatch",
    "ipcInsufficientEnvelopePrivilege",
    "ipcAddressNotPageAligned",
    "ipcOffsetNotPageAligned",
    "ipcDuplicateAlias",
    "ipcAnonymousProcessCantCreateMemoryObject",
    "ipcAbortedByException",
    "ipcMailboxDeleted",
    "ipcInsufficientRAM",
    "ipcAddressWrapAround",
    "ipcInvalidExtParSize",
    "ipcInvalidTimeoutSpecifier",
    "ipcInsufficientTime"
    };
    return is <= processInsufficientMainBudget ? is_strs[is] : "WTF?";
}
/*****************************************************************************
 * Function:    ps_to_str
 *
 * Description: Convert a processStatus return value to a descriptive string
 *
 * Parameters:  [in] ps: processStatus return value
 *
 * Returns:     CHAR* null terminated string describing ps
 *
 * Notes:
 *
 ****************************************************************************/
const char* ps_to_str(processStatus ps)
{
	const char* ps_strs[] = {   "processSuccess",
    "processInvalidHandle",
    "processInsufficientPrivilege",
    "processInvalidTemplate",
    "processInsufficientResourcesObsolete",
    "processNotActive",
    "processInvalidName",
    "processInsufficientSystemQuota",
    "processSourceAddressWrapAround",
    "processDestinationAddressWrapAround",
    "processSourcePageNotPresent",
    "processDestinationPageNotPresent",
    "processInsufficientProcessQuota",
    "processInsufficientBudget",
    "processInsufficientRAM",
    "processInsufficientThreadQuota",
    "processInsufficientMutexQuota",
    "processInsufficientEventQuota",
    "processInsufficientSemaphoreQuota",
    "processInsufficientMOQuota",
    "processInsufficientAMOQuota",
    "processInsufficientEnvelopeQuota",
    "processInsufficientMailboxQuota",
    "processInsufficientNameQuota",
    "processInsufficientPlatformResourceQuota",
    "processAMOSizeLargerThanParent",
    "processThreadStackLargerThanParent",
    "processScheduleBeforePeriodMismatch",
    "processDestinationPageDeleted",
    "processInsufficientRetainedBudget",
    "processNotMemoryAddress",
    "processArgumentNotOnStack",
    "processInsufficientArgumentQuota",
    "processArgumentVectorAllocationError",
    "processArgumentStringAllocationError",
    "processArgumentVectorCopyError",
    "processArgumentStringCopyError",
    "processWindowActivatedThreadMainPeriodMismatch", /* Reserved for WAT */
    "processNotZeroMainBudget",                       /* Reserved for WAT */
    "processInsufficientMainBudget"                   /* Reserved for WAT */
   };
   return ps <= processInsufficientMainBudget ? ps_strs[ps] : "WTF?";
}
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
BOOLEAN MailBox::Create(const char* mbName, UINT32 maxMsgSize,UINT32 maxQueDepth)
{
    BOOLEAN result = FALSE;

    strncpy(m_mailBoxName, mbName,  eMaxMailboxNameLen);

    m_ipcStatus = createMailbox(m_mailBoxName, maxMsgSize, maxQueDepth, &m_hMailBox);

    if(m_ipcStatus == ipcValid)
    {
        result = TRUE;
        m_type = eRecv;
    }
        return result;
    }


/*****************************************************************************
 * Function:    IssueGrant
 *
 * Description:  Grants access to a process to sending msgs to mailbox
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
BOOLEAN MailBox::IssueGrant(const char* procName)
{
    BOOLEAN result = TRUE;

    if (m_type == eRecv)
    {
        AddSender(procName);
    }// switch on mailbox type
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

    switch (m_type)
    {
        case eUndefined:
            // First time attempting to set up a sender mailbox obj...
            strncpy(m_procName,    procName, eMaxProcNameLen);
            strncpy(m_mailBoxName, mbName,   eMaxMailboxNameLen);
            m_type = eSend;
            //
            //Deliberate fallthrough...
            //
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
                }
            }
		    else
		    {
                // getProcessHandle failed, report.
			    m_hProcess = NULL;
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
BOOLEAN MailBox::Receive(void* buff, UINT32 sizeBytes, BOOLEAN bWaitForMessage)
{

    // If there were any sender processes which could not be granted access,
    // try to connect them each time we do a read.
    if (m_successfulGrantCnt < m_grantListSize )
    {
        OpenSenders();
    }

    // Read the mailbox
    m_ipcStatus = receiveMessage(m_hMailBox,
                                 buff,
                                 BYTES_TO_DWORDS(sizeBytes),
                                 bWaitForMessage);
    return (m_ipcStatus == ipcValid);
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
 * Returns:    TRUE if ipcStatus is ipcValid, otherwise FALSE
 *
 * Notes:
 *
 ****************************************************************************/
BOOLEAN MailBox::Send(void* buff, UINT32 sizeBytes, BOOLEAN bBlockOnQueueFull)
{
    BOOLEAN result = TRUE;

    // try to (re-)connect as necessary
    if ( !IsConnected() )
    {
        Connect(m_procName, m_mailBoxName);
    }

    if (IsConnected())
    {
        m_ipcStatus = sendMessage(m_hMailBox,
                                         buff,
                                         BYTES_TO_DWORDS(sizeBytes),
                                         bBlockOnQueueFull);
    }
    return (m_ipcStatus == ipcValid);
}


const char* MailBox::GetProcessStatusString()
{
	return (const char*)ps_to_str(GetProcessStatus() );
}

const char* MailBox::GetIpcStatusString()
{
	return is_to_str(GetIpcStatus());
}

void MailBox::Reset()
{
    SIGNED32  i;
    // Reset my connections based on my config ( sender/receiver)

    switch(m_type)
    {
    case eRecv:
        if (m_successfulGrantCnt != 0)
        {
            // Reset the flag. This will force the receive method
            // to reconnect before reading from src-proc
            for (i = 0; i < eMaxGrantsAllowed; ++i)
            {
                m_grantList[i].bConnected = FALSE;
            }
            m_successfulGrantCnt = 0;
        }
        break;

    case eUndefined:
    case eSend:
        if (m_hMailBox != NULL)
        {
            m_ipcStatus = ipcInvalid;
            m_hMailBox = NULL;
        }
        break;
    default:
        break;
    }
}



/***********************************************************************************
 *  Protected methods
 **********************************************************************************/


/*****************************************************************************
 * Function:    AddSender
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
void MailBox::AddSender(const char* procName)
{
  // Add the allowed-sender to the process list.
  // NOTE: NOT CHECKING FOR DUPLICATE ATTEMPTS!!
  strncpy(&m_grantList[m_grantListSize].ProcName[0], procName, eMaxProcNameLen);
  m_grantListSize++;

  // Attempt to grant this and any other unsuccessful sender processes.
  OpenSenders();

  return;
}

/*****************************************************************************
 * Function:    OpenSenders
 *
 * Description:  Open/grant other processes access to my mailbox.
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
void MailBox::OpenSenders(void)
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
            }
        }// Process not connected
    }// for grant list table.
}
