//-----------------------------------------------------------------------------
//          Copyright (C) 2013-2016 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: CmReconfig.cpp
//
//    Description: Handle the reconfiguration process between PySte/ASE and 
//                 the ADRF
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
#include "AseCommon.h"

#include "SecComm.h"
#include "CmReconfig.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const char* modeNames[] = {
    "Idle",           // waiting for recfg action
    "WaitAck",        // wait before send ACK to ADRF
    "RecfgLatch",     // MS recfg rqst sent and latch, don't send rqst again
    "WaitRequest",    // MS is now waiting for the recfg rqst from ADRF - no timeout
    "SendFilenames",  // locally we are 'delaying' for file fetch from the MS
    "WaitStatus"      // wait for the recfg status code
};

static const CHAR* recfgStatus[] = {
    "Ok",           // eCmRecfgStsOk:
    "BadFile",      // eCmRecfgStsBadFile:
    "CRC ",         // eCmRecfgCrcError:
    "Delimiter",    // eCmRecfgDelimitError:
    "ParseErr",     // eCmRecfgParseError:
    "BinCrc",       // eCmRecfgBinFileCrcError:
    "BinChan",      // eCmRecfgBinChanCrcError:
    "Bad XML",      // eCmRecfgBadXmlFile:
    "XML CRC",      // eCmRecfgXmlCrcError:
    "XMLParse",     // eCmRecfgXmlParseError:
    "No CM Rsp",    // eCmRecfgNoCmResponse:
    "Sts Time Out", // eCmRecfgStsNoVfyRsp:
    "Wait Sts",     // eCmRecfgStsMax:
    "Unknown Sts?", // any other value
};

static const CHAR* cmdStrings[] = {
    "RecfgRqst",  // RECFG_REQ_CODE = 0x100,   // Request Cfg File from CM
    "RecfgRslt",  // RECFG_RESULT_CODE = 0x101,
    "MsRecfgAck", // MS_RECFG_ACK = 0x102,
    "MsDtRqst",   // MS_DATETIME_REQ = 0x103,
    "None"        // = 104
};

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmReconfig::CmReconfig(AseCommon* pCommon)
: m_mode(eCmRecfgIdle)
, m_modeTimeout(0)
, m_lastErrCode(eCmRecfgStsMax)
, m_lastReCfgFailed(false)
, m_recfgCount(0)
, m_recfgCmds(0)
// Test Control Items
, m_tcRecfgAckDelay(0)
, m_tcFileNameDelay(0)
, m_tcRecfgLatchWait(1000)
, m_lastCmd(ADRF_TO_CM_CODE_MAX)
, m_pCommon(pCommon)
{
    memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
    memset(m_cfgFileName, 0, sizeof(m_cfgFileName));
    memset(m_unexpectedCmds, 0, sizeof(m_unexpectedCmds));
    memset(m_mbErr, 0, sizeof(m_mbErr));
}

//---------------------------------------------------------------------------------------------
// Function: Init
// Description: Make sure we are reset when the ADRF powers down, if we are in a script keep
// counts.
//
void CmReconfig::Init()
{
    if (!IS_SCRIPT_ACTIVE)
    {
        m_recfgCount = 0;
        m_recfgCmds = 0;
        m_tcRecfgAckDelay = 0;
        m_tcFileNameDelay = 0;
        m_tcRecfgLatchWait = 1000;
        m_lastCmd = ADRF_TO_CM_CODE_MAX;
    }

    // we may have setup a very long timeout on the latch state so honor that
    if (m_mode != eCmRecfgLatch && m_mode != eCmRecfgWaitRequest)
    {
        m_mode = eCmRecfgIdle;
        m_modeTimeout = 0;
        m_lastErrCode = eCmRecfgStsMax;
        m_lastReCfgFailed = false;
        memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
        memset(m_cfgFileName, 0, sizeof(m_cfgFileName));
        memset(m_unexpectedCmds, 0, sizeof(m_unexpectedCmds));
        memset(m_mbErr, 0, sizeof(m_mbErr));
    }
}

//---------------------------------------------------------------------------------------------
// Function: ResetControls
// Description: reset the object controls to their defaults when a script ends
//
void CmReconfig::ResetScriptControls()
{
    // reset the object controls to their defaults when a script ends
    m_tcRecfgAckDelay = 0;
    m_tcFileNameDelay = 0;
    m_tcRecfgLatchWait = 1000;
}

//---------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Set the filenames to be used during the reconfiguration process
//
BOOLEAN CmReconfig::CheckCmd( SecComm& secComm, MailBox& out)
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    //int port;  // 0 = gse, 1 = ms

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
            m_lastErrCode = eCmRecfgStsMax;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            secComm.ErrorMsg("MS Recfg Rqst Fail: <%s>", m_mbErr);
            secComm.m_response.successful = FALSE;
        }

        serviced = TRUE;
        break;

    case eGetReconfigSts:
        secComm.m_response.value = FLOAT32(m_lastErrCode);
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

        //----------------------------------------------------------------------------
        // Test Controls
    case eCmRecfgAckDelay:
        // send the old one back and set the new value
        secComm.m_response.value = float(m_tcRecfgAckDelay);
        m_tcRecfgAckDelay = request.variableId;
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

    case eCmFileNameDelay:
        // send the old one back and set the new value
        secComm.m_response.value = float(m_tcFileNameDelay);
        m_tcFileNameDelay = request.variableId;
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

    case eCmLatchWait:
        // set the mode timeout for waiting in the request recfg latch state
        secComm.m_response.value = float(m_tcRecfgLatchWait);
        m_tcRecfgLatchWait = request.variableId;
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

        //----------------------------------------------------------------------------
        // Test Status
    case eGetRcfCount:
        secComm.m_response.value = float(m_recfgCount);
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


//---------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------
// Function: SetCfgFileName
// Description: MS has requested a reconfigure
//
bool CmReconfig::StartReconfig(MailBox& out)
{
    bool status = false;
    if ( m_mode == eCmRecfgIdle)
    {
        CM_TO_ADRF_RESP_STRUCT outData;
        memset( &outData, 0, sizeof(outData));

        outData.code = MS_RECFG_REQ;
        status = out.Send( &outData, sizeof(outData));

        if (status)
        {
            m_mode = eCmRecfgLatch;
            m_modeTimeout = m_tcRecfgLatchWait;
        }
        else
        {
            sprintf(m_mbErr, "StartReconfig Send: %s", out.GetStatusStr());
        }
    }
    else
    {
        sprintf(m_mbErr, "Invalid Mode <%s> s/b <Idle>", GetModeName());
    }

    return status;
}

//---------------------------------------------------------------------------------------------
// Function: ProcessCfgMailboxes
// Description: Handle responding to the ADRF mailboxes for Reconfigure
//
void CmReconfig::ProcessCfgMailboxes(bool msOnline, MailBox& in, MailBox& out)
{
    ADRF_TO_CM_RECFG_RESULT inData;
    memset( &inData, 0, sizeof(inData));

    // any envelopes for us?
    BOOLEAN inOk = in.Receive(&inData, sizeof(inData));

    // always run ProcessRecfg - to ensure mode timeouts are accurate
    if (!ProcessRecfg(msOnline, inData, out) && inOk && inData.code != 0)
    {
        CM_TO_ADRF_RESP_STRUCT outData;

        memset( &outData, 0, sizeof(outData));

        // if we got a request for MS time and the MS is on-line - then respond
        if (inData.code == MS_DATETIME_REQ && IS_MS_ONLINE)
        {
            LINUX_TM_FMT time;

            time.tm_year = m_pCommon->clocks[eClkMs].m_time.tm_year; // year from 1900
            time.tm_mon  = m_pCommon->clocks[eClkMs].m_time.tm_mon;  // month    0..11
            time.tm_mday = m_pCommon->clocks[eClkMs].m_time.tm_mday; // day of the month 1..31
            time.tm_hour = m_pCommon->clocks[eClkMs].m_time.tm_hour; // hours    0..23
            time.tm_min  = m_pCommon->clocks[eClkMs].m_time.tm_min;  // minutes  0..59
            time.tm_sec  = m_pCommon->clocks[eClkMs].m_time.tm_sec;  // seconds  0..59

            memcpy ( &outData.buff[0], &time, sizeof(time));

            outData.code = MS_DATETIME_RESP;
            out.Send( &outData, sizeof(outData));
        }
        else
        {
            // here we have an unknown command
        }
    }

    if ( !IS_SCRIPT_ACTIVE)
    {
        ResetScriptControls();
    }
}

//---------------------------------------------------------------------------------------------
// Function: ProcessRecfg
// Description: Handle commands related to reconfiguration
//
// TODO: Handle resetting the protocol state when ADRF send the RECFG_REQ_CODE code
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
        m_lastCmd = cmd;
        m_recfgCmds += 1;
    }
    else
    {
        status = false;
    }

    // NOTE: Always running this if statement allows us to handle resetting
    //       the protocol state whenever ADRF sends the RECFG_REQ_CODE code
    if (inData.code == RECFG_REQ_CODE)
    {
        // The ADRF is requesting we start a reconfigure
        // TODO: are we responding
        // TODO: are we responding with garbage?
        m_modeTimeout = m_tcRecfgAckDelay;
        m_mode = eCmRecfgWaitAck;
        m_lastErrCode = eCmRecfgStsMax;

        m_recfgCount += 1;
        cmdHandled = true;
    }

    if (m_mode == eCmRecfgWaitAck)
    {
        if (m_modeTimeout == 0)
        {
            outData.code = RECFG_REQ_ACK;
            if (strstr(m_cfgFileName, "Failed Reconfig") == NULL && strlen(m_cfgFileName) > 0)
            {
                // do you want to base this on msOnline ? it sort of used to be ??
                outData.buff[0] = RECFG_ACK_CM_OK;
                m_mode = eCmRecfgSendFilenames;
                m_modeTimeout = m_tcFileNameDelay;
                m_lastErrCode = eCmRecfgStsMax;
            }
            else
            {
                outData.buff[0] = RECFG_ACK_CM_NOT_OK;
                m_mode = eCmRecfgIdle;
            }

            // send a response yea or nay
            out.Send( &outData, sizeof(outData));
        }
        else
        {
            m_modeTimeout -= 1;
        }
    }

    else if (m_mode == eCmRecfgLatch)
    {
        if (inData.code == MS_RECFG_ACK)
        {
            // recfg request acknowledged wait for the ADRf to start the reconfigure
            m_mode = eCmRecfgWaitRequest;
            cmdHandled = true;
        }
        else
        {
            if (m_modeTimeout == 0)
            {
                // TBD: should we resend the request if we don't get latch (3 times?)
                m_mode = eCmRecfgIdle;
                m_modeTimeout = 0;
            }
            else
            {
                m_modeTimeout -= 1;
            }
        }
    }

    else if (m_mode == eCmRecfgWaitRequest)
    {
        if (m_modeTimeout == 0)
        {
            // TBD: should we resend the request if we don't get latch (3 times?)
            m_mode = eCmRecfgIdle;
            m_modeTimeout = 0;
        }
        else
        {
            m_modeTimeout -= 1;
        }
    }

    else if (m_mode == eCmRecfgSendFilenames)
    {
        if (m_modeTimeout == 0)
        {
            // TODO: ADRF wait up to 2 minutes for files
            outData.code = RECFG_FILE_READY;
            memcpy(&outData.buff[0],   m_cfgFileName, 128);
            memcpy(&outData.buff[128], m_xmlFileName, 128);
            out.Send( &outData, sizeof(outData));

            m_mode = eCmRecfgStatus;
            m_modeTimeout = 30000;  // 5min timeout on the status response from the ADRF
        }
        else
        {
            m_modeTimeout -= 1;
        }
    }

    else if (m_mode == eCmRecfgStatus)
    {
        if (inData.code == RECFG_RESULT_CODE)
        {
            m_lastErrCode = CmReconfigStatus(inData.errCode);
            m_lastReCfgFailed = inData.bOk;
            m_mode = eCmRecfgIdle;

            // delete the files
            m_file.Delete( m_xmlFileName, File::ePartCmProc);
            m_file.Delete( m_cfgFileName, File::ePartCmProc);

            if (m_lastReCfgFailed == FALSE)
            {
                // delete the files from the partition & clear the names
                memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
                memset(m_cfgFileName, 0, sizeof(m_cfgFileName));

                // indicate CCDL needs to restart
                m_pCommon->recfgSuccess = true;
            }
            else
            {
                sprintf(m_xmlFileName, "Failed Reconfig - Cfg File Err");
                sprintf(m_cfgFileName, "Failed Reconfig - Cfg File Err");
            }

            cmdHandled = true;
        }
        else
        {
            if (m_modeTimeout == 0)
            {
                m_mode = eCmRecfgIdle;

                // delete the files
                m_file.Delete( m_xmlFileName, File::ePartCmProc);
                m_file.Delete( m_cfgFileName, File::ePartCmProc);

                sprintf(m_xmlFileName, "Failed Reconfig - Timeout");
                sprintf(m_cfgFileName, "Failed Reconfig - Timeout");
                m_lastErrCode = eCmRecfgStsNoVfyRsp;
                m_lastReCfgFailed = TRUE;
            }
            else
            {
                m_modeTimeout -= 1;
            }
        }
    }

    if (status && !cmdHandled)
    {
        UINT32 cmdX = cmd - RECFG_REQ_CODE;
        m_unexpectedCmds[cmdX] += 1;
    }

    return status;
}

//---------------------------------------------------------------------------------------------
// Function: GetMode
// Description: Return a string rep of the mode name
//
const char* CmReconfig::GetModeName() const
{
    return modeNames[m_mode];
}

//---------------------------------------------------------------------------------------------
// Function: GetMode
// Description: Return a string rep of the mode name
//
const char* CmReconfig::GetCfgStatus() const
{
    if (m_lastErrCode >= eCmRecfgStsOk && m_lastErrCode <= eCmRecfgStsMax)
    {
        return recfgStatus[m_lastErrCode];
    }
    else
    {
        return recfgStatus[eCmRecfgStsMax+1];
    }
}

const char* CmReconfig::GetLastCmd() const
{
    return cmdStrings[m_lastCmd-RECFG_REQ_CODE];
}
