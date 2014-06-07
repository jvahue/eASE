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

#include "File.h"
#include "Interface_CM.h"
#include "MailBox.h"

class CmReconfig
{
public:
    enum CmReconfigState {
        eCmRecfgIdle,           // waiting for recfg action
        eCmRecfgWaitAck,        // wait before send ACK
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
        eCmAdrfFactoryRestart = 150,

    };

    // enums must match RECFG_ERR_CODE_ENUM in Interface_CM.h before
    enum CmReconfigStatus {
      eCmRecfgStsOk = 0,     
      eCmRecfgStsBadFile,
      eCmRecfgCrcError,
      eCmRecfgDelimitError,
      eCmRecfgParseError,
      eCmRecfgBinFileCrcError,
      eCmRecfgBinChanCrcError,
      eCmRecfgBadXmlFile,
      eCmRecfgXmlCrcError,
      eCmRecfgXmlParseError,
      eCmRecfgNoCmResponse,

      eCmRecfgStsNoVfyRsp,  // .. eCmRecfgStsNoVfyRsp - this is our status for no response
      eCmRecfgStsMax
    };

    CmReconfig(AseCommon* pCommon);
    void Init();

    void ProcessCfgMailboxes(bool msOnline, MailBox& in, MailBox& out);
    BOOLEAN CheckCmd( SecComm& secComm, MailBox& out);

    void SetCfgFileName(const char* name, UINT32 size);
    bool StartReconfig(MailBox& out);
    const char* GetModeName() const;
    const char* GetCfgStatus() const;
    const char* GetLastCmd() const;

    char m_xmlFileName[eCmRecfgFileSize];
    char m_cfgFileName[eCmRecfgFileSize];
    UINT32 m_unexpectedCmds[eCmRecfgAdrfCmds];

    CmReconfigState m_mode;
    UINT32 m_modeTimeout;
    CmReconfigStatus m_lastErrCode;
    BOOLEAN m_lastReCfgFailed;
    UINT32 m_recfgCount;  // how many recfg rqst have we seen
    UINT32 m_recfgCmds;   // how many recfg rqst have we seen

    // script test control items
    UINT32 m_tcRecfgAckDelay;  // CM_tcRecfgAckDelay(x)
    UINT32 m_tcFileNameDelay;  // CM_tcFileNameDelay(x)
    UINT32 m_tcRecfgLatchWait; // CM_tcRecfgLatchWait(x)

    ADRF_TO_CM_CODE m_lastCmd; // what was the last cmd

private:
    bool ProcessRecfg(bool msOnline, ADRF_TO_CM_RECFG_RESULT& inData, MailBox& out);
    void ResetScriptControls();

    File m_file;
    char m_mbErr[128];
    AseCommon* m_pCommon;
};

#endif /* CMRECONFIG_H_ */
