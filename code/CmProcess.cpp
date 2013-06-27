#include "CmProcess.h"
#include <deos.h>
#include <mem.h>
#include <string.h>

#include "video.h"

static const CHAR adrfProcessName[] = "adrf";

CmProcess::CmProcess()
	:m_bRspPending(FALSE)
{
}

/****************************************************************************
 protected methods for FxProc
 ****************************************************************************/

void CmProcess::Create()
{
	// create alias for this process because adrf will be granting write access
	// to CMProcess, not ASE

	processStatus ps = createProcessAlias( "CMProcess");

	// Set up mailboxes for processing reconfig msg from ADRF
	m_gseInBox.Create(CM_GSE_ADRF_MAILBOX, sizeof(m_gseRsp), eMaxQueueDepth);
	m_gseInBox.GrantProcess(adrfProcessName);

    // Connect to the the GSE recv box in the ADRF.
	m_gseOutBox.Connect(adrfProcessName, ADRF_GSE_CM_MAILBOX);

	// Create the thread thru the base class method.
	// Use the default Ase template
	AseThread::Create("CmProcess", "StdThreadTemplate");

}


void CmProcess::Process()
{
	UNSIGNED32* pSystemTickTime;
	UNSIGNED32  nextRequestTime;
	UNSIGNED32  nowTime;
	UNSIGNED32  interval = 100;  // 100 X 10 millisecs/tick = 1 sec interval

	// Grab the system tick pointer
	pSystemTickTime = systemTickPointer();

	nextRequestTime = *pSystemTickTime + interval;

	m_gseCmd.gseSrc = GSE_SOURCE_CM;
	m_gseCmd.gseVer = 42;
	memset(m_gseRsp.rspMsg,0x20, sizeof(m_gseRsp.rspMsg) );
	strncpy(m_gseCmd.commandLine, "EFAST.STATUS.STATS.POWERONTIME", sizeof(m_gseCmd.commandLine));

    while (1)
    {
    	nowTime = *pSystemTickTime;
    	// If not expecting a resp msg and time has elapsed to request the
    	// power-on count... do it.
    	if (!m_bRspPending && nextRequestTime < nowTime)
    	{
    		// send a box  pwer on time request
    		if ( m_gseOutBox.Send(&m_gseCmd, sizeof(m_gseCmd)))
    		{
    			m_bRspPending = TRUE;
    		}
    	}

		if (m_bRspPending == TRUE)
    	{
    		// Expecting cmd response... check inbox.
    		if( m_gseInBox.Receive(&m_gseRsp, sizeof(m_gseRsp)) )
    		{
    			m_bRspPending = FALSE;
    			strncpy(m_boxOnTime, m_gseRsp.rspMsg, strlen(m_gseRsp.rspMsg));
    			nextRequestTime = nowTime + interval;
    		}
    	}

    	waitUntilNextPeriod();
    }
}
