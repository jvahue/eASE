//-----------------------------------------------------------------------------
//          Copyright (C) 2014-2017 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: A717QAR.cpp
//
//    Description: Implements test control object for the UTAS A717 Module
//
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "AseCommon.h"
#include "alt_basic.h"
#include "ioiStaticCls.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define QAR_A717_IOI_NAME1 "A717Subframe1"
#define QAR_A717_IOI_NAME2 "A717Subframe2"
#define QAR_A717_IOI_NAME3 "A717Subframe3"
#define QAR_A717_IOI_NAME4 "A717Subframe4"

#define QAR_A717_STATUS_NAME "A717Status"
//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Local Variables                                                           -/
//----------------------------------------------------------------------------/

// Cfg Mode settings.

// Written by A717QAR, READ-ONLY by A717Subframe
BOOLEAN     g_bReverse;     // Use reverse barker id 0 = normal, 1 = reverse
UINT16      g_frameSize;    // Number of 32 bit words in the subframe
QAR_FORMAT  g_fmt;          // 



//----------------------------------------------------------------------------/
// Constant Data                                                             -/
//----------------------------------------------------------------------------/
//#include "AseStaticIoiInfo.h"

const CHAR* ioiSfName[] = { QAR_A717_IOI_NAME1,
                            QAR_A717_IOI_NAME2,
                            QAR_A717_IOI_NAME3,
                            QAR_A717_IOI_NAME4 };

const CHAR* ioiStatusName = QAR_A717_STATUS_NAME;

UINT16 ReverseBarker(UINT16 barker)
{

    UINT16 revBarker;
    UINT16 x;

    // Shift the bits thru each other left to right.
    x = barker;
    x = (((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1));
    x = (((x & 0xcccc) >> 2) | ((x & 0x3333) << 2));
    x = (((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4));
    x = (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8));
    revBarker = ((x >> 8) | (x << 8));

    // the reversed value is aligned in the 
    // high-order byte, Barker is 12 bits, so left-shift 4.
    revBarker >>= 4;
    return revBarker;
}

//---------------------------------------------------------------------------------------------
// General function used to open Ioi controlled by the A717Qar object directly
BOOLEAN OpenIoi(const CHAR* pIoiName, A717_IOI_STRUCT* pIOIStruct,
                void* pMsgBuffer, UINT32 buffSize, BOOLEAN* pIoiValid,
                ioiPermissionType permission)
{
  // Init  QAR_STATUS IOI
  // Open the output IOI for this subframes data
    ioiStatus openStatus;
    ioiStatus closeStatus;
    memset(pMsgBuffer, 0, buffSize);
    pIOIStruct->pdata = (UINT32*)pMsgBuffer;
    if (*pIoiValid)
    {
        closeStatus = ioi_close(pIOIStruct->fd);
    }
    if (closeStatus == ioiSuccess)
    {
        *pIoiValid = FALSE;
    }
    openStatus = ioi_open(pIoiName, permission, &pIOIStruct->fd);
    if (openStatus == ioiSuccess)
    {
        *pIoiValid = TRUE;
    }
    return *pIoiValid;
}
//=============================================================================================
// Public interfaces
//=============================================================================================

A717Qar::A717Qar()
    , m_bInit(FALSE) // flag to do post construct init
    , m_nextSfIdx(0)
    , m_crntTick(0)
    , m_writeErrCnt(0)
    //, m_startTime(0)
    //, m_elapsedTime( 0 )
{
    g_frameSize = 64;     // Set the default frame size
    g_bReverse = FALSE;  // default to normal barker sync
    g_fmt = QAR_BIPOLAR_RETURN_ZERO;

    m_testCtrl.m_acceptCfgReq = TRUE;
    m_testCtrl.m_bQarEnabled = TRUE;
    m_testCtrl.m_bSynced = TRUE;
    m_testCtrl.qarRunState = eRUNNING;
}

void A717Qar::Reset(StaticIoiObj* cfgRqst, StaticIoiObj* cfgRsp, StaticIoiObj* sts, 
                    StaticIoiObj* sf1, StaticIoiObj* sf2, 
                    StaticIoiObj* sf3, StaticIoiObj* sf4)
{

}

//---------------------------------------------------------------------------------------------
void A717Qar::InitIoi()
{
    // Initialize the cfg and status IOI's. Tell the subframe objs to do likewise
    Initialize();

    // TODO OpenIOi for recfg Req
    // TODO OpenIOi for recfg Resp
    
    for (int i = 0; i < eNUM_SUBFRAMES; ++i)
    {
        m_pSF[i]->InitIoi();
    }
}

//---------------------------------------------------------------------------------------------
int A717Qar::UpdateIoi()
{
    // This function is called at 100Hz
    // Determine if it is time to send out a 1Hz SF/status update, etc...
    // Check if QAR is enabled for this configuration
    UINT8 subFrameID;

    if (m_bInitSFOutput)
    {
        // On startup the "next" SF will be  #1 (index 0).
        // send it during this processing frame
        m_nextSfIdx = 0;
        m_crntTick = eTICKS_PER_SEC;
        m_bInitSFOutput = FALSE;
    }
    else
    {
        // Update the tick count
        m_crntTick += 1;
    }

    // Is it time to send a SF - do this every 1Hz?
    // Output the next SF (if QAR active) and the status msg.
    if (m_crntTick >= eTICKS_PER_SEC)  
    {
        // If enabled and running, send the next expected subframe...
        if (m_testCtrl.m_bQarEnabled && m_testCtrl.qarRunState == eRUNNING)
        {
            // send next SF data via its IOI
            if (false == m_pSF[m_nextSfIdx]->UpdateIoi())
            {
                m_writeErrCnt += 1;
            }
            else
            {
                subFrameID = m_nextSfIdx + 1;
            }

            // Set up next SF to go out in 1 sec.      
            m_nextSfIdx = INC_WRAP(m_nextSfIdx, eNUM_SUBFRAMES);
        }
        else
        {
            // UTAS ICD says: QAR_STATUS.subframeID shall be zero if no data sent.
            // TBDjv: maybe this should default to this and only increment when 
            // m_pSF[m_nextSfIdx]->UpdateIoi() is true - do we need to set = 0 when UpdateIoi 
            // fails? not doing this
            subFrameID = 0;
        }

        // A QAR_STATUS msg is ALWAYS sent at 1Hz, regardless of run-state
        WriteQarStatusMsg(subFrameID);

        m_crntTick = 0;
    } 

    return 0;
}

//---------------------------------------------------------------------------------------------
void A717Qar::WriteQarStatusMsg(UINT8 subframeID)
{
    UINT32 reg = 0x0000F51E; // All OK
    ioiStatus ioiStat;

    // The ICD defines the mode as disabled vs enabled.
    m_qarModStatus.bDisabled = m_testCtrl.m_bQarEnabled ? 0 : 1;

    m_qarModStatus.qarRunState = m_testCtrl.qarRunState;
    m_qarModStatus.subframeID = subframeID;
    m_qarModStatus.bRevSync = g_bReverse;
    m_qarModStatus.numWords = g_frameSize;
    m_qarModStatus.fmt = (UINT32)g_fmt;

    // Set the overall sync status to match the high level flag in status msg
    UINT32 syncStatus = (m_testCtrl.m_bSynced) ? 1 : 0;
    reg = reg | (syncStatus &  SYNC_MASK);

    m_qarModStatus.statusRegister = reg;

    if (m_statusIoiValid)
    {
        ioiStat = ioi_write(m_moduleStatusIoi.fd, m_moduleStatusIoi.pdata);
        m_statusIoiValid = (ioiStat == ioiSuccess);
    }

}

//---------------------------------------------------------------------------------------------
void A717Qar::SetRunState(QAR_RUN_STATE newState)
{
    // Put the QAR in a specific mode so we can do some mischief.
    if (newState != m_qarModStatus.qarRunState)
    {
        switch (newState)
        {
        case eRUNNING:
            // Init the output logic to send data starting with first SF
            m_bInitSFOutput = TRUE;
            break;

            // fall-through these for now...
        case eNO_VALID_CFG:
        case eERR_CBIT:
        case eERR_PBIT:
            break;

        default:
            break;

        } // switch QAR_RUN_STATE
        m_qarModStatus.qarRunState = newState;
    } // mode changed 
    return;
}

//---------------------------------------------------------------------------------------------
void A717Qar::SetQarData(UINT8 sfMask, UINT32 offset, UINT8* pBuffer, UINT32 byteCnt)
{
    // For each SF indicated in the sfMask, update the Subframes' buffer with the content
    for (int i = 0; i < eNUM_SUBFRAMES; ++i)
    {
        if ((sfMask >> i) & 0x01)
        {
            m_pSF[i]->UpdateBuffer(offset, pBuffer, byteCnt);
        }
    }
}

//---------------------------------------------------------------------------------------------
void A717Qar::ResetBarkers(UINT8 sfMask)
{
    // For each SF indicated in the sfMask, tell the SF to use it's native barker code.
    for (int i = 0; i < eNUM_SUBFRAMES; ++i)
    {
        if ((sfMask >> i) & 0x01)
        {
            m_pSF[i]->ResetBarker();
        }
    }
}

//=============================================================================================
// Protected methods
//=============================================================================================


//---------------------------------------------------------------------------------------------
void A717Qar::SetWordSize(UINT32 wordSize)
{
    g_frameSize = wordSize;
}

//---------------------------------------------------------------------------------------------
void Reset(StaticIoiStr* cfgRqst, StaticIoiStr* cfgRsp, StaticIoiStr* sts,
           StaticIoiStr* sf1, StaticIoiStr* sf2, StaticIoiStr* sf3, StaticIoiStr* sf4);
{
    for (int i = 0; i < eNUM_SUBFRAMES; ++i)
    {
        m_pSF[i]->Reset();
    }
}

//---------------------------------------------------------------------------------------------
void A717Qar::Initialize()
{
    if (!m_bInit)
    {
        // Init the table
        m_pSF[0] = &m_a717QarSf1;
        m_pSF[1] = &m_a717QarSf2;
        m_pSF[2] = &m_a717QarSf3;
        m_pSF[3] = &m_a717QarSf4;

        Reset();
        m_bInitSFOutput = TRUE;
        m_bInit = TRUE;
    }

}

//=============================================================================================
//=============================================================================================
//=============================================================================================
A717Subframe::A717Subframe(const CHAR* pIoiName, int sfIdx, int barker)
{
    m_pIoiName = pIoiName;
    m_mySFNum = sfIdx;
    m_mySfMask = (1 << (sfIdx - 1));
    m_stdBarker = barker;
    m_revBarker = ReverseBarker(barker);
    m_bDefaultBarker = true;
    m_ioiObj.pdata = m_qarWords;
    m_writeErrCnt = 0;
    Reset();
}

//---------------------------------------------------------------------------------------------
void A717Subframe::Reset()
{
    m_ioiValid = false;
    m_ioiStatusSf = ioiNoSuchItem;
}

//---------------------------------------------------------------------------------------------
void A717Subframe::InitIoi()
{
  // Open the output IOI for this subframes data
    ioiStatus openStatus;
    ioiStatus closeStatus;

    memset(m_qarWords, 0, sizeof(m_qarWords));

    if (m_ioiValid)
    {
        closeStatus = ioi_close(m_ioiObj.fd);
    }

    if (closeStatus == ioiSuccess)
    {
        m_ioiValid = FALSE;
        m_ioiStatusSf = closeStatus;
    }

    openStatus = ioi_open(m_pIoiName, ioiWritePermission, &m_ioiObj.fd);
    if (openStatus == ioiSuccess)
    {
        m_ioiValid = TRUE;
        m_ioiStatusSf = openStatus;
    }
}

//---------------------------------------------------------------------------------------------
// Signal the default barker code for this SF should be sent
void A717Subframe::ResetBarker()
{
    m_bDefaultBarker = true;
}

//---------------------------------------------------------------------------------------------
// publishing the buffer associated with this SF's IOI on its schedule

BOOLEAN A717Subframe::UpdateIoi()
{
    m_ioiStatusSf = ioiSuccess;

    // if we are valid and running, and not being run by a parameter
    if (m_ioiValid)
    {
        // If the tester indicated a custom barker code in the call to UpdateBuffer,
        // then m_bDefaultBarker would be false.
        // leave SF buffer[0] alone as a custom value for testing.

        // Note: m_ioiObj.pdata points to m_qarWords[]

        if (m_bDefaultBarker) // TBD: why are we doing this repeatedly - why not in reset
        {
            m_qarWords[0] = g_bReverse ? (UINT32)m_revBarker : (UINT32)m_stdBarker;
        }
        else
        {
            // The  m_qarWords[0] has a user defined value for test. Leave it alone
        }

        m_ioiStatusSf = ioi_write(m_ioiObj.fd, m_ioiObj.pdata);
        m_ioiValid = (m_ioiStatusSf == ioiSuccess);
    }

    return m_ioiStatusSf == ioiSuccess;
}

//---------------------------------------------------------------------------------------------
// Process input received from the Script Execution Control
// This function is called by 
//
bool A717Subframe::UpdateBuffer(UINT32 offset, UINT8* pBuffer, UINT32 byteCnt)
{
    bool status = true;
    UINT32  wordCnt;    // Number of words (really 'slots') needed
    UINT32  wordOffset; // offset into the SF buffer to put the new data values
    UINT16* inPtr = (UINT16*)pBuffer;
    UINT32* destPtr;

    // If the tester has specified any value at offset zero, they are setting/clearing a
    // custom barker code. Don't use the default barker until the tester has called 
    // ResetQarBarker for this SF.
    if (0 == offset)
    {
        m_bDefaultBarker = false;
    }

    // move data from SEC into our objects SF data array

    // Make sure the data will fit in the SF buffer based on offset and byteCnt.
    // Limit the move to what is available from offset.
    wordCnt = (byteCnt / 2);
    wordOffset = (offset / 2);

    if (wordCnt > (g_frameSize - wordOffset))
    {
        wordCnt = (g_frameSize - wordOffset);
    }

    // input is an array of SINT16, dest is UINT32
    //inPtr   = (UINT16*)pBuffer;
    destPtr = (UINT32*)&m_qarWords + wordOffset;

    for (int i = 0; i < wordCnt; ++i)
    {
        *(destPtr++) = *(inPtr++);
    }

    return status;
}
