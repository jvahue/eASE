#ifndef ccdl_h
#define ccdl_h
/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File:        ccdl.h

    Description: This file implements the base class for simulating the Cross
    channel Data Link (CCDL)

    VERSION
    $Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "CrossChannel.h"
#include "EfastMgr.h"
#include "ReportDefs.h"

#include "MailBox.h"
#include "Parameter.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
class CCDL
{
public:
    enum CcdlState {
        eCcdlStateErr,
        eCcdlStateOk,
        eCcdlStateInit
    };

    enum CcdlModes {
        eCcdlStart,     // Startup mode
        eCcdlStartTx,   // Start Channel A Tx Setup
        eCcdlStartRx,   // Wait Channel B Rx Setup from A
        eCcdlRun,       // We are running and sending only param data
        eCcdlRunHist,   // we are running and sending param & flt trigger data
        eCcdlHold       // the ccdl is in hold mode no Tx
    };

    enum CcdlItems {
        eCcdlIdSource,
        eCcdlIdElapsed,
        eCcdlIdLclFileCrc,
        eCcdlIdCmbFileCrc,
        eCcdlIdLclXmlCrc,
        eCcdlIdAcidRx,
        eCcdlIdAcidOk,
        eCcdlIdMaxItem,
    };

    CCDL(AseCommon* pCommon);
    void Reset();

    void Update(MailBox& in, MailBox& out);
    void UpdateEfast();
    void  Write(CC_SLOT_ID id, void* buf, INT32 size);
    INT32 Read(CC_SLOT_ID id, void* buf, INT32 size);
    bool CcdlIsRunning() {return m_mode == eCcdlRun;}

    virtual BOOLEAN CheckCmd( SecComm& secComm);
    int PageCcdl(int theLine, bool& nextPage, MailBox& in, MailBox& out);

    void PackRequestParams(Parameter* parameters, UINT32 maxParamIndex);
    void SendParamRqst();
    void GetParamData();
    bool SendParamData();

    void Receive(MailBox& in);
    bool Transmit(MailBox& out);
    void ValidateRemoteSetup();


    AseCommon* m_pCommon;
    bool m_isValid;
    bool m_inhibit;
    EFAST_CH_ENUM m_actingChan;
    CcdlModes m_mode;
    UINT32 m_modeDelay;
    CcdlState m_rxState;
    CcdlState m_txState;

    // Out msg components
    UINT32 m_rqstParamIdMap[PARAM_XCH_BUFF_MAX]; // slot to param ID (index) map
    PARAM_XCH_BUFF m_rqstParamMap;               // what we request we will receive
    PARAM_XCH_BUFF m_txParamData;                // this is what we send to the ADRF at run time
    PARAM_XCH_BUFF m_txHistData;                 // this is where we send the history data from
    
    // In msg components
    PARAM_XCH_BUFF m_rxParamData;         // this is what we get in
    UINT32 m_ccdlRawParam[eAseMaxParams]; // a place to hold the data from the remote channel

    EFAST_CROSS_CH_DATA m_eFastIn;
    EFAST_CROSS_CH_DATA m_eFastOut;
    EFAST_CROSS_CH_DATA m_eFastHold;
    bool m_useCcdlItem[eCcdlIdMaxItem];

    BOOLEAN m_reportIn[MAX_ADRF_REPORT];
    BOOLEAN m_reportOut[MAX_ADRF_REPORT];

    UINT32 m_ccdlCalls;
    UINT32 m_rxCount;      // how many input msgs
    UINT32 m_txCount;      // how many output msg
    UINT32 m_rxParam;      // how many input params
    UINT32 m_txParam;      // how many output params
    UINT32 m_rxFailCount;  // how many missing/failed inputs
    UINT32 m_txFailCount;  // how many missing/failed outputs

    Parameter* m_parameters;
    UINT32 m_maxParamIndex;

    UINT32 m_wrCalls;
    UINT32 m_wrWrites[CC_MAX_SLOT];

    UINT32 m_rdCalls;
    UINT32 m_rdReads[CC_MAX_SLOT];

    // Flight Trigger History Vars
    UINT16 m_histPacketRx;  // how many history packets have we received
    UINT16 m_histPacketTx;  // how many history packets have we transmitted
};

#endif

