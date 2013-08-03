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

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmReconfig::CmReconfig()
    : m_state(eCmRecfgIdle)
    , m_expectedReCfgAck(false)
    , m_expectedReCfgSts(false)
    , m_fileNameDelay(0)
    , m_recfgAckDelay(0)
    , m_unexpectedRecfgResult(0)
{
    memset(m_xmlFileName, 0, sizeof(m_xmlFileName));
    memset(m_cfgFileName, 0, sizeof(m_cfgFileName));
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
    strncpy(m_cfgFileName, &name[xmlLen], eCmRecfgFileSize);
}

//-------------------------------------------------------------------------------------------------
// Function: SetCfgFileName
// Description: Set the filenames to be used during the reconfiguration process
//
void CmReconfig::StartReconfig(MailBox& in, MailBox& out)
{
    CM_TO_ADRF_RESP_STRUCT inData;
    CM_TO_ADRF_RESP_STRUCT outData;

    outData.code = MS_RECFG_REQ;
    out.Send( &outData, sizeof(outData));

    m_expectedReCfgAck = true;
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessCfgMailboxes
// Description: Handle responding to the ADRF mailboxes for Reconfig
//
void CmReconfig::ProcessCfgMailboxes(MailBox& in, MailBox& out)
{
    CM_TO_ADRF_RESP_STRUCT inData;
    CM_TO_ADRF_RESP_STRUCT outData;

    if( in.Receive(&inData, sizeof(inData)))
    {
        // is the ADRF ready to accept a cfg file set
        if (inData.code == RECFG_REQ_CODE)
        {
            // TODO: are we responding
            // TODO: are we responding with garbage?
            outData.code = RECFG_REQ_ACK;
            out.Send( &outData, sizeof(outData));

            m_state = eCmRecfgSendFilenames;
        }
        else if (inData.code == RECFG_RESULT_CODE)
        {
            if (m_expectedReCfgSts)
            {
                m_unexpectedRecfgResult += 1;
                m_expectedReCfgSts = false;
            }
        }
        else if (inData.code == MS_DATETIME_REQ)
        {
            // send a datetime stamp into the ADRF
            // TBD: where does this come from?

            // Send MS Date Time
            LINUX_TM_FMT linuxTime;

            // linuxTime.tm_year = 110;  // Add this to 1900 to get year
            linuxTime.tm_year = 2010;  // Straight Year
            linuxTime.tm_mon = 12;
            linuxTime.tm_mday = 25;
            linuxTime.tm_hour = 13;
            linuxTime.tm_min = 15;
            linuxTime.tm_sec = 25;

            memcpy ( &outData.buff[0], &linuxTime, sizeof(linuxTime));

            outData.code = MS_DATETIME_RESP;
            out.Send( &outData, sizeof(outData));
        }
    }

    // is it time for us to send the filenames
    if (m_state == eCmRecfgSendFilenames)
    {
        if (m_fileNameDelay-- == 0)
        {
            outData.code = RECFG_FILE_READY;
            memcpy(&outData.buff[0],   m_cfgFileName, 128);
            memcpy(&outData.buff[128], m_xmlFileName, 128);
            out.Send( &outData, sizeof(outData));

            m_state = eCmRecfgIdle;
            m_expectedReCfgSts = true;
        }
    }
}
