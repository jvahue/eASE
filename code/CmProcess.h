#ifndef CMPROCESS_H
#define CMPROCESS_H

#include "CmdRspThread.h"
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
        	eMaxQueueDepth = 10
        };

	    CmProcess();
        virtual void Run();  // override the AseThread::Create
        virtual BOOLEAN CheckCmd( SecComm& secComm);

        CHAR m_boxOnTime[32];

    protected:
        // Properties
        BOOLEAN m_bRspPending;

        GSE_COMMAND  m_gseCmd;
        GSE_RESPONSE m_gseRsp;


        // mailboxes
        MailBox m_gseInBox;  // GSE -> CMProcess message
        MailBox m_gseOutBox; // CMProcess -> GSE messages

        FIFO m_gseRxFifo;  // holds any data received from the GSE MB

        /* To be activated
        MailBox m_gseMfdInBox;
        MailBox m_gseMfdOutBox;

        MailBox m_reConfigInBox;
        MailBox m_reConfigOutBox;

        MailBox m_fileXferInBox;
        MailBox m_fileXferOutBox;
        */

        // Methods
        virtual void RunSimulation(); // override the CmdRspThread::Simulation
        virtual void HandlePowerOff();// override the CmdRspThread::HandlePowerOff

    private:
        File m_putFile;
        File m_getFile;
        char m_readyFile[256];  // TODO: remove after debugging
        void ProcessGseMessages();
        bool PutFile(SecComm& secComm);
        bool GetFile(SecComm& secComm);

};


#endif
