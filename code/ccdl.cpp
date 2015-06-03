//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: ccdl.cpp
//
//    Description: Simulate the Cross Channel Data Link
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

#include "SecComm.h"
#include "video.h"
#include "ccdl.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
typedef struct CROSS_INFOtag {
    UINT32 paramIndex;
    UINT32 masterId;
    UINT32 seen;
} CROSS_INFO;

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
#undef CC_SLOT
#define CC_SLOT(Id,Name,Size) {Name,Size,0}

//Size information for each CC slot
static CC_SLOT_INFO m_slotInfo[] = {CC_SLOT_LIST};

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
static const CHAR adrfProcessName[] = "adrf";

// CCDL Mailbox Names
static const CHAR cmReCfgMailboxName[]   = "CM_RECONFIG_ADRF";  // Comm Manager Mailbox
static const CHAR adrfReCfgMailboxName[] = "ADRF_RECONFIG_CM";  // Adrf Mailbox

static BYTE m_inBuffer[CC_MAX_SIZE];
static BYTE m_outBuffer[CC_MAX_SIZE];


static CHAR* chanStr[] = {
    "A",
    "B"
};

static CHAR* modeStr[] = {
    "Start",
    "StartTx",
    "StartRx",
    "Run",
    "Hold"
};

static CHAR* stateStr[] = {
    "Err",
    "Ok",
    "Init"
};

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
//---------------------------------------------------------------------------------------------
// Function: Constructor
// Description: Create an initialize the CCDL object
//
CCDL::CCDL( AseCommon* pCommon )
    : m_pCommon(pCommon)
    , m_isValid(true)
    , m_inhibit(false)
    , m_actingChan(EFAST_CHA)
    , m_mode(eCcdlStart)
    , m_modeDelay(0)
    , m_rxState(eCcdlStateInit)
    , m_txState(eCcdlStateInit)
    , m_rxParam(0)
    , m_txParam(0)
    , m_parameters(NULL)
    , m_maxParamIndex(0)
    , m_wrCalls(0)
    , m_rdCalls(0)
{
    UINT32 current_offset = 0;

    // For each slot defined in the list, loop through and allocate the location
    // of the slots.  This sets up the pointers used to insert and remove messages
    // from the message buffer. If the size happens to exceed the size of the cross
    // channel message (2k) then flag an error.
    for(UINT32 i = 0; i < CC_MAX_SLOT; i++)
    {
        m_slotInfo[i].offset = current_offset;

        current_offset += m_slotInfo[i].size + sizeof(CC_MSG_SIZE);

        if(current_offset > (CC_MAX_SIZE))
        {
            m_isValid = false;
            break;
        }
    }

    memset( m_useCcdlItem, 0, sizeof(m_useCcdlItem));

    Reset();
}

//---------------------------------------------------------------------------------------------
// Function: Reset
// Description: Reset the data of the CCDL
//
void CCDL::Reset()
{
    // set the CCDL acting as channel ID
    if (m_pCommon->isChannelA)
    {
        m_actingChan = EFAST_CHB;
    }
    else
    {
        m_actingChan = EFAST_CHA;
    }

    m_ccdlCalls = 0;

    m_rxState = eCcdlStateInit;
    m_rxCount = 0;
    m_rxFailCount = 0;
    m_rdCalls = 0;
    memset((void*)m_rdReads, 0, sizeof(m_rdReads));

    m_txState = eCcdlStateInit;
    m_txCount = 0;
    m_txFailCount = 0;
    m_wrCalls = 0;
    memset((void*)m_wrWrites, 0, sizeof(m_wrWrites));

    m_mode = eCcdlStart;

    memset((void*)&m_rqstParamMap, 0, sizeof(m_rqstParamMap));
    memset((void*)&m_rxParamData, 0, sizeof(m_rxParamData));
    m_rqstParamMap.type = PARAM_XCH_TYPE_SETUP;

    memset((void*)&m_txParamData, 0, sizeof(m_rxParamData));
    m_txParamData.type = PARAM_XCH_TYPE_DATA;

    memset((void*)m_reportIn, 0, sizeof(m_reportIn));
    memset((void*)m_reportOut, 0, sizeof(m_reportOut));

    memset((void*)&m_eFastIn, 0, sizeof(m_eFastIn));
    memset((void*)&m_eFastOut, 0, sizeof(m_eFastOut));
    
    memset((void*)m_ccdlRawParam, 0, sizeof(m_ccdlRawParam));

    memset((void*)m_inBuffer, 0, sizeof(m_inBuffer));
    memset((void*)m_outBuffer, 0, sizeof(m_outBuffer));
}

//---------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Handle any command directed at the CCDL
//
BOOLEAN CCDL::CheckCmd( SecComm& secComm )
{
    BOOLEAN serviced = FALSE;
    ResponseType rType = eRspNormal;
    int itemId;

    SecRequest request = secComm.m_request;
    itemId = request.variableId;

    secComm.m_response.successful = TRUE;
    switch (request.cmdId)
    {
    case eSetCcdlItem:
        if (itemId == request.resetRequest)
        {
            m_useCcdlItem[itemId] = false;
        }
        else
        {
            m_useCcdlItem[itemId] = true;
            switch (itemId)
            {
            case eCcdlIdSource:
                if (itemId == eCcdlIdSource && 
                    request.sigGenId >= TIME_LOCAL_SRC && request.sigGenId < TIME_MAX_SRC)
                {
                    m_eFastHold.srcTime = (EFAST_TIME_SRC)request.sigGenId;
                }
                else
                {
                    m_useCcdlItem[itemId] = false;
                    secComm.ErrorMsg("CCDL Time Source Error: <%d>", request.sigGenId);
                    secComm.m_response.successful = FALSE;
                }
                break;
            case eCcdlIdElapsed:
                m_eFastHold.sysElapsedTime = request.sigGenId;
                break;
            case eCcdlIdLclFileCrc:
                m_eFastHold.lcFileCRC = request.sigGenId;
                break;
            case eCcdlIdCmbFileCrc:
                m_eFastHold.combinedFileCRC = request.sigGenId;
                break;
            case eCcdlIdLclXmlCrc:
                m_eFastHold.lcXMLCRC = request.sigGenId;
                break;
            case eCcdlIdAcidRx:
                m_eFastHold.bACIDRx = request.sigGenId == 1;
                break;
            case eCcdlIdAcidOk:
                m_eFastHold.bACIDOk = request.sigGenId == 1;
                break;
            }
        }
        serviced = TRUE;
        break;

    case eSetCcdlState:
        m_inhibit = (itemId == 0);
        serviced = TRUE;
        break;

    default:
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("Ccdl");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
}

//---------------------------------------------------------------------------------------------
// Function: Update
// Description: Handle all of the CCDL processing tasks
//
void CCDL::Update(MailBox& in, MailBox& out)
{
    UINT32 pIndex;
    static UINT32 rxTimer = 0;
    static UINT32 lastTx = 0;

    if (m_isValid)
    {
        m_ccdlCalls += 1;

        //if (m_pCommon->recfgSuccess || !IS_ADRF_ON)
        //{
        //    m_mode = eCcdlStart;
        //    m_pCommon->recfgSuccess = false;
        //}


        // we only use this timer if we are acting as channel A (see eCcdlStartTx)
        if (rxTimer > 0)
        {
            if (m_mode == eCcdlStartRx)
            {
                // when waiting for Chan B setup resend our setup after 1s
                if (++rxTimer > 100)
                {
                    // go back and transmit our request packet again
                    m_mode = eCcdlStartTx;
                    rxTimer = 0;
                }
            }
            else
            {
                rxTimer = 0;
            }
        }

        switch (m_mode) {
        case eCcdlStart:
            lastTx = 0;

            memset((void*)m_inBuffer, 0, sizeof(m_inBuffer));
            memset((void*)m_outBuffer, 0, sizeof(m_outBuffer));

            m_rxState = eCcdlStateInit;
            m_rxCount = 0;
            m_rxFailCount = 0;
            m_rdCalls = 0;
            memset((void*)m_rdReads, 0, sizeof(m_rdReads));

            m_txState = eCcdlStateInit;
            m_txCount = 0;
            m_txFailCount = 0;
            m_wrCalls = 0;
            memset((void*)m_wrWrites, 0, sizeof(m_wrWrites));

            if (m_actingChan == EFAST_CHA)
            {
                m_mode = eCcdlStartTx;
            }
            else
            {
                m_mode = eCcdlStartRx;
            }
            break;

        case eCcdlStartTx:
            // limit Tx to 900 ms interval so we don't overflow the buffer
            // TBD should this be 500 vs 900 to handle the restart logic of 2Hz?
            if ((lastTx + 90) < GET_SYSTEM_TICK)
            {
                lastTx = GET_SYSTEM_TICK;

                SendParamRqst();
                // on failure to Tx we will go back to start mode
                if (Transmit(out))
                {
                    if (m_actingChan == EFAST_CHA)
                    {
                        m_mode = eCcdlStartRx;
                        rxTimer = 1;
                    }
                    else
                    {
                        m_mode = eCcdlRun;
                    }
                }
            }
            break;

        case eCcdlStartRx:
            Receive(in);
            GetParamData();
            break;

        case eCcdlRun:
            Receive(in);
            GetParamData();
            Transmit(out);
            break;

        //case eCcdlHold:
        //    memset(m_inBuffer, 0, sizeof(m_inBuffer));
        //    memset(m_outBuffer, 0, sizeof(m_outBuffer));
        //    break;

        default:
            break;
        }

    }
}

//---------------------------------------------------------------------------------------------
// Function: Receive
// Description: Receive the ccdl msg and unpack it into our components
//
void CCDL::Receive(MailBox& in)
{
    // if we get something unpack it into our holder data
    if (m_inhibit)
    {
        in.Receive(m_inBuffer, sizeof(m_inBuffer));
        memset(m_inBuffer, 0, sizeof(m_inBuffer));
    }
    else if (in.Receive(m_inBuffer, sizeof(m_inBuffer)))
    {
        m_rxCount += 1;
    }
    else
    {
        m_rxFailCount += 1;
    }
}

//---------------------------------------------------------------------------------------------
// Function: Transmit
// Description: Pack up the msg components and send it.
//
bool CCDL::Transmit(MailBox& out)
{
    bool result = true;

    if (!m_inhibit)
    {
        result = out.Send(m_outBuffer, sizeof(m_outBuffer)) == TRUE;
    }

    if (result)
    {
        // clear sizes to signal buffer is free
        for(int i = 0; i < CC_MAX_SLOT; i++)
        {
          UINT16* size_ptr = (UINT16*)&m_outBuffer[m_slotInfo[i].offset];
          *size_ptr = 0;
        }
        m_txCount += 1;
    }
    else
    {
        m_txFailCount += 1;
        // maybe we want to go to start mode if we cannot talk to the ADRF
    }

    return result;
}

//---------------------------------------------------------------------------------------------
// Function: Write
// Description: Write Data to a particular slot
//
void  CCDL::Write(CC_SLOT_ID id, void* buf, INT32 size)
{
    //Buffer first location is 2-byte size, followed by data.
    BYTE* tx_buf = m_outBuffer + m_slotInfo[id].offset + sizeof(CC_MSG_SIZE);
    UINT16* buf_size = (UINT16*)(m_outBuffer + m_slotInfo[id].offset);

    m_wrCalls++;
    //Verify buffer is free (size = 0)
    if(*buf_size == 0)
    {
      memcpy(tx_buf, buf, size);
      //Set size last, used as a semaphore to signal data has been written
      // 16-bit write s/b atomic?
      *buf_size = (UINT16)size; //keep lint happy
      m_wrWrites[id]++;
    }
}

//---------------------------------------------------------------------------------------------
// Function: Read
// Description: Read Data from a particular slot if any new data is there
//
INT32 CCDL::Read(CC_SLOT_ID id, void* buf, INT32 size)
{
    UINT16 retval = 0;

    //Buffer first location is 2-byte size, followed by data.
    BYTE* rx_buf = m_inBuffer + m_slotInfo[id].offset + sizeof(CC_MSG_SIZE);
    UINT16* buf_size = (UINT16*)(m_inBuffer + m_slotInfo[id].offset);

    m_rdCalls++;
    //Verify buffer has data (size != 0)
    if(*buf_size != 0)
    {
        retval = MIN(size, *buf_size);
        memcpy(buf, rx_buf, retval);
        //Set size last, used as a semaphore to signal data has been written
        // 16-bit write s/b atomic?
        *buf_size = 0;
        m_rdReads[id]++;
    }

    return retval;
}

//---------------------------------------------------------------------------------------------
// Function: SendParamRqst
// Description: Send our request for param data
//
void CCDL::SendParamRqst()
{
    Write(CC_PARAM, &m_rqstParamMap, sizeof(m_rqstParamMap));
}

//---------------------------------------------------------------------------------------------
// Function: PackRequestParams
// Description: Pack our request for parameters, ask for all non xch data.  If param count is
// greater than the ccdl take the first PARAM_XCH_BUFF_MAX.  The tester is responsible for
// spreading the params out to ensure the ADRF code can send any parameter of its 3000 
// parameters
//
// NOTE: this is called from InitIoi when we know we are doing a Reconfig - if ASE is restarted
// you must Reconfig the target so ASE has some Param setup.
//
// TODO:
// SCR-300 - make sure we request ccdl params like the adrf does, a single entry for each 
//           masterId
//
void CCDL::PackRequestParams( Parameter* parameters, UINT32 maxParamIndex)
{
    UINT32 i;
    Parameter* aParam = parameters;

    // hold these for verifying Remote in CCDL param request
    m_parameters = parameters;
    m_maxParamIndex = maxParamIndex;
    m_rxParam = 0;

    memset((void*)&m_rqstParamMap, 0, sizeof(m_rqstParamMap));

    m_rqstParamMap.type = PARAM_XCH_TYPE_SETUP;
    for (i=0; i < maxParamIndex && m_rxParam < PARAM_XCH_BUFF_MAX; ++i)
    {
        // todo: make sure we only send
        if (aParam->m_isValid && aParam->m_src != PARAM_SRC_CROSS)
        {
            m_rqstParamMap.data[m_rxParam].id = m_rxParam;
            m_rqstParamIdMap[m_rxParam] = i;
            m_rqstParamMap.data[m_rxParam].val = aParam->m_masterId;
            m_rxParam += 1;
        }
        ++aParam;
    }

    m_rqstParamMap.num_params = m_rxParam;

    // reset us to start mode as we have a "new" request set
    m_mode = eCcdlStart;
}

//---------------------------------------------------------------------------------------------
// Function: GetParamRqst
// Description: Receive the ccdl request from the "remote chan" - the ADRF in our channel
//
//
// todo: when ever we receive a PARAM_XCH_TYPE_SETUP we must send ours
//       back - for async startup, remote lost, remote reconfig situations
//
void CCDL::GetParamData()
{
    INT32 pIndex;

    INT32 bytes = Read(CC_PARAM, &m_rxParamData, sizeof(m_rxParamData));
    if (bytes > 0)
    {
        // nothing to here really just transition chan B to send mode
        if (m_mode == eCcdlStartRx)
        {
            ValidateRemoteSetup();
        }
        else if ( m_mode == eCcdlRun)
        {
            bool paramsOk = true;
            if ( m_rxParamData.type == PARAM_XCH_TYPE_DATA)
            {
                // scatter the data to the param positions
                for (int x=0; x < m_rxParamData.num_params; ++x)
                {
                    pIndex = m_rqstParamMap.data[x].id;
                    if (!m_parameters[pIndex].m_isValid || 
                        m_parameters[pIndex].m_src == PARAM_SRC_CROSS)
                    {
                        paramsOk = false;
                    }
                    m_ccdlRawParam[pIndex] = m_rxParamData.data[x].val;
                }
                m_rxState = paramsOk ? eCcdlStateOk : eCcdlStateErr;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------
// Function: ValidateRemoteSetup
// Description: Verify the remote channel correctly built the request for us to Tx data to them
//
// SCR-300: Update this to scan down the list of parameters for each masterId seen 
//          Also verify that each masterId exists in the list received only once.
//          We may have 5 params with cross as src, but only three entries in here.
//
// Item 1. Collect all xch params in a list
// Item 2. tag each as found
// Item 3. make sure all are present in the request
// Item 4. make sure the request has only one entry for each master id
//
// Note: all params with the same masterId will point to the same slot, but only the fast (or one of
// the fastest) will be processed as a parent and sent across during the IoiUpdate processing.
//
void CCDL::ValidateRemoteSetup()
{
    UINT32 crossCount = 0;
    CROSS_INFO crossInfo[eAseMaxParams];
    
    if ( m_rxParamData.type == PARAM_XCH_TYPE_SETUP)
    {
        // scan down the parameter list and find all the params with src = cross
        // Item 1.
        for (int x=0; x < m_maxParamIndex; ++x)
        {
            if (m_parameters[x].m_isValid && m_parameters[x].m_src == PARAM_SRC_CROSS)
            {
                crossInfo[crossCount].paramIndex = x;
                crossInfo[crossCount].masterId = m_parameters[x].m_masterId;
                crossInfo[crossCount].seen = 0;
                crossCount += 1;
            }
        }

        // verify the right sensors have been sent src = CROSS
        // verify any masterId that are children - sent one with the fast update rate
        for (int slot = 0; slot < m_rxParamData.num_params; ++slot)
        {
            for (int x = 0; x < crossCount; ++x)
            {
                if (crossInfo[x].masterId == m_rxParamData.data[slot].val)
                {
                    // Item 2.
                    crossInfo[x].seen += 1;
                    m_parameters[crossInfo[x].paramIndex].m_ccdlId = slot;
                }
            }
        }
        
        // make sure all cross params were seen once and only once
        for (int x = 0; x < crossCount; ++x)
        {
            // Item 3/4
            if (crossInfo[x].seen != 1)
            {
                m_txState = eCcdlStateErr;
                break;
            }
        }

        // sequence the mode
        if (m_actingChan == EFAST_CHA)
        {
            m_mode = eCcdlRun;
        }
        else
        {
            m_mode = eCcdlStartTx;
        }
    }
}

//---------------------------------------------------------------------------------------------
// Function: PageCcdl
// Description: Update the CCDL display - uses the Ioi page
// display: acting channel, rx, tx, ParamTx, ParamRx
//
int CCDL::PageCcdl(int theLine, bool& nextPage, MailBox& in, MailBox& out)
{
    CHAR buffer[128];
    static int baseLine = 0;

    if (nextPage)
    {
        // capture the baseline
        baseLine = theLine;
        nextPage = false;
    }

    if (theLine == baseLine)
    {
        debug_str(Ioi, theLine, 0, "CCDL(%d) is %s: Mode(%s) Rx(%d/%d/%s) Tx(%d/%d/%s)",
                  m_ccdlCalls, chanStr[m_actingChan], modeStr[m_mode],
                  m_rxCount, m_rxFailCount, stateStr[m_rxState],
                  m_txCount, m_txFailCount, stateStr[m_txState]);
    }
    else if (theLine == (baseLine + 1))
    {
        debug_str(Ioi, theLine, 0, "CCDL: ParamRx(%d) ParamTx(%d) ParamRqst(%d)",
                  m_rxParamData.num_params, 
                  m_txParamData.num_params,
                  m_rqstParamMap.num_params);
    }

    else if (theLine == (baseLine + 2))
    {
        debug_str(Ioi, theLine, 0, "Rd(%d|%d|%d - %d) Wr(%d|%d|%d - %d)",
                  m_rdReads[CC_PARAM],
                  m_rdReads[CC_REPORT_TRIG],
                  m_rdReads[CC_EFAST_MGR],
                  m_rdCalls,
                  m_wrWrites[CC_PARAM],
                  m_wrWrites[CC_REPORT_TRIG],
                  m_wrWrites[CC_EFAST_MGR],
                  m_wrCalls);
    }

    else if (theLine == (baseLine + 3))
    {
        debug_str(Ioi, theLine, 0, "MB: %s %s",
            in.GetStatusStr(), out.GetStatusStr());
        nextPage = true;
    }

    // theLine += 1; - this is done by the caller we do not need to do it

    return theLine;
}

void CCDL::UpdateEfast()
{
    if (m_pCommon->newBaseTimeRqst != m_pCommon->newBaseTimeSrv)
    {
        TimeSrcObj* remTime = &m_pCommon->clocks[eClkRemote];
        TIMESTRUCT time;
        time.Year  = remTime->m_time.tm_year;
        time.Month = remTime->m_time.tm_mon;
        time.Day   = remTime->m_time.tm_mday;
        time.Hour  = remTime->m_time.tm_hour;
        time.Minute = remTime->m_time.tm_min;
        time.Second = remTime->m_time.tm_sec;

        // compute the new base time
        m_eFastHold.dateTime.secSinceBaseYr = remTime->GetSecSinceBaseYr();
        m_eFastHold.dateTime.ts = remTime->GetTimeStamp();
        m_pCommon->remElapsedMif = 0;

        m_pCommon->newBaseTimeSrv += 1;
    }

    if (Read(CC_EFAST_MGR, &m_eFastIn, sizeof(m_eFastIn)))
    {
        // loop it all back unless we are overriding a value
        m_eFastOut = m_eFastIn;

        // share our base time
        m_eFastOut.dateTime = m_eFastHold.dateTime;
        m_eFastOut.sysElapsedTime = m_pCommon->remElapsedMif;
        
        if (m_useCcdlItem[eCcdlIdSource]) m_eFastOut.srcTime =  m_eFastHold.srcTime;
        if (m_useCcdlItem[eCcdlIdElapsed]) m_eFastOut.sysElapsedTime =  m_eFastHold.sysElapsedTime;
        if (m_useCcdlItem[eCcdlIdLclFileCrc]) m_eFastOut.lcFileCRC =  m_eFastHold.lcFileCRC;
        if (m_useCcdlItem[eCcdlIdCmbFileCrc]) m_eFastOut.combinedFileCRC =  m_eFastHold.combinedFileCRC;
        if (m_useCcdlItem[eCcdlIdLclXmlCrc]) m_eFastOut.lcXMLCRC =  m_eFastHold.lcXMLCRC;
        if (m_useCcdlItem[eCcdlIdAcidRx]) m_eFastOut.bACIDRx =  m_eFastHold.bACIDRx;
        if (m_useCcdlItem[eCcdlIdAcidOk]) m_eFastOut.bACIDOk =  m_eFastHold.bACIDOk;

        Write(CC_EFAST_MGR, &m_eFastOut, sizeof(m_eFastOut));
    }
}
