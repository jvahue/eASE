//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: CmProcess.cpp
//
//    Description:
//
// Video Display Layout
//
//-----------------------------------------------------------------------------

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "CmReconfig.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
typedef struct {
  SINT32     tm_sec;   // seconds  0..59
  SINT32     tm_min;   // minutes  0..59
  SINT32     tm_hour;  // hours    0..23
  SINT32     tm_mday;  // day of the month  1..31
  SINT32     tm_mon;   // month    0..11
  SINT32     tm_year;  // year from 1900
//SINT32     tm_wday;  // day of the week
//SINT32     tm_yday;  // day in the year
//SINT32     tm_isdst; // daylight saving time -1/0/1
} LINUX_TM_FMT, *LINUX_TM_FMT_PTR;


/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const char* modeNames[] = {
    "Idle",           // waiting for reconfig action
    "RecfgLatch",     // MS recfg rqst sent and latch, don't send rqst again
    "WaitRequest",    // MS is now waiting for the recfg rqst from ADRF - no timeout
    "SendFilenames",  // locally we are 'delaying' for file fetch from the MS
    "WaitStatus"      // wait for the recfg status code
};

static const CHAR* recfgStatus[] = {
    "Ok",
    "Bad File",
    "No Status"
};

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmReconfig::CmReconfig()
    : m_state(eCmRecfgIdle)
    , m_modeTimeout(0)
    , m_lastErrCode(RECFG_ERR_CODE_MAX)
    , m_lastStatus(false)
    , m_recfgCount(0)
    // Test Control Items
    , m_tcFileNameDelay(500)
{
    memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
    memset(m_cfgFileName, 0, sizeof(m_cfgFileName));
    memset(m_unexpectedCmds, 0, sizeof(m_unexpectedCmds));
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Set the filenames to be used during the reconfiguration process
//
BOOLEAN CmReconfig::CheckCmd( SecComm& secComm, MailBox& out)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eSetCfgFileName:

        SetCfgFileName(request.charData, request.charDataSize);
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

    case eStartReconfig:
        if (StartReconfig(out))
        {
            m_lastErrCode = RECFG_ERR_CODE_MAX;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            secComm.ErrorMsg("MS Recfg Request Fail Mode: %s MB: <%s>",
                             GetModeName(),
                             m_mbErr);
            secComm.m_response.successful = FALSE;
        }

        serviced = TRUE;
        break;

    case eGetReconfigSts:
        strcpy( secComm.m_response.streamData, GetCfgStatus());
        secComm.m_response.streamSize = strlen(secComm.m_response.streamData);
        secComm.m_response.successful = TRUE;

        serviced = TRUE;
        break;

    default:
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("CmReconfig");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
}


//-------------------------------------------------------------------------------------------------
// Function: SetCfgFileName
// Description: Set the filenames to be used during the reconfiguration process
//
void CmReconfig::SetCfgFileName(const char* name, UINT32 size)
{
    UINT32 xmlLen;

    strncpy(m_xmlFileName, name, eCmRecfgFileSize);
    xmlLen = strlen(m_xmlFileName);
    strncpy(m_cfgFileName, &name[xmlLen+1], eCmRecfgFileSize);
}

//-------------------------------------------------------------------------------------------------
// Function: SetCfgFileName
// Description: MS has requested a reconfig
//
bool CmReconfig::StartReconfig(MailBox& out)
{
    bool status = false;
    if ( m_state == eCmRecfgIdle)
    {
        CM_TO_ADRF_RESP_STRUCT outData;
        memset( &outData, 0, sizeof(outData));

        outData.code = MS_RECFG_REQ;
        status = out.Send( &outData, sizeof(outData));

        if (status)
        {
            m_state = eCmRecfgLatch;
            m_modeTimeout = 100;
        }
        else
        {
            sprintf(m_mbErr, "%s", out.GetIpcStatusString());
        }
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessCfgMailboxes
// Description: Handle responding to the ADRF mailboxes for Reconfig
//
void CmReconfig::ProcessCfgMailboxes(bool msOnline, MailBox& in, MailBox& out)
{
    ADRF_TO_CM_RECFG_RESULT inData;
    memset( &inData, 0, sizeof(inData));

    // any envelopes for us?
    BOOLEAN inOk = in.Receive(&inData, sizeof(inData));

    // always run ProcessRecfg
    if (!ProcessRecfg(msOnline, inData, out) && inOk && inData.code != 0)
    {
        CM_TO_ADRF_RESP_STRUCT outData;

        memset( &outData, 0, sizeof(outData));

        if (inData.code == MS_DATETIME_REQ)
        {
            // send a datetime stamp into the ADRF
            // TBD: where does this come from?

            // Send MS Date Time
            LINUX_TM_FMT linuxTime;

            // linuxTime.tm_year = 110;  // Add this to 1900 to get year
            linuxTime.tm_year = 2013;  // Straight Year
            linuxTime.tm_mon = 7;
            linuxTime.tm_mday = 26;
            linuxTime.tm_hour = 0;
            linuxTime.tm_min = 0;
            linuxTime.tm_sec = 0;

            memcpy ( &outData.buff[0], &linuxTime, sizeof(linuxTime));

            outData.code = MS_DATETIME_RESP;
            out.Send( &outData, sizeof(outData));
        }
        else
        {
            // here we have an unknown command
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessRecfg
// Description: Handle commands related to reconfiguration
//
// TODO: Handle reseting the protocol state when ADRF send the RECFG_REQ_CODE code
//
bool CmReconfig::ProcessRecfg(bool msOnline, ADRF_TO_CM_RECFG_RESULT& inData, MailBox& out)
{
    bool status;
    bool cmdHandled = false;
    ADRF_TO_CM_CODE cmd = inData.code;

    CM_TO_ADRF_RESP_STRUCT outData;
    memset( &outData, 0, sizeof(outData));

    // is the command (if there is one) directed at recfg?
    if ( cmd == RECFG_REQ_CODE || cmd == MS_RECFG_ACK || cmd == RECFG_RESULT_CODE)
    {
        status = true;
    }
    else
    {
        status = false;
    }

    // NOTE: Always running this if statement allows us to handle resetting
    //       the protocol state whenever ADRF sends the RECFG_REQ_CODE code
    if (inData.code == RECFG_REQ_CODE)
    {
        // The ADRF is requesting we start a reconfig
        // TODO: are we responding
        // TODO: are we responding with garbage?
        outData.code = RECFG_REQ_ACK;
        outData.buff[0] = msOnline ?
                          RECFG_ACK_CM_OK :
                          RECFG_ACK_CM_NOT_OK; // TODO: 8/5/13: this is opposite the doc

        out.Send( &outData, sizeof(outData));

        m_state = eCmRecfgSendFilenames;
        m_modeTimeout = m_tcFileNameDelay;
        m_lastErrCode = RECFG_ERR_CODE_MAX;

        m_recfgCount += 1;
        cmdHandled = true;
    }

    if (m_state == eCmRecfgLatch)
    {
        if (inData.code == MS_RECFG_ACK)
        {
            // recfg request acknowledged wait for the ADRf to start the reconfig
            m_state = eCmRecfgWaitRequest;
            cmdHandled = true;
        }
        else
        {
            if (m_modeTimeout == 0)
            {
                // TBD: should we resend the request if we don't get latch (3 times?)
                m_state = eCmRecfgIdle;
                m_modeTimeout = 0;
            }
            else
            {
                m_modeTimeout -= 1;
            }
        }
    }

    else if (m_state == eCmRecfgSendFilenames)
    {
        if (m_modeTimeout == 0)
        {
            // TODO: ADRF wait up to 2 minutes for files
            outData.code = RECFG_FILE_READY;
            memcpy(&outData.buff[0],   m_cfgFileName, 128);
            memcpy(&outData.buff[128], m_xmlFileName, 128);
            out.Send( &outData, sizeof(outData));

            m_state = eCmRecfgStatus;
            m_modeTimeout = 0;
        }
        else
        {
            m_modeTimeout -= 1;
        }
    }

    else if (m_state == eCmRecfgStatus)
    {
        if (inData.code == RECFG_RESULT_CODE)
        {
            m_lastErrCode = inData.errCode;
            m_lastStatus = inData.bOk;
            m_state = eCmRecfgIdle;
            // I know it seems backwards, but then your thinking logically aren't you!
            if (m_lastStatus == FALSE)
            {
                // delete the files from the partition & clear the names
                if (strlen(m_xmlFileName) > 0)
                {
                    m_file.Delete( m_xmlFileName, File::ePartCmProc);
                    memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
                }

                if (strlen(m_cfgFileName) > 0)
                {
                    m_file.Delete( m_cfgFileName, File::ePartCmProc);
                    memset(m_cfgFileName, 0, sizeof(m_cfgFileName));
                }
            }

            cmdHandled = true;
        }
    }

    if (status && !cmdHandled)
    {
        UINT32 cmdX = cmd - RECFG_REQ_CODE;
        m_unexpectedCmds[cmdX] += 1;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Function: GetMode
// Description: Return a string rep of the mode name
//
const char* CmReconfig::GetModeName() const
{
    return modeNames[m_state];
}

//-------------------------------------------------------------------------------------------------
// Function: GetMode
// Description: Return a string rep of the mode name
//
const char* CmReconfig::GetCfgStatus() const
{
    return recfgStatus[m_lastErrCode];
}
