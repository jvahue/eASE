#ifndef CMFILEXFER_H_
#define CMFILEXFER_H_

/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:         CmFileXfer.h

    Description:  This class implements the functitonality of the ADRF
    log transfer.  It handles all requests and responses during the
    transfer.

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
#include "IF_FileXfer.h"
#include "Mailbox.h"
#include "SecComm.h"

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
class CmFileXfer
{
public:
    enum CmFileXferMode {
        eXferIdle,          // no activity
        eXferAckTimeout,    // ready to respond with an (N)ACK after the timeout
        // ePySTE States
        eXferFileWait,      // wait for ePySte to request the file
        eXferFileOffload,   // stream file up to ePySte to verify the CRC
        eXferFileCheck,     // wait for ePySTe to return the CRC
        eXferFileValid,     // wait for ADRF to validate the CRC

        // ADRF States
    };

    CmFileXfer();
    void ProcessFileXfer(bool msOnline, MailBox& in, MailBox& out);
    BOOLEAN CheckCmd( SecComm& secComm);

    const char* GetModeName() const;

    bool m_fileXferRequested;
    CmFileXferMode m_mode;
    UINT32 m_modeTimeout;
    UINT32 m_fileXferRqsts;
    UINT32 m_fileXferMsgs;

    char m_xferFileName[CM_FILE_NAME_LEN];

    // Test Control Items
    UINT32 m_tcAckDelay;          // CM_tcAckDelayMs(x) - delay before repsonding with a (N)ACK
    CM_CMD_STATUS m_tcAckStatus;  //
    CM_ACK_INFO m_tcAckInfo;


protected:
    void FileXferResponse(FILE_RCV_MSG& rcv, MailBox& out);
    void SendAck(MailBox& out);

    File m_xferFile;
    FILE_RCV_MSG m_sendMsgBuffer;

    UINT32 m_fileCrc;
    UINT8  m_fileCrcAck;

    bool m_msOnline;   // updated on each call to ProcessFileXfer

};
#endif /* CMFILEXFER_H_ */
