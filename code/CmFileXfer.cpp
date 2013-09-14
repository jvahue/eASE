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
    "Idle",   // eXferIdle
    "Rqst",   // eXferRqst
    "Xfer",   // eXferFile
    "Fail",   // eXferFileFail
    "CRC",    // eXferFileCrc
    "Valid?"  // eXferFileValid
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
//-----------------------------------------------------------------------------
// Function: CmFileXfer
// Description: Implement the file transfer protocol
//
CmFileXfer::CmFileXfer()
    : m_mode(eXferIdle)
    , m_modeTimeout(0)
    , m_fileXferMsgs(0)

    , m_fileXferRqsts(0)
    , m_fileXferServiced(0)
    , m_fileXferSuccess(0)
    , m_fileXferFailed(0)
    , m_fileXferFailLast(0)

    , m_tcAckDelay(0)
    , m_tcAckStatus(CM_ACK)
    , m_tcAckInfo(CM_FILE_XFR)

    , m_fileCrc(0)
    //, m_fileCrcAck(0)

    , m_msOnline(false)

{
  memset(m_xferFileName, 0, sizeof(m_xferFileName));
  memset((void*)&m_sendMsgBuffer, 0, sizeof(m_sendMsgBuffer));
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Command Handler for CmFileXfer
//
BOOLEAN CmFileXfer::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eLogFileReady:
        if (m_mode == eXferRqst)
        {
            strcpy( secComm.m_response.streamData, m_xferFileName);
            secComm.m_response.streamSize = strlen(m_xferFileName);
            secComm.m_response.successful = true;
        }
        else
        {
            secComm.m_response.successful = false;
        }

        serviced = TRUE;
        break;

    case eLogFileCrc:
        if (m_mode == eXferFile)
        {
            m_fileCrc = request.sigGenId;
            //m_fileCrcAck = request.variableId ? CM_XFR_ACK : CM_XFR_NACK;
            m_mode = eXferFileCrc;
            secComm.m_response.successful = true;
            m_fileXferServiced += 1;
        }
        else
        {
            secComm.m_response.successful = false;
        }

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

    // we can receive a msg at any time
    if (in.Receive(&rcv, sizeof(rcv)))
    {
        m_fileXferMsgs += 1;
        FileXferResponse(rcv, out);
    }

    switch (m_mode)
    {
    case eXferIdle:          // no activity
        m_xferFileName[0] = '\0';
        break;

    case eXferRqst:
        // wait for ePySte to ask for the m_xferFile
        SendAckTimeout(out);
        break;

    case eXferFile:      // File is being transferred
        m_tcAckInfo = CM_FILE_XFR;
        break;

    case eXferFileFail:   // CmProc was unable to open the request file to send to ePySte
        if (m_modeTimeout > 0)
        {
            m_modeTimeout -= 1;
        }
        else
        {
            m_mode = eXferIdle;
            m_xferFileName[0] = '\0';
        }
        break;

    case eXferFileCrc:     // ePySTe returned the CRC
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

    case eXferFileValid:
        // wait for ADRF to validate the CRC, see FileXferResponse
        break;

    // TODO: could add one more state to delay the COMPLETE response from us back to the ADRF

    default:
        break;
    };
}

//-------------------------------------------------------------------------------------------------
// Function: FileXferResponse
// Description: Handle responding to the client
//
// TODO: handle aborted transfer override=On and filename = NULL
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

            // set ACK Info
            m_tcAckInfo = m_msOnline ? CM_QUEUED_MS_OK : CM_QUEUED_MS_OFFLINE;

            // indicate we have a file ready to go
            m_mode = eXferRqst;
            m_modeTimeout = m_tcAckDelay;
            SendAckTimeout(out);

            m_fileXferRqsts += 1;
        }

        // no override so we only respond if its the same file name
        else if (strcmp(m_xferFileName, pXfrMsg->filename) == 0)
        {
            // requesting status on the file transfer
            SendAck(out);
            if (m_mode == eXferFileFail)
            {
                // we just told the ADRF about the bad file go to idle
                m_mode = eXferIdle;
            }
        }
        else
        {
            // TODO: add a counter of bad filename requests
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
                strncpy(pCompleteMsg->filename, pCrcValMsg->filename, CM_FILE_NAME_LEN);
                pCompleteMsg->msgId = CM_ID_COMPLETE;

                out.Send(&m_sendMsgBuffer, sizeof(m_sendMsgBuffer));

                m_fileXferSuccess += 1;
            }
            else
            {
                // keep track of fail msgs
                if (CM_CRC_NACK == pCrcValMsg->crcValid)
                {
                    m_fileXferFailed += 1;
                }
                else if (CM_CRC_NACK_LAST == pCrcValMsg->crcValid)
                {
                    m_fileXferFailLast += 1;
                }
            }

            // clear rqst filename and return to idle mode
            memset(m_xferFileName, 0, sizeof(m_xferFileName));
            m_mode = eXferIdle;
        }
        else
        {
            // unexpected CRC Validation msg

        }
    }
}

//-------------------------------------------------------------------------------------------------
// Function: SendAckCtrl
// Description: Control sending Ack responses to the client
//
void CmFileXfer::SendAckTimeout(MailBox& out)
{
    if (m_modeTimeout > 0)
    {
        m_modeTimeout -= 1;
    }
    else
    {
        SendAck(out);
    }
}

//-------------------------------------------------------------------------------------------------
// Function: SendAck
// Description: Actually send the Ack
//
void CmFileXfer::SendAck(MailBox& out)
{
    FILE_ACK_MSG ack;
    ack.msgId = CM_ID_ACK;
    strcpy( ack.filename, m_xferFileName);
    ack.ackStatus = m_tcAckStatus;
    ack.ackInfo = m_tcAckInfo;

    // we can send it
    out.Send(&ack, sizeof(ack));
}

//-------------------------------------------------------------------------------------------------
// Function: FileStatus
// Description: Indicate a files status from CmProc - this only cares if the filename passed in
// matches the xfer filename
//
void CmFileXfer::FileStatus(char* filename, bool canOpen)
{
    if (strcmp(filename, m_xferFileName) == 0)
    {
        if (canOpen)
        {
            m_mode = eXferFile;
        }
        else
        {
            m_mode = eXferFileFail;
            m_tcAckInfo = CM_INVALID_FILE;   // which one ?
            m_modeTimeout = 500;             // wait up to 5 seconds for the ADRF to get status
            //m_tcAckInfo = XFR_INVALID_FILE;
        }
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
