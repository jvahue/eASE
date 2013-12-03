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
#include "AseCommon.h"

#include "CmProcess.h"

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

static const CHAR pingCmd[] = "efast";

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
CmProcess::CmProcess()
    : m_requestPing(false)
    , m_lastGseSent(0)
    , m_performAdrfOffload(false)
    , m_reconfig(&aseCommon)
    , m_fileXfer(&aseCommon)
{
    // TODO: remove after debugging
    memset( m_rqstFile, 0, sizeof(m_rqstFile));
    memset( m_lastGseCmd, 0, sizeof(m_lastGseCmd));
    memset( m_boxOnTime, 0, sizeof(m_boxOnTime));
    memset( (void*)&m_gseCmd, 0, sizeof(m_gseCmd));
    memset( (void*)&m_gseRsp, 0, sizeof(m_gseRsp));
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
    static bool scriptStateZ = false;

    // Handle Reconfig Requests
    m_reconfig.ProcessCfgMailboxes(IS_MS_ONLINE, m_reConfigInBox, m_reConfigOutBox);

    // Handle File Xfer Request
    m_fileXfer.ProcessFileXfer(IS_MS_ONLINE, m_fileXferInBox, m_fileXferOutBox);

    ProcessGseMessages();

    if (m_gseOutBox.GetIpcStatus() == ipcValid)
    {
        m_pCommon->adrfState = eAdrfReady;
        m_requestPing = false;
    }
    // if the ADRF is on and we are not running a script see if the adrf is ready
    else if (m_pCommon->adrfState == eAdrfOn && !IS_SCRIPT_ACTIVE)
    {
        // at 1Hz see if the adrf is ready
        if ((m_frames - m_lastGseSent) > 100)
        {
            // if we are invalid 
            if (m_gseOutBox.GetIpcStatus() != ipcValid)
            {
                // after 120 sec reset the MB
                if (m_gseOutBox.m_connectAttempts < CmReconfig::eCmAdrfFactoryRestart)
                {
                    m_gseOutBox.Send((void*)pingCmd, sizeof(pingCmd));

                    // back off detecting a valid MB
                    m_lastGseSent = m_frames;
                    m_requestPing = false;
                }
                else
                {
                    m_gseOutBox.Reset();
                }
            }
        }
    }

    // if the script is not running - oneshot reset ASE process states
    if (!IS_SCRIPT_ACTIVE && scriptStateZ)
    {
        // Recfg Task
        m_reconfig.Init();
        m_getFile.Close();
        m_putFile.Close();

        // File Xfer - don't do anything here - this is reset on ADRF Power down as the ADRF
        // .. might have requested a file transfer and we have not done anything with it yet.
        // .. On Power down ADRf will restart all file transfers - see HandlePowerOff
    }

    scriptStateZ = IS_SCRIPT_ACTIVE;
}

//-------------------------------------------------------------------------------------------------
// Function: ProcessGseMessages
// Description:
//
void CmProcess::ProcessGseMessages()
{
    if (m_gseInBox.GetIpcStatus() == ipcValid)
    {
        m_gseCmd.gseSrc = GSE_SOURCE_CM;
        m_gseCmd.gseVer = 1;
        memset(m_gseRsp.rspMsg, 0, sizeof(m_gseRsp.rspMsg) );

        // If not expecting a resp msg and time has elapsed to request the
        // Expecting cmd response ... check inbox.
        if( m_gseInBox.Receive(&m_gseRsp, sizeof(m_gseRsp)) )
        {
            int size = strlen(m_gseRsp.rspMsg);
            m_gseRxFifo.Push(m_gseRsp.rspMsg, size);
        }
    }
    else
    {
        m_gseInBox.Reset();
    }
}

//-------------------------------------------------------------------------------------------------
// Function: HandlePowerOff
// Description:
//
void CmProcess::HandlePowerOff()
{
    // reset mailboxes due to power off (adrf process gone)
    m_gseInBox.Reset();
    m_gseOutBox.Reset();

    m_reConfigInBox.Reset();
    m_reConfigOutBox.Reset();
    if (!IS_SCRIPT_ACTIVE)
    {
        m_reconfig.Init();
    }

    m_fileXferInBox.Reset();
    m_fileXferOutBox.Reset();

    m_requestPing = false;
    m_lastGseSent = m_frames;  // when power comes back give the give ePySte time to send cmd

    m_fileXfer.ResetCounters();
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
            //GSE_COMMAND* mb;
            port = request.variableId;  // 0 = gse, 1 = ms

            memcpy((void*)m_gseCmd.commandLine, (void*)request.charData, request.charDataSize);
            m_gseCmd.commandLine[request.charDataSize] = '\0';

            m_gseOutBox.Send(&m_gseCmd, sizeof(m_gseCmd));
            secComm.m_response.successful = TRUE;
            m_lastGseSent = m_frames;

            //sprintf(secComm.m_response.errorMsg, "CmProcess(%s): Unable to send command %s <%s>",
            //            m_gseOutBox.IsConnected() ? "Conn" : "NoConn",
            //            m_gseOutBox.GetIpcStatusString(),
            //            m_gseOutBox.GetProcessStatusString());


            // terminate the cmd before the CR

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

    case eDeleteFile:
        // see if the file exists
        secComm.m_response.successful = m_getFile.Delete(request.charData,
                                                         File::PartitionType(request.variableId));
        if (!secComm.m_response.successful)
        {
            secComm.ErrorMsg("Failed File Delete <%s> Part(%) errorCode: %d",
                             request.charData, request.variableId, m_getFile.GetFileError());
            secComm.m_response.value = float(m_getFile.GetFileError());
        }

        serviced = TRUE;
        break;

    case eFileExists:
        // see if a file exists on target
        if ( m_getFile.Open(secComm.m_request.charData,
                            File::PartitionType(secComm.m_request.sigGenId), 'r'))
        {
            secComm.m_response.successful = TRUE;
        }
        else
        {
            if (m_getFile.GetFileError() != eFileNotFound)
            {
               secComm.ErrorMsg("Failed File Exists Check Error(%d)", m_getFile.GetFileError());
            }
            secComm.m_response.successful = FALSE;
        }
        m_getFile.Close();

        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eDisplayState:
        if (request.variableId == (int)CmProc)
        {
            m_updateDisplay = request.sigGenId != 0;
            secComm.m_response.successful = true;
            serviced = TRUE;
        }
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
            strcpy(m_rqstFile, secComm.m_request.charData);
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
            // charData holds the file name requested by ePySte
            memset(m_rqstFile, 0, sizeof(m_rqstFile));
            memcpy(m_rqstFile, secComm.m_request.charData, secComm.m_request.charDataSize);

            if (m_getFile.Open(m_rqstFile, File::PartitionType(secComm.m_request.sigGenId), 'r'))
            {
                m_fileXfer.FileStatus(m_rqstFile, true);

                // check to see if we are uploading a FileXfer request from ADRF
                m_performAdrfOffload = strcmp( m_rqstFile, m_fileXfer.m_xferFileName) == 0;

                nameLength = strlen(m_getFile.GetFileName());
                strncpy( secComm.m_response.streamData, m_getFile.GetFileName(), nameLength);
                secComm.m_response.streamSize = nameLength;

                // return the file size
                secComm.m_response.value = float(m_getFile.GetFileSize());

                status = true;
            }
            else
            {
                m_fileXfer.FileStatus(m_rqstFile, false);
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
        bool continueAdrf = strcmp( m_rqstFile, m_fileXfer.m_xferFileName) == 0;

        if ((m_performAdrfOffload && continueAdrf) || !m_performAdrfOffload)
        {
            bytesRead = m_getFile.Read(secComm.m_response.streamData, eSecStreamSize);
            secComm.m_response.streamSize = bytesRead;
            if (bytesRead < eSecStreamSize)
            {
                m_getFile.Close();
                memset(m_rqstFile, 0, sizeof(m_rqstFile));
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
            memset(m_rqstFile, 0, sizeof(m_rqstFile));
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
int CmProcess::UpdateDisplay(VID_DEFS who, int theLine)
{
    // this needs to be big so that it can handle when file names are put in it
    char buffer[256];

    switch (theLine) {
    case 0:
        CmdRspThread::UpdateDisplay(CmProc, 0);
        break;

    case 1:
       debug_str(CmProc, theLine, 0, "GseCmd: %s", m_lastGseCmd);
       break;

    case 2:
        debug_str(CmProc, theLine, 0, "Gse RxFifo: %d", m_gseRxFifo.Used());
        break;

    case 3:
        debug_str(CmProc, theLine, 0, "Cfg(%d/%d/%s) Mode/Status: %s(%d)/%s(%s)",
                 m_reconfig.m_recfgCount, m_reconfig.m_recfgCmds,
                 m_reconfig.GetLastCmd(),
                 m_reconfig.GetModeName(),
                 m_reconfig.m_modeTimeout,
                 m_reconfig.m_lastReCfgFailed ? "Err" : "Ok",
                 m_reconfig.GetCfgStatus());
        break;

    case 4:
        debug_str(CmProc, theLine, 0, "Log(%d/%d) Rx/Tx: %d/%d Mode: %s(%d)",
            m_fileXfer.m_fileXferRqsts,
            m_fileXfer.m_fileXferServiced,
            m_fileXfer.m_fileXferRx,
            m_fileXfer.m_fileXferTx,
            m_fileXfer.GetModeName(),
            m_fileXfer.m_modeTimeout
            );
        break;

    case 5:
        debug_str(CmProc, theLine, 0, "Log Stats S/F/FL(%d/%d/%d) Bad/Crc:Snd-Rsp/Fmis(%d/%d-%d/%d)",
            m_fileXfer.m_fileXferSuccess,
            m_fileXfer.m_fileXferFailed,
            m_fileXfer.m_fileXferFailLast,
            m_fileXfer.m_fileXferError,
            m_fileXfer.m_failCrcSend,
            m_fileXfer.m_fileXferValidError,
            m_fileXfer.m_noMatchFileName
            );
        break;

    case 6:
        // Show put file status
        debug_str(CmProc, theLine, 0, "XFILE: %s", m_fileXfer.m_xferFileName);
        break;

    case 7:
        // Show put file status
        debug_str(CmProc, theLine, 0, "PUT: %s", m_putFile.GetFileStatus(buffer));
        break;

    case 8:
        // Show get file status
        debug_str(CmProc, theLine, 0, "GET: %s", m_getFile.GetFileStatus(buffer));
        break;

    case 9:
        // Show Cfg file names
        debug_str(CmProc, theLine, 0, "XML: %s", m_reconfig.m_xmlFileName);
        break;

    case 10:
        // Show Cfg file names
        debug_str(CmProc, theLine, 0, "CFG: %s", m_reconfig.m_cfgFileName);
        break;

    case 11:
        // Update Mailbox Status
        debug_str(CmProc, theLine, 0, "Gse %s %s",
                  m_gseInBox.GetStatusStr(),
                  m_gseOutBox.GetStatusStr());
        break;

    case 12:
        debug_str(CmProc, theLine, 0, "Cfg %s %s",
                  m_reConfigInBox.GetStatusStr(),
                  m_reConfigOutBox.GetStatusStr());
        break;

    case 13:
        debug_str(CmProc, theLine, 0, "Log %s %s",
                  m_fileXferInBox.GetStatusStr(),
                  m_fileXferOutBox.GetStatusStr());
        break;

    default:
        theLine = -1;
        break;
    }

    // go to the next line or back to 0
    theLine += 1;

    return theLine;
}


