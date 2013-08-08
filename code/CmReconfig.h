#ifndef CMRECONFIG_H_
#define CMRECONFIG_H_

/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:         CmReconfig.h

    Description:  This class implements the functitonality of the ADRF
    reconfiguration.  It handles all requests and responses during the
    reconfiguration.

    It has control items that allow the test scripts to define how it responds
    to ADRF.  All of these Test Control item are identified by m_tc<Name>.
    All of them can be set via commands from the test script.

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "alt_stdtypes.h"

#include "AseCommon.h"
#include "File.h"
#include "Interface_CM.h"
#include "MailBox.h"
#include "SecComm.h"


class CmReconfig
{
public:
    enum CmReconfigState {
        eCmRecfgIdle,           // waiting for reconfig action
        eCmRecfgLatch,          // MS recfg rqst sent and latch, don't send rqst again
        eCmRecfgWaitRequest,    // MS is now waiting for the recfg rqst from ADRF - no timeout
        eCmRecfgSendFilenames,  // locally we are 'delaying' for file fetch from the MS
        eCmRecfgStatus,         // wait for the recfg status code
    };

    enum  CmReCfgConstants {
        eCmRecfgXml,
        eCmRecfgCfg,
        eCmRecfgFileSize = 128,
        eCmRecfgAdrfCmds = 4,

    };

    CmReconfig();

    void ProcessCfgMailboxes(bool msOnline, MailBox& in, MailBox& out);
    BOOLEAN CheckCmd( SecComm& secComm, MailBox& out);

    void SetCfgFileName(const char* name, UINT32 size);
    bool StartReconfig(MailBox& out);
    char* GetModeName() const;
    char* GetCfgStatus() const;

    char m_xmlFileName[eCmRecfgFileSize];
    char m_cfgFileName[eCmRecfgFileSize];
    UINT32 m_unexpectedCmds[eCmRecfgAdrfCmds];

    CmReconfigState m_state;
    UINT32 m_modeTimeout;
    RECFG_ERR_CODE_ENUM m_lastErrCode;
    BOOLEAN m_lastStatus;
    UINT32 m_recfgCount;

    // script test control items
    UINT32 m_tcFileNameDelay;  // CM_tcFileNameDelay(x)
    UINT32 m_tcRecfgAckDelay;  // CM_tcRecfgAckDelay(x)

private:
    bool ProcessRecfg(bool msOnline, ADRF_TO_CM_RECFG_RESULT& inData, MailBox& out);

    File m_file;
    char m_mbErr[128];
};

#endif /* CMRECONFIG_H_ */
