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
#include "Interface_CM.h"
#include "MailBox.h"


class CmReconfig
{
public:
    enum CmReconfigState {
        eCmRecfgIdle,
        eCmRecfgSendFilenames,
        eCmRecfgGetResult,
    };

    enum  CmReCfgConstants {
        eCmRecfgXml,
        eCmRecfgCfg,
        eCmRecfgFileSize = 128

    };

    CmReconfig();

    void SetCfgFileName(const char* name, UINT32 size);
    void StartReconfig(MailBox& in, MailBox& out);

    void ProcessCfgMailboxes(MailBox& in, MailBox& out);

    char m_xmlFileName[eCmRecfgFileSize];
    char m_cfgFileName[eCmRecfgFileSize];

    CmReconfigState m_state;
    bool m_expectedReCfgAck;
    bool m_expectedReCfgSts;
    UINT32 m_fileNameDelay;
    UINT32 m_recfgAckDelay;
    UINT32 m_unexpectedRecfgResult;
};

#endif /* CMRECONFIG_H_ */
