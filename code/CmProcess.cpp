#include <deos.h>
#include <mem.h>
#include <string.h>
#include <stdio.h>

#include "CmProcess.h"
#include "video.h"

static CHAR blankLine[80];
static const CHAR adrfProcessName[] = "adrf";

CmProcess::CmProcess()
    :m_bRspPending(FALSE)
{
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
    m_gseInBox.GrantProcess(adrfProcessName);

    // Connect to the the GSE recv box in the ADRF.
    m_gseOutBox.Connect(adrfProcessName, ADRF_GSE_CM_MAILBOX);

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("CmProcess", "StdThreadTemplate");

}


void CmProcess::RunSimulation()
{
    UNSIGNED32* pSystemTickTime;
    UNSIGNED32  nextRequestTime;
    UNSIGNED32  nowTime;
    UNSIGNED32  interval = 100;  // 100 X 10 millisecs/tick = 1 sec interval

    // Grab the system tick pointer
    pSystemTickTime = systemTickPointer();

    m_gseCmd.gseSrc = GSE_SOURCE_CM;
    m_gseCmd.gseVer = 1;
    memset(m_gseRsp.rspMsg, 0, sizeof(m_gseRsp.rspMsg) );

    while (1)
    {
        nowTime = *pSystemTickTime;
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

        waitUntilNextPeriod();
    }
}

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
            sprintf(secComm.m_response.errorMsg, "Command Length (%d) exceeds (%d)",
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

