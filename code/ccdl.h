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
// defined in EfastMgrUtil.c
typedef struct
{
    DATETIME_STRUCT dateTime; // Current Time in Base Time of local Ch
    EFAST_TIME_SRC srcTime;   // Current Time Source
    UINT32 sysElapsedTime;    // Local Ch elapsed time, to be used by remote ch
    //   to adjust dateTime to it's (remote ch) Base Time.
} EFAST_CROSS_CH_DATA;

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

    virtual BOOLEAN CheckCmd( SecComm& secComm);
    virtual int UpdateDisplay(VID_DEFS who, int theLine);

    void PackRequestParams(Parameter* parameters, UINT32 maxParamIndex);
    void SendParamRqst(MailBox& out);
    void GetParamRqst(MailBox& in);

    void Receive(MailBox& in);
    void Transmit(MailBox& out);
    void ValidateRemoteSetup();

    bool m_isValid;
    EFAST_CH_ENUM m_actingChan;
    PARAM_XCH_BUFF m_txSet;  // this is what we send to the ADRF at run time

protected:
    AseCommon* m_pCommon;
    UINT32 m_rxCount;
    UINT32 m_txCount;
    CcdlModes m_mode;
    UINT32 m_modeDelay;
    bool m_ccdlSetupError;

    // Out msg components
    PARAM_XCH_BUFF m_rxMap;  // what we request we will receive
    PARAM_XCH_BUFF m_remoteParam;  // this is what we get

    EFAST_CROSS_CH_DATA m_eFastIn;
    EFAST_CROSS_CH_DATA m_eFastOut;

    BOOLEAN m_reportIn[MAX_ADRF_REPORT];
    BOOLEAN m_reportOut[MAX_ADRF_REPORT];

    UINT32 m_ccdlRawParam[eAseMaxParams]; // a place to hold the data from the remote channel
                                          // so we can compare it to what we sent/expect

    PackMap m_packMap[CC_MAX_SLOT];

    UINT32 m_rxFailCount;
    UINT32 m_txFailCount;

    Parameter* m_parameters;
    UINT32 m_maxParamIndex;
};

#endif

