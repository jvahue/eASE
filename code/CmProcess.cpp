//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: CmProcess.cpp
//
//    Description: Simulate the CM Process
//
/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "CmProcess.h"
#include "video.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define ADRF_CFG_FILE "config.bin"

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const CHAR adrfProcessName[] = "adrf";

static const CHAR cmReCfgMailboxName[]   = "CM_RECONFIG_ADRF";   // Comm Manager Mailbox
static const CHAR adrfReCfgMailboxName[]  = "ADRF_RECONFIG_CM";  // Adrf Mailbox

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmProcess::CmProcess()
    : m_bRspPending(FALSE)
{
    // TODO: remove after debugging
    memset( m_readyFile, 0, sizeof(m_readyFile));
    memset( m_lastGseCmd, 0, sizeof(m_lastGseCmd));
}

/****************************************************************************
 protected methods for FxProc
 ****************************************************************************/

void CmProcess::Run()
{

    // create alias for this process because adrf will be granting write access
    // to CMProcess, not ASE
    processStatus ps = createProcessAlias( "CMProcess");

    //--------------------------------------------------------------------------
    // Set up mailboxes for GSE commands with ADRF
    m_gseInBox.Create(CM_GSE_ADRF_MAILBOX, sizeof(m_gseRsp), eMaxQueueDepth);
    m_gseInBox.IssueGrant(adrfProcessName);

    // Connect to the the GSE recv box in the ADRF.
    m_gseOutBox.Connect(adrfProcessName, ADRF_GSE_CM_MAILBOX);

    //--------------------------------------------------------------------------
    // Set up mailboxes for processing reconfig msg from ADRF
    m_reConfigInBox.Create(cmReCfgMailboxName, 516, 2);
    m_reConfigInBox.IssueGrant(adrfProcessName);

    // Connect to the the Reconfig.
    m_reConfigOutBox.Connect(adrfProcessName, adrfReCfgMailboxName);

    //--------------------------------------------------------------------------
    // Set up mailboxes for processing log file msg from ADRF
    m_fileXferInBox.Create("CM_FILE_TRANSFER_ADRF", 512, 2);
    m_fileXferInBox.IssueGrant(adrfProcessName);

    // Connect to the the GSE recv box in the ADRF.
    m_fileXferOutBox.Connect(adrfProcessName, "ADRF_FILE_TRANSFER_CM");

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("CmProcess", "StdThreadTemplate");
}

//-------------------------------------------------------------------------------------------------
// Function: RunSimulation
// Description:
//
void CmProcess::RunSimulation()
{
    // Handle Reconfig Requests
    m_reconfig.ProcessCfgMailboxes(IS_MS_ONLINE, m_reConfigInBox, m_reConfigOutBox);

    // Handle File Xfer Request
    m_fileXfer.ProcessFileXfer(IS_MS_ONLINE, m_fileXferInBox, m_fileXferOutBox);

    ProcessGseMessages();
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessGseMessages
// Description:
//
void CmProcess::ProcessGseMessages()
{
    m_gseCmd.gseSrc = GSE_SOURCE_CM;
    m_gseCmd.gseVer = 1;
    memset(m_gseRsp.rspMsg, 0, sizeof(m_gseRsp.rspMsg) );

    // If not expecting a resp msg and time has elapsed to request the
    // Expecting cmd response... check inbox.
    if( m_gseInBox.Receive(&m_gseRsp, sizeof(m_gseRsp)) )
    {
        int size = strlen(m_gseRsp.rspMsg);
        m_gseRxFifo.Push(m_gseRsp.rspMsg, size);
    }
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessLogMessages
// Description:
//
void CmProcess::ProcessLogMessages()
{

}

//-------------------------------------------------------------------------------------------------
// Function: HandlePowerOff
// Description:
//
void CmProcess::HandlePowerOff()
{
    // reset mailboxes due to power off ( adrf process gone)
    m_gseInBox.Reset();
    m_gseOutBox.Reset();

    m_reConfigInBox.Reset();
    m_reConfigOutBox.Reset();

    m_fileXferInBox.Reset();
    m_fileXferOutBox.Reset();
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description:
//
BOOLEAN CmProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
    BOOLEAN subServiced = FALSE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms

    SecRequest request = secComm.m_request;
    switch (request.cmdId)
    {
    case eWriteStream:
        if ( request.charDataSize < GSE_MAX_LINE_SIZE)
        {
            GSE_COMMAND* mb;
            port = request.variableId;  // 0 = gse, 1 = ms

            memcpy((void*)m_gseCmd.commandLine, (void*)request.charData, request.charDataSize);
            m_gseCmd.commandLine[request.charDataSize] = '\0';

            m_gseOutBox.Send(&m_gseCmd, sizeof(m_gseCmd));
            secComm.m_response.successful = TRUE;

            //sprintf(secComm.m_response.errorMsg, "CmProcess(%s): Unable to send command %s <%s>",
            //            m_gseOutBox.IsConnected() ? "Conn" : "NoConn",
            //            m_gseOutBox.GetIpcStatusString(),
            //            m_gseOutBox.GetProcessStatusString());


            // temrinate the cmd before the CR

            request.charData[request.charDataSize-1] = '\0';
            strncpy(m_lastGseCmd, request.charData, eGseCmdSize);

        }
        else
        {
            secComm.ErrorMsg("GSE Command Length (%d) exceeds (%d)",
                             request.charDataSize, GSE_MAX_LINE_SIZE);
            secComm.m_response.successful = FALSE;
        }
        serviced = TRUE;
        break;

    case eReadStream:
        port = request.variableId;  // 0 = gse, 1 = ms

        secComm.m_response.streamSize = m_gseRxFifo.Pop(secComm.m_response.streamData,
                                                        eSecStreamSize);
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

    case eClearStream:
        // TODO: maybe - this should set a flag to the main thread,
        //       read stream above would need to monitor the flag then
        port = request.variableId;  // 0 = gse, 1 = ms

        m_gseRxFifo.Reset();
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        break;

    case ePutFile:
        if (PutFile(secComm))
        {
            secComm.m_response.successful = TRUE;
        }
        serviced = TRUE;

        break;

    case eGetFile:
        if (GetFile(secComm))
        {
            secComm.m_response.successful = TRUE;
        }
        else
        {
            secComm.m_response.successful = FALSE;
        }

        serviced = TRUE;
        break;

    case eDeleteCfgFile:
        // see if the file exists
        if (m_getFile.Open(ADRF_CFG_FILE, File::ePartAdrf, 'r'))
        {
            secComm.m_response.successful = m_getFile.Delete(ADRF_CFG_FILE, File::ePartAdrf);
        }
        else
        {
            // file does not exist so it like we deleted it !
            secComm.m_response.successful = TRUE;
        }

        serviced = TRUE;
        break;

    default:
        subServiced = m_reconfig.CheckCmd(secComm, m_reConfigOutBox);
        if (!subServiced)
        {
            subServiced = m_fileXfer.CheckCmd(secComm);
        }
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("CmProc");
        secComm.IncCmdServiced(rType);
    }

    return serviced || subServiced;
}

//-------------------------------------------------------------------------------------------------
// Function: PutFile
// Description: Handles putting a file on to the target system into the partition specified
//
// Notes:
// variableId: 0 = File name in m_request.charData
//             1 = File data in m_request.charData
// sigGenId: holds the partition Id
//
// When putting a file:
//    (1) The put file must not be open already when we start (i.e., m_request.variabielId = 0)
//    (2) The put file must open when writing data (i.e., m_request.variabielId = 1)
//    (3) When data is recieved with size 0 the file is closed
//
bool CmProcess::PutFile( SecComm& secComm)
{
    bool status = false;

    // are we opening a file?
    if (secComm.m_request.variableId == 0)
    {
        if (!m_putFile.IsOpen())
        {
            m_putFile.Open( secComm.m_request.charData, File::PartitionType(secComm.m_request.sigGenId), 'w');
            status = true;

            // TODO: remove after debugging is complete
            strcpy(m_readyFile, secComm.m_request.charData);
        }
        else
        {
            secComm.ErrorMsg("PutFile: Filename <%s> already open", m_putFile.GetFileName());
        }
    }
    else if (m_putFile.IsOpen())
    {
        if (secComm.m_request.charDataSize > 0)
        {
            if (m_putFile.Write(secComm.m_request.charData, secComm.m_request.charDataSize))
            {
                status = true;
            }
        }
        else
        {
            m_putFile.Close();
            status = true;
        }
    }
    else
    {
        secComm.ErrorMsg("PutFile: No file open for write");
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Function: GetFile
// Description: Handles getting a file from the target system from the partition specified
//
// Notes:
// variableId: 0 = Send file name in m_response.streamData indicates file exists
//             1 = send file data in m_response.streamData
// sigGenId: holds the partition Id
//
// When getting a file:
//    (1) if a file is ready to Tx the name will be put in m_response.streamData
//        otherwise m_response.streamData will be empty
//    (2) File data will be in m_response.streamData
//        when m_response.streamData is not max size or 0 the get file will close
//
// To simulate CmProcess when Getting a file we set some state data about if it matches
// the FileXfer filename.  If it does and the FileXfer filename changes we stop the
// transfer, and indicate a failure - which allows ePySte to delete the file.
//
bool CmProcess::GetFile( SecComm& secComm)
{
    INT32 bytesRead;
    UINT32 nameLength;
    bool status = false;

    // ePySte asking if a file is available
    if (secComm.m_request.variableId == 0)
    {
        if (!m_getFile.IsOpen())
        {
            // charData holds the file name
            memset(m_readyFile, 0, sizeof(m_readyFile));
            memcpy(m_readyFile, secComm.m_request.charData, secComm.m_request.charDataSize);

            if (m_getFile.Open(m_readyFile, File::PartitionType(secComm.m_request.sigGenId), 'r'))
            {
                // check to see if we are uploading a FileXfer request from ADRF
                m_performAdrfOffload = strcmp( m_readyFile, m_fileXfer.m_xferFileName) == 0;

                nameLength = strlen(m_getFile.GetFileName());
                strncpy( secComm.m_response.streamData, m_getFile.GetFileName(), nameLength);
                secComm.m_response.streamSize = nameLength;

                status = true;
            }
            else
            {
                secComm.ErrorMsg("GetFile: File not available");
            }
        }
        else
        {
            secComm.ErrorMsg("GetFile: File is Open");
        }
    }

    // ePySte wants file data
    else if (m_getFile.IsOpen())
    {
        // if we were offloading an ADRF requested file - see if the adrf wants to continue
        bool continueAdrf = strcmp( m_readyFile, m_fileXfer.m_xferFileName) == 0;

        if ((m_performAdrfOffload && continueAdrf) || !m_performAdrfOffload)
        {
            bytesRead = m_getFile.Read(secComm.m_response.streamData, eSecStreamSize);
            secComm.m_response.streamSize = bytesRead;
            if (bytesRead < eSecStreamSize)
            {
                m_getFile.Close();
                memset(m_readyFile, 0, sizeof(m_readyFile));
                m_performAdrfOffload = false;
            }

            if (bytesRead >= 0)
            {
                status = true;
            }
        }
        else
        {
            m_getFile.Close();
            memset(m_readyFile, 0, sizeof(m_readyFile));
            m_performAdrfOffload = false;
        }
    }
    else
    {
        secComm.ErrorMsg("GetFile: No file open for read");
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateDisplay
// Description: Update the video output display
//
// Video Display Layout
// 0-1: hdr display
// 2: Cfg(%d) Mode/Status: %s(%d)/%s(%s)
// 3: Gse Cmd: %s
// 4: RxFifo: %d
// 5: Gse In: <proc>(%d)/<ipc> Out: <proc>/<ipc>
// 6: Cfg In: <proc>(%d)/<ipc> Out: <proc>/<ipc>
// 7: Log In: <proc>(%d)/<ipc> Out: <proc>/<ipc>
// ...
//-----------------------------------------------------------------------------
void CmProcess::UpdateDisplay(VID_DEFS who)
{
    char buffer[256];
    UINT32 atLine = eFirstDisplayRow;

    CmdRspThread::UpdateDisplay(CmProc);

    // Status Display
    //debug_str(CmProc, atLine, 0,"%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "Cfg(%d) Mode/Status: %s(%d)/%s(%s)",
              m_reconfig.m_recfgCount,
              m_reconfig.GetModeName(),
              m_reconfig.m_modeTimeout,
              m_reconfig.m_lastStatus ? "Err" : "Ok",
              m_reconfig.GetCfgStatus());
    atLine += 1;

    debug_str(CmProc, atLine, 0, "Log(%d) Msgs: %d Mode: %s(%d)",
              m_fileXfer.m_fileXferRqsts,
              m_fileXfer.m_fileXferMsgs,
              m_fileXfer.GetModeName(),
              m_fileXfer.m_modeTimeout
              );
    atLine += 1;

    //debug_str(CmProc, atLine, 0,"%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "GseCmd: %s", m_lastGseCmd);
    atLine += 1;

    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "Gse RxFifo: %d", m_gseRxFifo.Used());
    atLine += 1;

    // Show put file status
    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "PUT: %s", m_putFile.GetFileStatus(buffer));
    atLine += 1;

    // Show get file status
    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "GET: %s", m_getFile.GetFileStatus(buffer));
    atLine += 1;

    // Show Cfg file names
    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "XML: %s", m_reconfig.m_xmlFileName);
    atLine += 1;

    // Show Cfg file names
    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "CFG: %s", m_reconfig.m_cfgFileName);
    atLine += 1;

    // Update Mailbox Status
    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "Gse %s %s",
              m_gseInBox.GetStatusStr(),
              m_gseOutBox.GetStatusStr());
    atLine += 1;

    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "Cfg %s %s",
              m_reConfigInBox.GetStatusStr(),
              m_reConfigOutBox.GetStatusStr());
    atLine += 1;

    //debug_str(CmProc, atLine, 0, "%s", m_blankLine);
    debug_str(CmProc, atLine, 0, "Log %s %s",
              m_fileXferInBox.GetStatusStr(),
              m_fileXferOutBox.GetStatusStr());

    atLine += 1;
}


