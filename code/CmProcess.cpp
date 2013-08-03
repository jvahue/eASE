#include <deos.h>
#include <mem.h>
#include <string.h>
#include <stdio.h>

#include "CmProcess.h"
#include "video.h"

static CHAR blankLine[80];
static const CHAR adrfProcessName[] = "adrf";

CmProcess::CmProcess()
    : m_bRspPending(FALSE)
{
    // TODO: remove after debugging
    memset( m_readyFile, 0, sizeof(m_readyFile));
}

/****************************************************************************
 protected methods for FxProc
 ****************************************************************************/

void CmProcess::Run()
{

    // create alias for this process because adrf will be granting write access
    // to CMProcess, not ASE
    memset(blankLine, 0x20, sizeof(blankLine));
    processStatus ps = createProcessAlias( "CMProcess");

    // Set up mailboxes for processing reconfig msg from ADRF
    m_gseInBox.Create(CM_GSE_ADRF_MAILBOX, sizeof(m_gseRsp), eMaxQueueDepth);
    m_gseInBox.IssueGrant(adrfProcessName);

    // Connect to the the GSE recv box in the ADRF.
    m_gseOutBox.Connect(adrfProcessName, ADRF_GSE_CM_MAILBOX);

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

    debug_str(CmProc, 8, 0,"%s", blankLine);
    debug_str(CmProc, 8, 0, "GseRsp: %s", m_gseInBox.GetIpcStatusString());

    debug_str(CmProc, 11, 0, "GseRxFifo: %d", m_gseRxFifo.Used());
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
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description:
//
BOOLEAN CmProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = FALSE;
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

            debug_str(CmProc, 10, 0,"%s", blankLine);
            debug_str(CmProc, 10, 0, "GseCmd: %s", m_gseCmd.commandLine);

            if (m_gseOutBox.GetIpcStatus()     != ipcValid ||
                m_gseOutBox.GetProcessStatus() != processSuccess)
            {
                debug_str(CmProc, 12, 0,"%s",blankLine);
                debug_str(CmProc, 12, 0, "Mailbox send error Ipc: %s, Proc: %s",
                            m_gseOutBox.GetIpcStatusString(),
                            m_gseOutBox.GetProcessStatusString());
            }
            else
            {
                debug_str(CmProc, 12, 0,"%s",blankLine);
                debug_str(CmProc, 12, 0, "Mailbox CmProc -> Adrf OK");
            }
        }
        else
        {
            secComm.ErrorMsg("Command Length (%d) exceeds (%d)",
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
        serviced = TRUE;

        break;

    default:
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("CmProc");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
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
            m_putFile.Open( secComm.m_request.charData, secComm.m_request.sigGenId, 'w');
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
// variableId: 0 = Send file name in m_response.streamData
//             1 = send file data in m_response.streamData
// sigGenId: holds the partition Id
//
// When getting a file:
//    (1) if a file is ready to Tx the name will be put in m_response.streamData
//        otherwise m_response.streamData will be empty
//    (2) File data will be in m_response.streamData
//        when m_response.streamData is not max size or 0 the get file will close

//
bool CmProcess::GetFile( SecComm& secComm)
{
    INT32 bytesRead;
    UINT32 nameLength;
    bool status = false;

    // ePySte asking if a file is available
    if (secComm.m_request.variableId == 0)
    {
        // TODO: how do we determine is a file available?
        if (m_readyFile[0] != '\0')
        {
            m_getFile.Open( m_readyFile, secComm.m_request.sigGenId, 'r');

            nameLength = strlen(m_getFile.GetFileName());
            strncpy( secComm.m_response.streamData, m_getFile.GetFileName(), nameLength);
            secComm.m_response.streamSize = nameLength;
            status = true;
        }
        else
        {
            secComm.ErrorMsg("GetFile: No Log File available");
        }
    }
    else if (m_getFile.IsOpen())
    {
        bytesRead = m_getFile.Read(secComm.m_response.streamData, eSecStreamSize);
        if (bytesRead < eSecStreamSize)
        {
            m_getFile.Close();

            // TODO: remove after debugging is complete
            memset(m_readyFile, 0, sizeof(m_readyFile));
        }

        if (bytesRead >= 0)
        {
            status = true;
        }
    }
    else
    {
        secComm.ErrorMsg("GetFile: No file open for read");
    }

    return status;
}

