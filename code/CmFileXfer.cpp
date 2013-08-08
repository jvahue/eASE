//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: CmFileXfer.cpp
//
//    Description: Implement the file transfer protocol
//
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include <string.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "CmFileXfer.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
static const char* modeNames[] = {
    "Idle",         //
    "AckTimeout",   //
    "FileWait",     //
    "FileOffload",  //
    "FileValidate"  //
};

//----------------------------------------------------------------------------/
// Local Function Prototypes                                                 -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Public Functions                                                          -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Class Definitions                                                         -/
//----------------------------------------------------------------------------/
//-------------------------------------------------------------------------------------------------
// Function: CmFileXfer
// Description: Implement the file transfer protocol
//
CmFileXfer::CmFileXfer()
    : m_fileXferRequested(false)
    , m_mode(eXferIdle)
    , m_modeTimeout(0)
    , m_fileXferRqsts(0)
    , m_fileXferMsgs(0)

    , m_tcAckDelay(0)
    , m_tcAckStatus(CM_ACK)
    , m_tcAckInfo(CM_FILE_XFR)

{
    memset(m_xferFileName, 0, sizeof(m_xferFileName));
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Command Handler for CmFileXfer
//
BOOLEAN CmFileXfer::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eLogFileReady:
        strcpy( secComm.m_response.streamData, m_xferFileName);
        secComm.m_response.streamSize = strlen(m_xferFileName);
        m_mode = eXferFileOffload;

        serviced = TRUE;
        break;

    case eLogFileCrc:
        m_fileCrc = request.variableId;
        m_fileCrcAck = request.sigGenId ? CM_XFR_ACK : CM_XFR_NACK;
        m_mode = eXferFileCheck;

        serviced = TRUE;
        break;

    default:
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("CmFileXfer");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessFileXfer
// Description: Implement the file transfer protocol
//
void CmFileXfer::ProcessFileXfer(bool msOnline, MailBox& in, MailBox& out)
{
    FILE_RCV_MSG rcv;
    FILE_CONFIRM_MSG* pConfirm;

    m_msOnline = msOnline;

    // we can recieve a msg at any time
    if (in.Receive(&rcv, sizeof(rcv)))
    {
        m_fileXferMsgs += 1;
        FileXferResponse(rcv, out);
    }

    switch (m_mode)
    {
    case eXferIdle:          // no activity
        break;

    case eXferAckTimeout:    // ready to respond with an (N)ACK after the timeout
        SendAck(out);
        break;

    case eXferFileWait:      // wait for ePySte to request the file
        m_tcAckInfo = m_msOnline ? CM_QUEUED_MS_OK : CM_QUEUED_MS_OFFLINE;
        break;

    case eXferFileOffload:   // stream file up to ePySte to verify the CRC
        m_tcAckInfo = CM_FILE_XFR;
        break;

    case eXferFileCheck:     // ePySTe returned the CRC
        pConfirm = (FILE_CONFIRM_MSG*)&m_sendMsgBuffer;

        pConfirm->msgId = CM_ID_CONFIRM;
        strcpy(pConfirm->filename, m_xferFileName);
        pConfirm->xfrResult = CM_XFR_ACK;
        pConfirm->msCrc = m_fileCrc;

        if (out.Send(&m_sendMsgBuffer, sizeof(m_sendMsgBuffer)))
        {
            m_mode = eXferFileValid;
        }

        break;

    case eXferFileValid:     // wait for ADRF to validate the CRC

        break;

    default:
        break;
    };
}


//-------------------------------------------------------------------------------------------------
// Function: FileXferResponse
// Description: Handle responding to the client
//
void CmFileXfer::FileXferResponse(FILE_RCV_MSG& rcv, MailBox& out)
{
    if (rcv.msgId == CM_ID_XFR)
    {
        FILE_XFR_MSG* pXfrMsg = (FILE_XFR_MSG*)&rcv;

        // we are being asked to start transfer
        if (m_xferFileName[0] == '\0' || pXfrMsg->xfrOverride)
        {
            // ok we are not in the middle of one accept it
            memcpy(m_xferFileName, pXfrMsg->filename, CM_FILE_NAME_LEN);

            // send ACK or delay
            m_tcAckInfo = m_msOnline ? CM_QUEUED_MS_OK : CM_QUEUED_MS_OFFLINE;

            // reset us to idle mode for the call
            m_mode = eXferIdle;
            SendAck(out);

            m_fileXferRequested = true;
            m_fileXferRqsts += 1;
        }

        // no override so we only respond if its the same file name
        else if (strcmp(m_xferFileName, pXfrMsg->filename) == 0)
        {
            // requesting status on the file transfer
            SendAck(out);
        }
    }
    else if (rcv.msgId == CM_ID_CRC_VAL)
    {
        // make sure we are doing something
        if (m_xferFileName[0] != '\0' && m_mode == eXferFileValid)
        {
            memset(&m_sendMsgBuffer, 0, sizeof(m_sendMsgBuffer));

            // make sure they have the right name
            FILE_CRC_VAL_MSG* pCrcValMsg = (FILE_CRC_VAL_MSG*)&rcv;
            if (CM_CRC_ACK == pCrcValMsg->crcValid)
            {
                // send Complete Message
                FILE_COMPLETE_MSG* pCompleteMsg = (FILE_COMPLETE_MSG*)&m_sendMsgBuffer;
                strncpy(pCompleteMsg->filename, pCrcValMsg->filename, MAX_MSG_LEN);
                pCompleteMsg->msgId = CM_ID_COMPLETE;

                out.Send(&m_sendMsgBuffer, sizeof(m_sendMsgBuffer));

                m_fileXferRequested = false;
                memset(m_xferFileName, 0, sizeof(m_xferFileName));
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Function: FileXferResponse
// Description: Handle responding to the client
//
void CmFileXfer::SendAck( MailBox& out)
{
    FILE_ACK_MSG ack;

    if ((m_mode == eXferIdle) && (m_tcAckDelay > 0))
    {
        // init the timeout
        m_mode = eXferAckTimeout;
        m_modeTimeout = m_tcAckDelay;
    }
    else if (m_mode == eXferAckTimeout)
    {
        if (m_modeTimeout > 0)
        {
            m_modeTimeout -= 1;
        }
        else
        {
            ack.msgId = CM_ID_ACK;
            strcpy( ack.filename, m_xferFileName);
            ack.ackStatus = m_tcAckStatus;
            ack.ackInfo = m_tcAckInfo;

            // we can send it
            out.Send(&ack, sizeof(ack));
            m_mode = eXferFileWait;
        }
    }
    else
    {
        ack.msgId = CM_ID_ACK;
        strcpy( ack.filename, m_xferFileName);
        ack.ackStatus = m_tcAckStatus;
        ack.ackInfo = m_tcAckInfo;

        // we can send it
        out.Send(&ack, sizeof(ack));
    }
}


//-------------------------------------------------------------------------------------------------
// Function: GetMode
// Description: Display string for current mode
//
const char* CmFileXfer::GetModeName() const
{
    return modeNames[m_mode];
}
