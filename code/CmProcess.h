#ifndef CMPROCESS_H
#define CMPROCESS_H

#include "CmdRspThread.h"
#include "CmFileXfer.h"
#include "CmReconfig.h"
#include "Fifo.h"
#include "File.h"
#include "Gse_Interface.h"
#include "MailBox.h"
#include "SecComm.h"

// File: CmProcess.h

/******************************************************************************
* Description: Implements the ASE thread connecting to the ADRF for testing
*              Configuration Management behavior.
*
*
*/
class CmProcess : public CmdRspThread
{
    public:
        enum CmProcConstants
        {
        	eMaxQueueDepth = 10,
        	eGseCmdSize = 128,
        };

	    CmProcess();
        virtual void Run();  // override the AseThread::Create
        virtual BOOLEAN CheckCmd( SecComm& secComm);
        virtual int UpdateDisplay(VID_DEFS who, int theLine);

        char m_boxOnTime[32];
        char m_readyFile[128];

    protected:
        // Properties
        char m_lastGseCmd[eGseCmdSize];

        GSE_COMMAND  m_gseCmd;
        GSE_RESPONSE m_gseRsp;

        // mailboxes
        MailBox m_gseInBox;   // GSE -> CMProcess message
        MailBox m_gseOutBox;  // CMProcess -> GSE messages
        UINT32 m_lastGseSent; // when was the last gse cmd sent?
        bool m_requestPing;   // request adrf status

        FIFO m_gseRxFifo;  // holds any data received from the GSE MB

        /* To be activated
        MailBox m_gseMfdInBox;
        MailBox m_gseMfdOutBox;
        */

        MailBox m_reConfigInBox;
        MailBox m_reConfigOutBox;

        MailBox m_fileXferInBox;
        MailBox m_fileXferOutBox;

        // Methods
        virtual void RunSimulation(); // override the CmdRspThread::Simulation
        virtual void HandlePowerOff();// override the CmdRspThread::HandlePowerOff

    private:
        CmReconfig m_reconfig;
        CmFileXfer m_fileXfer;

        File m_putFile;
        File m_getFile;

        bool m_performAdrfOffload;

        bool PutFile(SecComm& secComm);
        bool GetFile(SecComm& secComm);

        void ProcessGseMessages();
        void ProcessLogMessages();
};


#endif
