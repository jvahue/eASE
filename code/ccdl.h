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

typedef struct
{
    UINT32 slotId;
    UINT32 size;
    void* in;
    void* out;
} PackMap;

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
    enum CcdlConstants {
        eCcdl = 1, // change this value to take more rows at top
    };

    enum CcdlModes {
        eCcdlStart,
        eCcdlStartTx,
        eCcdlStartRx,
        eCcdlRun,
        eCcdlHold
    };

    CCDL(AseCommon* pCommon);
    void Reset() {m_mode = eCcdlStart;}

    void Update(MailBox& in, MailBox& out);
    void  Write(CC_SLOT_ID id, void* buf, INT32 size);
    INT32 Read(CC_SLOT_ID id, void* buf, INT32 size);
    bool CcdlIsRunning() {return m_mode == eCcdlRun;}

    virtual BOOLEAN CheckCmd( SecComm& secComm);
    int PageCcdl(int theLine, bool& nextPage, MailBox& in, MailBox& out);

    void PackRequestParams(Parameter* parameters, UINT32 maxParamIndex);
    void SendParamRqst();
    void GetParamData();

    void Receive(MailBox& in);
    void Transmit(MailBox& out);
    void ValidateRemoteSetup();

    PARAM_XCH_BUFF m_txParamData;  // this is what we send to the ADRF at run time

protected:
    AseCommon* m_pCommon;
    bool m_isValid;
    EFAST_CH_ENUM m_actingChan;
    CcdlModes m_mode;
    UINT32 m_modeDelay;
    bool m_setupErrorRx;
    bool m_setupErrorTx;

    // Out msg components
    PARAM_XCH_BUFF m_rqstParamMap;        // what we request we will receive
    PARAM_XCH_BUFF m_rxParamData;         // this is what we get in
    UINT32 m_ccdlRawParam[eAseMaxParams]; // a place to hold the data from the remote channel

    EFAST_CROSS_CH_DATA m_eFastIn;
    EFAST_CROSS_CH_DATA m_eFastOut;

    BOOLEAN m_reportIn[MAX_ADRF_REPORT];
    BOOLEAN m_reportOut[MAX_ADRF_REPORT];

    PackMap m_ccdlPackMap[CC_MAX_SLOT];

    UINT32 m_rxCount;      // how many input msgs
    UINT32 m_txCount;      // how many output msg
    UINT32 m_rxParam;      // how many input params
    UINT32 m_txParam;      // how many output params
    UINT32 m_rxFailCount;  // how many missing/failed inputs
    UINT32 m_txFailCount;  // how many missing/failed outputs

    Parameter* m_parameters;
    UINT32 m_maxParamIndex;
};

#endif

