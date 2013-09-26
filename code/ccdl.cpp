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
static const CHAR cmReCfgMailboxName[]   = "CM_RECONFIG_ADRF";   // Comm Manager Mailbox
static const CHAR adrfReCfgMailboxName[]  = "ADRF_RECONFIG_CM";  // Adrf Mailbox

static BYTE m_inBuffer[CC_MAX_SIZE];
static BYTE m_outBuffer[CC_MAX_SIZE];

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
//-------------------------------------------------------------------------------------------------
// Function: Constructor
// Description: Create an initialize the CCDL object
//
CCDL::CCDL( AseCommon* pCommon )
    : m_pCommon(pCommon)
    , m_rxCount(0)
    , m_txCount(0)
    , m_isValid(true)
    , m_mode(eCcdlStart)
    , m_ccdlSetupError(false)
    , m_modeDelay(0)
    , m_rxFailCount(0)
    , m_txFailCount(0)
    , m_parameters(NULL)
    , m_maxParamIndex(0)
{
    void *theAreg;
    accessStyle asA;
    UNSIGNED32 myChanID;
    resourceStatus  status;
    platformResourceHandle hA;

    UINT32 current_offset = 0;

    //For each slot defined in the list, loop through and allocate the location
    //of the slots.  This sets up the pointers used to insert and remove messages
    //from the message buffer. If the size happens to exceed the size of the cross
    //channel message (2k) then flag an error.
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

    status = attachPlatformResource("", "FPGA_CHAN_ID", &hA, &asA, &theAreg);

    // what channel will we act as?
    if (status == resValid)
    {
#ifndef VM_WARE
        status = readPlatformResourceDWord(hA, 0, &myChanID);
#else
        myChanID = 0x00;  // force to Ch A
#endif
    }

    // but we are the other channel
    m_actingChan = EFAST_CHA;
    if (status == resValid)
    {
        if ( (myChanID & 0x03) == 1 )
        {
            m_actingChan = EFAST_CHB;
        }
        else
        {
            m_actingChan = EFAST_CHA;
        }
    }

    memset((void*)&m_rxMap, 0, sizeof(m_rxMap));
    memset((void*)&m_txSet, 0, sizeof(m_txSet));
    memset((void*)&m_remoteParam, 0, sizeof(m_remoteParam));
    m_packMap[CC_PARAM].slotId = CC_PARAM;
    m_packMap[CC_PARAM].size = sizeof(m_rxMap);
    m_packMap[CC_PARAM].in   = &m_txSet;
    m_packMap[CC_PARAM].out  = &m_remoteParam;
    m_rxMap.type = PARAM_XCH_TYPE_MAX;
    m_txSet.type = PARAM_XCH_TYPE_MAX;

    memset((void*)m_reportIn, 0, sizeof(m_reportIn));
    memset((void*)m_reportOut, 0, sizeof(m_reportOut));
    m_packMap[CC_REPORT_TRIG].slotId = CC_REPORT_TRIG;
    m_packMap[CC_REPORT_TRIG].size = sizeof(m_reportIn);
    m_packMap[CC_REPORT_TRIG].in   = &m_reportIn;
    m_packMap[CC_REPORT_TRIG].out  = &m_reportOut;

    memset((void*)&m_eFastIn, 0, sizeof(m_eFastIn));
    memset((void*)&m_eFastOut, 0, sizeof(m_eFastOut));
    m_packMap[CC_EFAST_MGR].slotId = CC_EFAST_MGR;
    m_packMap[CC_EFAST_MGR].size = sizeof(m_eFastIn);
    m_packMap[CC_EFAST_MGR].in   = &m_eFastIn;
    m_packMap[CC_EFAST_MGR].out  = &m_eFastOut;
    
    memset((void*)m_ccdlRawParam, 0, sizeof(m_ccdlRawParam));
}

//-------------------------------------------------------------------------------------------------
// Function: Update
// Description: Handle all of the CCDL processing tasks
//
void CCDL::Update(MailBox& in, MailBox& out)
{
    UINT32 pIndex;

    if (m_isValid)
    {
        if (m_pCommon->recfgSuccess)
        {
            m_mode = eCcdlStart;
            m_pCommon->recfgSuccess = false;
        }

        Receive(in);

        switch (m_mode) {
        case eCcdlStart:
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
            break;

        case eCcdlStartRx:
            if (m_remoteParam.type == PARAM_XCH_TYPE_SETUP)
            {
                GetParamRqst(in);
            }
            m_remoteParam.type = PARAM_XCH_TYPE_MAX;
            break;

        case eCcdlRun:
            if ( m_remoteParam.type == PARAM_XCH_TYPE_DATA)
            {
                // scatter the data to the param positions
                for (int x=0; x < m_rxMap.num_params; ++x)
                {
                    pIndex = m_rxMap.data[x].id;
                    m_ccdlRawParam[pIndex] = m_remoteParam.data[x].val;
                }
            }
            m_remoteParam.type = PARAM_XCH_TYPE_MAX;
            break;

        default:
            break;
        }

        Transmit(out);
    }
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Handle any command directed at the CCDL
//
BOOLEAN CCDL::CheckCmd( SecComm& secComm )
{
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateDisplay
// Description: Update the CCDL display maybe use the Ioi page
//
int CCDL::UpdateDisplay( VID_DEFS who, int theLine )
{
    return theLine;
}

//-------------------------------------------------------------------------------------------------
// Function: PackRequestParams
// Description: Send over our request for parameters, ask for all non xch data.  If param count is
// greater than the ccdl take the first PARAM_XCH_BUFF_MAX.  The tester is responsible for
// spreading the params out to ensure the ADRF code can send parameter of its 3000 parameters
//
void CCDL::PackRequestParams( Parameter* parameters, UINT32 maxParamIndex)
{
    Parameter* aParam = parameters;

    // hold these for verifying Remote in CCDL param request
    m_parameters = parameters;
    m_maxParamIndex = maxParamIndex;

    memset((void*)&m_rxMap, 0, sizeof(m_rxMap));

    m_rxMap.type = PARAM_XCH_TYPE_SETUP;
    for (int i=0; i < maxParamIndex && m_rxMap.num_params < PARAM_XCH_BUFF_MAX; ++i)
    {
        if (aParam->m_src != PARAM_SRC_CROSS)
        {
            m_rxMap.data[m_rxMap.num_params].id = i;
            m_rxMap.data[m_rxMap.num_params].val = aParam->m_masterId;
            m_rxMap.num_params += 1;
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Function: GetParamRqst
// Description: Receive the ccdl request from the "remote chan" really the same one we are in.
//
void CCDL::GetParamRqst(MailBox& in)
{
    // nothing to here really just transition chan B to send mode
    ValidateRemoteSetup();
    if (m_actingChan == EFAST_CHB)
    {
        m_mode = eCcdlStartTx;
    }
    else
    {
        m_mode = eCcdlRun;
    }
}

//-------------------------------------------------------------------------------------------------
// Function: SendParamRqst
// Description: Verify the remote channel correctly built their request of us
//
void CCDL::SendParamRqst( MailBox& out )
{
}

//-------------------------------------------------------------------------------------------------
// Function: Receive
// Description: Receive the ccdl msg and unpack it into our components
//
void CCDL::Receive(MailBox& in)
{
    // if we get something unpack it into our holder data
    if (in.Receive(m_inBuffer, sizeof(m_inBuffer)))
    {
        m_rxCount += 1;

        // for each item in the CCDL unpack it into our data
        for (int i = 0; i < CC_MAX_SLOT; ++i)
        {
            BYTE* src = (BYTE*)(m_inBuffer + m_slotInfo[i].offset + sizeof(UINT16));
            memcpy( m_packMap[i].in, src, m_packMap[i].size);
        }
    }
    else
    {
        m_rxFailCount += 1;
    }
}

//-------------------------------------------------------------------------------------------------
// Function: Transmit
// Description: Pack up the msg components and send it.
//
void CCDL::Transmit(MailBox& out)
{
    // for each item in the CCDL pack it into our Tx data
    for (int i = 0; i < CC_MAX_SLOT; ++i)
    {
        BYTE* dst = (BYTE*)(m_outBuffer + m_slotInfo[i].offset);
        *(UINT16*)dst = m_packMap[i].size;

        if (m_mode == eCcdlStartTx && i == CC_PARAM && m_rxMap.type == PARAM_XCH_TYPE_SETUP)
        {
            memcpy( &dst[2], &m_rxMap, m_packMap[i].size);
            m_rxMap.type = PARAM_XCH_TYPE_MAX;
        }
        else
        {
            memcpy( &dst[2], m_packMap[i].out, m_packMap[i].size);
        }
    }

    m_txCount += 1;
    if (!out.Send(m_outBuffer, sizeof(m_outBuffer)))
    {
        m_txFailCount += 1;
    }
}

//-------------------------------------------------------------------------------------------------
// Function: ValidateRemoteSetup
// Description: Verify the remote channel correctly built their request of us
//
void CCDL::ValidateRemoteSetup()
{
    UINT32 paramCount = 0;
    PARAM_XCH_ITEM* pParam = &m_remoteParam.data[0];

    // verify the right sensor have been sent src = CROSS
    for (int x = 0; x < m_maxParamIndex; ++x)
    {
        if (m_parameters[x].m_src == PARAM_SRC_CROSS)
        {
            if (m_parameters[x].m_masterId = pParam->val)
            {
                if (paramCount == pParam->id)
                {
                    paramCount += 1;
                }
                else
                {
                    m_ccdlSetupError = true;
                }
            }
            else
            {
                m_ccdlSetupError = true;
            }
        }
    }

    m_ccdlSetupError = paramCount != m_remoteParam.num_params;
}
