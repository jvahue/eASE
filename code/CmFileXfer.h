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
        eXferIdle,        // no activity
        eXferRqst,        // ADRF has sent us a file name to upload
        eXferFile,        // ePySte asked for the file, and file is ok
        eXferFileFail,    // ePySte asked for the file, and file is bad
        eXferFileCrc,     // send ePySTe CRC to the ADRF to confirm
        eXferFileValid    // wait for ADRF to confirm the CRC
    };

    CmFileXfer();

    BOOLEAN CheckCmd( SecComm& secComm);
    void ProcessFileXfer(bool msOnline, MailBox& in, MailBox& out);
    void FileStatus(char* filename, bool canOpen);

    const char* GetModeName() const;

    bool m_fileXferRequested;
    CmFileXferMode m_mode;
    UINT32 m_modeTimeout;
    UINT32 m_fileXferMsgs;
    UINT32 m_fileXferRqsts;
    UINT32 m_fileXferServiced;
    UINT32 m_fileXferSuccess;
    UINT32 m_fileXferFailed;
    UINT32 m_fileXferFailLast;

    char m_xferFileName[CM_FILE_NAME_LEN];

    // Test Control Items
    UINT32 m_tcAckDelay;          // CM_tcAckDelayMs(x) - delay before responding with a (N)ACK
    CM_CMD_STATUS m_tcAckStatus;  //
    CM_ACK_INFO m_tcAckInfo;

protected:
    void FileXferResponse(FILE_RCV_MSG& rcv, MailBox& out);
    void SendAckTimeout(MailBox& out);
    void SendAck(MailBox& out);

    File m_xferFile;
    FILE_RCV_MSG m_sendMsgBuffer;

    UINT32 m_fileCrc;
    //UINT8  m_fileCrcAck;

    bool m_msOnline;   // updated on each call to ProcessFileXfer

};
#endif /* CMFILEXFER_H_ */
