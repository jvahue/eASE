#ifndef CMPROCESS_H
#define CMPROCESS_H

#include "AseThread.h"
#include "MailBox.h"
#include "Gse_Interface.h"

// File: CmProcess.h

/******************************************************************************
* Description: Implements the ASE thread connecting to the ADRF for testing
*              Configuration Management behavior.
*
*
*/

class CmProcess : public AseThread
{
    public:
	    CmProcess();
        virtual void Create();  // override the AseThread::Create
        enum
        {
        	eMaxQueueDepth = 10
        };

        CHAR m_boxOnTime[32];

    protected:
        // Properties
        BOOLEAN m_bRspPending;

        GSE_COMMAND  m_gseCmd;
        GSE_RESPONSE m_gseRsp;


        // mailboxes
        MailBox m_gseInBox;  // GSE -> CMProcess message
        MailBox m_gseOutBox; // CMProcess -> GSE messages

        /* To be activated
        MailBox m_gseMfdInBox;
        MailBox m_gseMfdOutBox;

        MailBox m_reConfigInBox;
        MailBox m_reConfigOutBox;

        MailBox m_fileXferInBox;
        MailBox m_fileXferOutBox;
        */



        // Methods
        virtual void Process(); // override the AseThread::Process
};


#endif
