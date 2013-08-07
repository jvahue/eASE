#ifndef CMRECONFIG_H_
#define CMRECONFIG_H_

/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:         CmReconfig.h

    Description:  This class implements the functitonality of the ADRF
    reconfiguration.  It handles all requests and responses during the
    reconfiguration.

    It has control items taht allow the test scripts to define how it responds
    to ADRF.

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

    void SetCfgFileName(const char* name, UINT32 size);
    bool StartReconfig(MailBox& out);
    char* GetModeName() const;

    char m_xmlFileName[eCmRecfgFileSize];
    char m_cfgFileName[eCmRecfgFileSize];
    UINT32 m_unexpectedCmds[eCmRecfgAdrfCmds];

    CmReconfigState m_state;
    UINT32 m_modeTimeout;
    RECFG_ERR_CODE_ENUM m_lastErrCode;
    BOOLEAN m_lastStatus;

    // script control items
    UINT32 m_msRerqstDelay;  // how long will the
    UINT32 m_fileNameDelay;  // CM response Control flag
    UINT32 m_recfgAckDelay;
    UINT32 m_recfgCount;

private:
    bool ProcessRecfg(bool msOnline, ADRF_TO_CM_RECFG_RESULT& inData, MailBox& out);

    File m_file;
};

#endif /* CMRECONFIG_H_ */
