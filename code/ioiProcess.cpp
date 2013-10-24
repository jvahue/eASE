//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: ioiProcess.cpp
//
//    Description: The file implements the Parameter object processing
//
//-----------------------------------------------------------------------------
/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <mem.h>
#include <string.h>
#include <stdio.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseCommon.h"

#include "ioiProcess.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define CC_RX_MBOX_NAME_ASE     "ADRF_MBOX_TX"  // these look backwards on purpose
#define CC_TX_MBOX_NAME_ASE     "ADRF_MBOX_RX"

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

static const char* ioiInitStatus[] = {
//   !  max length    !
    "Ok",
    "CfgFileNotFound",
    "CfgFileFailure",
    "NoCfgInfoForProc",
    "InsufficientRAM",
    "FmttLibLoadFailed",
    "BffrLibLoadFailed",
    "SMONotCreated",
    "CannotAttachToSMO"
};

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
IoiProcess::IoiProcess()
    : m_paramCount(0)
    , m_maxParamIndex(0)
    , m_paramLoopEnd(0)
    , m_paramInfoCount(0)
    , m_displayCount(0)
    , m_page(0)
    , m_paramDetails(0)
    , m_scheduled(0)
    , m_updated(0)
    , m_initStatus(ioiNoSuchItem)  // this is not a valid return value for ioi_init
    , m_initParams(false)
    , m_ioiOpenFailCount(0)
    , m_ioiCloseFailCount(0)
    , m_ioiWriteFailCount(0)
    , m_sgRun(false)               // sig gen comes up on hold
    , m_paramIoRunning(true)       // IO comes up running
    , m_avgIoiTime(0)
    , m_totalParamTime(0)
    , m_totalIoiTime(0)
    , m_ccdl(&aseCommon)
{
    // clear out the paramInfo
    memset((void*)m_paramInfo, 0, sizeof(m_paramInfo));

    // clear out our display index
    memset((void*)m_displayIndex, 0, sizeof(m_displayIndex));

    memset((void*)m_openFailNames, 0, sizeof(m_openFailNames));
    memset((void*)m_closeFailNames, 0, sizeof(m_closeFailNames));
}

/****************************************************************************
 public methods for IoIProcess
****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Function: Run
// Description: Prepare to run and then launch the process
//
void IoiProcess::Run()
{
    // Initialize IOI Before use
    m_initStatus = ioi_init("");

    // create alias for this process because some process needs to source the
    // ioi data
    processStatus ps = createProcessAlias( "IO");

    // create the CCDL mailboxes
    //--------------------------------------------------------------------------
    // Set up mailboxes for CCDL with ADRF
    m_ccdlIn.Create(CC_RX_MBOX_NAME_ASE, CC_MAX_SIZE, eMaxQueueDepth);
    m_ccdlIn.IssueGrant(adrfProcessName);

    // Connect to the the ADRF recv box in the ADRF.
    m_ccdlOut.Connect(adrfProcessName, CC_TX_MBOX_NAME_ASE);

    // create all of the IOI items
    InitIoi();

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("IoiProcess", "StdThreadTemplate");
}

//-------------------------------------------------------------------------------------------------
// Function: RunSimulation
// Description: Processing done to simulate the ioi process
//
void IoiProcess::RunSimulation()
{
    if (!m_pCommon->bScriptRunning)
    {
        m_sgRun = false;
    }

    UpdateIoi();

    // at 50 Hz pack the CCDL message and send it out
    if ((m_systemTick & 1) == 1)
    {
        m_ccdl.Update(m_ccdlIn, m_ccdlOut);
    }

}

//-------------------------------------------------------------------------------------------------
// Function: UpdateIoi
// Description: Update all of the "output" parameters from the ioi process
//
void IoiProcess::UpdateIoi()
{
    UINT32 i;
    UINT32 scheduleZ1 = 3001;
    UINT32 remoteX = 0;
    Parameter* param;

    m_scheduled = 0; // need to see how many are really scheduled
    m_updated = 0;   // toggle the lsb

    if ( m_initStatus == ioiSuccess)
    {
        m_totalParamTime = 0;
        m_totalIoiTime = 0;

        param = &m_parameters[0];
        for (i=0; i < m_paramLoopEnd && !m_initParams; ++i)
        {
            if (param->m_isValid && !param->m_isChild)
            {
                m_scheduled += param->Update( GET_SYSTEM_TICK, m_sgRun);
                m_totalParamTime += param->m_updateDuration;

                if (m_paramIoRunning && m_scheduled != scheduleZ1 && param->IsRunning())
                {
                    if (param->m_src != PARAM_SRC_CROSS)
                    {
                        WriteIoi(param);
                    }
                    else
                    {
                        // move data to xchan ioi slot
                        m_ccdl.m_txParamData.data[remoteX].id = param->m_ccdlId;
                        m_ccdl.m_txParamData.data[remoteX].val = param->m_ioiValue;
                        remoteX += 1;
                        m_ccdl.m_txParamData.num_params = remoteX;
                    }
                }
                scheduleZ1 = m_scheduled;
            }
            ++param;
        }
    }

    if (m_updated > 0)
    {
           m_avgIoiTime /= m_updated;
    }
    else
    {
        m_avgIoiTime = 0;
    }

    // complete the contents of the ccdl param data
    m_ccdl.m_txParamData.type = PARAM_XCH_TYPE_DATA;
    if ( m_ccdl.CcdlIsRunning())
    {
        m_ccdl.Write(CC_PARAM, &m_ccdl.m_txParamData, sizeof(m_ccdl.m_txParamData));
    }
}

//-------------------------------------------------------------------------------------------------
// Function: WriteIoi
// Description: Write param raw data value to it's ioi thingy
//
void IoiProcess::WriteIoi(Parameter* param )
{
    UINT32 start;
    ioiStatus writeStatus;

    if (!m_initParams)
    {
        start = HsTimer();
        writeStatus = ioi_write(param->m_ioiChan, &param->m_ioiValue);
        m_totalIoiTime += HsTimeDiff(start);

        if (writeStatus == ioiSuccess)
        {
            m_updated += 1; // count how many we updated
        }
        else
        {
            // TODO : what else, anything?
            m_ioiWriteFailCount += 1;
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateCCDL
// Description: Update the CCDL and send
//
void IoiProcess::UpdateCCDL()
{

}

//-------------------------------------------------------------------------------------------------
// Function: HandlePowerOff
// Description: Processing done to simulate the ioi process
//
void IoiProcess::HandlePowerOff()
{
    m_ccdlIn.Reset();
    m_ccdlOut.Reset();
    m_ccdl.Reset();
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateDisplay
// Description: Manage the display of the IOI Status page and the Param data page
//-----------------------------------------------------------------------------
int IoiProcess::UpdateDisplay(VID_DEFS who, int theLine)
{
    bool nextPage = false;

    if (m_page == 0)
    {
        theLine = PageIoiStatus(theLine, nextPage);
    }
    else if (m_page == 1)
    {
        theLine = PageParams(theLine, nextPage);
    }

    if (nextPage)
    {
        m_page = (m_page + 1) % eMaxPages;
        theLine = 0;
    }

    return theLine;
}

int IoiProcess::PageIoiStatus(int theLine, bool& nextPage)
{
    if (theLine == 0)
    {
        CmdRspThread::UpdateDisplay(Ioi, theLine);
    }
    else if (theLine == 1)
    {
        debug_str(Ioi, theLine, 0,
                  "IOI(%s) SigGen(%s) Params(%4d) ParamInfo(%4d) Sched(%4d) Updated(%4d)",
                  ioiInitStatus[m_initStatus],
                  m_sgRun ? " On" : "Off",
                  m_paramCount,
                  m_paramInfoCount,
                  m_scheduled, m_updated);
    }
    else if (theLine == 2)
    {
        debug_str(Ioi, theLine, 0,
                  "oErr(%d) wErr(%d) cErr(%d) TotP(%d) TotI(%d) AvgIoi(%d)",
                  m_ioiOpenFailCount,
                  m_ioiWriteFailCount,
                  m_ioiCloseFailCount,
                  m_totalParamTime,
                  m_totalIoiTime,
                  m_avgIoiTime);
    }
    else
    {
        int tgtLine = theLine - 3;

        if (tgtLine < (int)eIoiFailDisplay)
        {
            debug_str(Ioi, theLine, 0, "Open %-32s - Close %-32s",
                      m_openFailNames[tgtLine], m_closeFailNames[tgtLine]);
        }
        else
        {
            if (tgtLine == (int)eIoiFailDisplay)
            {
                nextPage = true;
            }
            theLine = m_ccdl.PageCcdl(theLine, nextPage, m_ccdlIn, m_ccdlOut);
        }
    }

    theLine += 1;

    return theLine;
}

//-------------------------------------------------------------------------------------------------
// Function: PageParams
// Description: Manage the display of the parameter values and info
//-----------------------------------------------------------------------------
int IoiProcess::PageParams(int theLine, bool& nextPage)
{
#define DETAIL_ROWS 2 // really three but ...
    UINT32 rowIndex;
    UINT32 baseIndex;
    Parameter* p1;
    Parameter* p2;
    char buf1[80];
    char buf2[80];
    UINT32 infoStarts = (eIoiMaxDisplay/2) + 2;
    static UINT32 newInfo = 0;

    switch (theLine) {
    case 0:
        CmdRspThread::UpdateDisplay(Params, 0);

        // every 2s display info on another 2 params
        if (newInfo++ == 10)
        {
            m_paramDetails = (m_paramDetails + 1) % (eIoiMaxDisplay/2);
            newInfo = 0;
        }

        break;

    case 1:
        debug_str(Params, theLine, 0, "SigGen(%s)", m_sgRun ? " On" : "Off");
        break;

    default:
        if (theLine >= 2 && theLine <= (infoStarts-1))
        {
            baseIndex = (theLine - 2) * 2; // 2:0, 3:2, 4:4, 5:6 ... 21:38
            p1 = &m_parameters[m_displayIndex[baseIndex]];
            p2 = &m_parameters[m_displayIndex[baseIndex+1]];
            if (m_displayIndex[baseIndex] != eAseMaxParams && m_displayIndex[baseIndex+1] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%s %s", p1->Display(buf1), p2->Display(buf2));
            }
            else if (m_displayIndex[baseIndex] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%s", p1->Display(buf1));
            }
            else if (m_displayIndex[baseIndex+1] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%39s %s", " ", p2->Display(buf2));
            }
            else
            {
                // jump to the display info - below will increment this
                theLine = infoStarts - 1;
            }
        }

        // display details for params
        else if (theLine == infoStarts || theLine <= (infoStarts+DETAIL_ROWS))
        {
            rowIndex = m_paramDetails * 2;

            // pick params to display
            p1 = &m_parameters[m_displayIndex[rowIndex]];
            p2 = &m_parameters[m_displayIndex[rowIndex+1]];

            if (m_displayIndex[rowIndex] != eAseMaxParams && m_displayIndex[rowIndex+1] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%-39s %s",
                          p1->ParamInfo(buf1, theLine-infoStarts),
                          p2->ParamInfo(buf2, theLine-infoStarts));
            }
            else if (m_displayIndex[rowIndex] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%s", p1->ParamInfo(buf1, theLine-infoStarts));
            }
            else if (m_displayIndex[rowIndex+1] != eAseMaxParams)
            {
                debug_str(Params, theLine, 0, "%39s %s", " ", p2->ParamInfo(buf2, theLine-infoStarts));
            }
            else
            {
                m_paramDetails = 0;
            }

            if (theLine == (infoStarts+DETAIL_ROWS))
            {
                nextPage = true;
            }
        }
        break;
    }

    theLine += 1;

    return theLine;
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Update all of the "output" parameters from the ioi process
//
BOOLEAN IoiProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = TRUE;
    ResponseType rType = eRspNormal;
    //int port;  // 0 = gse, 1 = ms
    int itemId;

    SecRequest request = secComm.m_request;
    itemId = request.variableId;

    switch (request.cmdId)
    {
    case eGetSensorNames:
        FillSensorNames(itemId, secComm.m_snsNames);
        secComm.m_response.successful = TRUE;
        serviced = TRUE;
        rType = eRspSensors;
        break;

    case eGetSensorValue:
        if (m_parameters[itemId].m_isValid)
        {
            secComm.m_response.value = m_parameters[itemId].m_value;
            secComm.m_response.successful = TRUE;
            serviced = TRUE;
        }
        else
        {
            secComm.ErrorMsg("Ioi: Invalid Param Index <%d>", itemId);
            secComm.m_response.successful = FALSE;
            serviced = TRUE;
        }
        break;

    case eSetSensorValue:
        // TBD: This option should be removed as ePySte will convert a call to SetSensor('x', 33.3)
        //      into eSetSensorSG with the type set to manual
        break;

    case eSetSensorSG:
        if ( m_parameters[itemId].m_isValid)
        {
            UINT32 sgType = request.sigGenId;
            if ( ARRAY( sgType, eMaxSensorMode))
            {
                bool setResult;
                Parameter& theParam = m_parameters[itemId];

                setResult = theParam.m_sigGen.SetParams(sgType, theParam.m_updateMs,
                                            request.param1, request.param2,
                                            request.param3, request.param4);

                if (sgType == eSGmanual)
                {
                    theParam.m_value = request.param1;
                }

                // Init the value
                theParam.m_value =  theParam.m_sigGen.Reset(theParam.m_value);
                secComm.m_response.successful = setResult;
                if (!setResult)
                {
                    secComm.ErrorMsg("Unable to complete parameter setup");
                }
            }
            else
            {
                secComm.ErrorMsg("Ioi: Invalid SG Type <%d>", sgType);
                secComm.m_response.successful = FALSE;
            }
        }
        else
        {
            secComm.ErrorMsg("Ioi: Invalid Param Index <%d>", itemId);
            secComm.m_response.successful = FALSE;
        }
        serviced = TRUE;
        break;

    case eResetSG:
        if ( request.resetAll)
        {
            for (UINT32 i = 0; i < eAseMaxParams; ++i)
            {
                if (m_parameters[i].m_isValid)
                {
                    m_parameters[i].m_sigGen.Reset(m_parameters[i].m_value);
                }
            }
            secComm.m_response.successful = TRUE;
        }
        else
        {
            // if the parameter index is valid reset it
            if ( m_parameters[itemId].m_isValid)
            {
                m_parameters[itemId].m_sigGen.Reset(m_parameters[itemId].m_value);
                secComm.m_response.successful = TRUE;
            }
            else
            {
                secComm.ErrorMsg("Ioi: Invalid Param Index <%d>", itemId);
                secComm.m_response.successful = FALSE;
            }
        }

        serviced = TRUE;
        break;

    case eRunSG:
        if (!m_sgRun)
        {
            m_sgRun = true;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            secComm.ErrorMsg( "Run SG Error: SGs are Running");
            secComm.m_response.successful = FALSE;
        }
        serviced = TRUE;
        break;

    case eHoldSG:
        if (m_sgRun)
        {
            m_sgRun = false;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            secComm.ErrorMsg("Hold SG Error: SGs are Holding");
            secComm.m_response.successful = FALSE;
        }
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eSetSdi:
        {
            // [msb                 lsb]
            // [Spare | Bus | Lbl | Vlu] <= bytes in itemId
            int bus = (itemId >> 16) & 0xff;
            int lbl = (itemId >> 8 ) & 0xff;
            int vlu = (itemId >> 0 ) & 0xff;
            bool found = false;
            Parameter* param = &m_parameters[0];

            // scan sensors and see if we can find the signal and any children
            for ( int i = 0; i < m_paramLoopEnd; ++i)
            {
                if (param->m_isValid && param->m_type == PARAM_FMT_A429)
                {
                    if (param->m_a429.channel == bus && param->m_a429.label == lbl)
                    {
                        param->SetSdi( vlu);
                        // don't break here in case there are children that need to be set
                        found = true;
                    }
                }
                ++param;
            }

            if (!found)
            {
                secComm.m_response.successful = false;
                secComm.ErrorMsg( "SetSdi: Invalid Signal Bus(%d), Label(%d)", bus, lbl);
            }

        }
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eSetSsm:
        {
            // [msb                 lsb]
            // [Spare | Bus | Lbl | Vlu] <= bytes in itemId
            int bus = (itemId >> 16) & 0xff;
            int lbl = (itemId >> 8 ) & 0xff;
            int vlu = (itemId >> 0 ) & 0xff;
            bool found = false;
            Parameter* param = &m_parameters[0];

            // scan sensors and see if we can find the signal and any children
            for ( UINT32 i = 0; i < m_paramLoopEnd; ++i)
            {
                if (param->m_isValid && param->m_type == PARAM_FMT_A429)
                {
                    if (param->m_a429.channel == bus && param->m_a429.label == lbl)
                    {
                        param->SetSsm( vlu);
                        // don't break here in case there are children that need to be set
                        found = true;
                    }
                }
                ++param;
            }

            if (!found)
            {
                secComm.m_response.successful = false;
                secComm.ErrorMsg("SetSsm: Invalid Signal Bus(%d), Label(%d)", bus, lbl);
            }

        }
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eSetLabel:
    {
        // [msb                 lsb]
        // [Spare | Bus | Lb0 | lb1] <= bytes in itemId
        int bus = (itemId >> 16) & 0xff;
        int lb0 = (itemId >> 8 ) & 0xff;
        int lb1 = (itemId >> 0 ) & 0xff;
        bool found = false;
        Parameter* param = &m_parameters[0];

        // scan sensors and see if we can find the signal and any children
        for ( UINT32 i = 0; i < m_paramLoopEnd; ++i)
        {
            if (param->m_isValid && param->m_type == PARAM_FMT_A429)
            {
                if (param->m_a429.channel == bus && param->m_a429.label == lb0)
                {
                    param->SetLabel( lb1);
                    // don't break here in case there are children that need to be set
                    found = true;
                }
            }
            ++param;
        }

        if (!found)
        {
            secComm.m_response.successful = false;
            secComm.ErrorMsg("Invalid Signal Bus(%d), Label(%d)", bus, lb0);
        }

    }
    serviced = TRUE;
    break;

    //----------------------------------------------------------------------------------------------
    case eSendParamData:
        if (CollectParamInfo(request.variableId, request.sigGenId, request.charData))
        {
            secComm.m_response.successful = true;
        }
        else
        {
            secComm.ErrorMsg("Parameter Count exceeded: %d ", request.sigGenId);
            secComm.m_response.successful = false;
        }

        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eInitParamData:
        InitIoi();
        secComm.m_response.successful = true;
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eDisplayParam:
        if (request.sigGenId < eIoiMaxDisplay)
        {
            if (m_parameters[request.variableId].m_isValid)
            {
                // always pack them in at the top of the display
                if (request.sigGenId >= m_displayCount && m_displayCount < eIoiMaxDisplay)
                {
                    request.sigGenId = m_displayCount;
                    m_displayCount += 1;
                }
                m_displayIndex[request.sigGenId] = request.variableId;
            }
            secComm.m_response.successful = true;
        }
        else
        {
            secComm.ErrorMsg("Slot Index %d exceeds %d, check maxSlot in DisplayParam -> SEC.py",
                             request.sigGenId, eIoiMaxDisplay);
            secComm.m_response.successful = false;
        }
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eResetIoi:
        // clear the paramInfo array
        memset((void*)m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;
        InitIoi();
        secComm.m_response.successful = true;

        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eDisplayState:
        if (request.variableId == (int)Ioi || request.variableId == (int)Params)
        {
            m_updateDisplay = request.sigGenId != 0;
            secComm.m_response.successful = true;
            // serviced: goes inside here so other Displays get a chance to see the cmd
            serviced = TRUE;
        }
        break;

    //----------------------------------------------------------------------------------------------
    case eParamState:
        if (m_parameters[request.variableId].m_isValid)
        {
            m_parameters[request.variableId].m_isRunning = request.sigGenId != 0;
            secComm.m_response.successful = true;
        }
        else
        {
            secComm.ErrorMsg("Parameter[%d] is not valid", request.variableId);
        }
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eParamIoState:
        m_paramIoRunning = m_parameters[request.variableId].m_isValid != 0;
        secComm.m_response.successful = true;
        serviced = TRUE;
        break;

    default:
        // we did not service this command
        serviced = FALSE;
        break;
    }

    if (serviced)
    {
        secComm.SetHandler("ioiProc");
        secComm.IncCmdServiced(rType);
    }

    return serviced;
}

//-------------------------------------------------------------------------------------------------
// Function: FillSensorNames
// Description: Send the parameter names up to ePySte
// Start at the location defined from PySte t0=0, t1..n=pBaseIndex(n-1)+125
// Send up to 125 at a time
// - loop until we find a valid one starting from 'start' the do 125 consecutive
//
void IoiProcess::FillSensorNames(INT32 start, SensorNames& snsNames) const
{
    UINT32 i;
    UINT32 filledIn = 0;

    snsNames.maxIndex = m_maxParamIndex;

    // clear all the names
    for (i = 0; i < eSecNumberOfSensors; ++i)
    {
        snsNames.names[i][0] = '\0';
    }

    snsNames.pBaseIndex = (UINT32)eAseMaxParams;
    for (i = start; (filledIn < (UINT32)eSecNumberOfSensors && i < eAseMaxParams); ++i)
    {
        if (snsNames.pBaseIndex != (UINT32)eAseMaxParams || m_parameters[i].m_isValid)
        {
            if (snsNames.pBaseIndex == (UINT32)eAseMaxParams)
            {
                snsNames.pBaseIndex = i;
            }
            strncpy(snsNames.names[filledIn], m_parameters[i].m_name, (int)eAseParamNameSize);
            filledIn += 1;
        }
    }
}

/****************************************************************************
 protected methods for IoIProcess
****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Function: CollectParamInfo
// Description: Receive the parameter info from ePySte for a Cfg to be loaded
//
bool IoiProcess::CollectParamInfo(int paramSetCount, UINT32 paramCount, char* data)
{
#define PARAM_SET_SIZE (eSecCharDataSize/sizeof(m_paramInfo))
    bool status = false;
    UINT32 i;
    ParamCfg* pCfg = (ParamCfg*)data;

    if (paramCount <= PARAM_SET_SIZE)
    {
        // this is the first set ePySte is sending down - clear out the paramInfo
        memset((void*)m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;
    }

    if ( ((UINT32)paramSetCount + m_paramInfoCount) <= (int)eAseMaxParams)
    {
        for (i=0; i < (UINT32)paramSetCount; ++i)
        {
            m_paramInfo[i+m_paramInfoCount].index    = pCfg->index;
            m_paramInfo[i+m_paramInfoCount].masterId = pCfg->masterId;
            strncpy(m_paramInfo[i+m_paramInfoCount].name, pCfg->name, (int)eAseParamNameSize);
            m_paramInfo[i+m_paramInfoCount].rateHz = pCfg->rateHz;
            m_paramInfo[i+m_paramInfoCount].src = pCfg->src;
            m_paramInfo[i+m_paramInfoCount].fmt = pCfg->fmt;
            m_paramInfo[i+m_paramInfoCount].gpa = pCfg->gpa;
            m_paramInfo[i+m_paramInfoCount].gpb = pCfg->gpb;
            m_paramInfo[i+m_paramInfoCount].gpc = pCfg->gpc;
            m_paramInfo[i+m_paramInfoCount].scale = pCfg->scale;
            ++pCfg;
        }

        m_paramInfoCount += (UINT32)paramSetCount;
        status = true;
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
// Function: InitIoi
// Description: Processing done to simulate the ioi process
//
// Note: this function checks for child relationships between parameters.  Each parameter has a
// child link, that can be followed from the first (parent) param.
void IoiProcess::InitIoi()
{
    bool childRelationship;
    UINT32 i;
    UINT32 i1;
    ioiStatus openStatus;
    ioiStatus closeStatus;

    if (m_initStatus == ioiSuccess)
    {
        m_initParams = true;

        // clear the display
        m_displayCount = 0;
        for (i=0; i < eIoiMaxDisplay; ++i)
        {
            m_displayIndex[i] = eAseMaxParams;
        }

        m_ioiOpenFailCount = 0;
        memset((void*)m_openFailNames, 0, sizeof(m_openFailNames));

        m_ioiCloseFailCount = 0;
        memset((void*)m_closeFailNames, 0, sizeof(m_closeFailNames));

        // close any open ioi
        for (i=0; i < (int)eAseMaxParams; ++i)
        {
            if (m_parameters[i].m_ioiValid)
            {
                closeStatus = ioi_close(m_parameters[i].m_ioiChan);
                if (closeStatus != ioiSuccess)
                {
                    // DEBUG: copy name for display
                    if (m_ioiCloseFailCount < (int)eIoiFailDisplay)
                    {
                        strncpy(m_closeFailNames[m_ioiCloseFailCount],
                                m_parameters[i].m_name, (int)eAseParamNameSize);
                    }
                    m_ioiCloseFailCount += 1;
                }
            }

            // reset all parameters
            m_parameters[i].Reset();
        }

        m_paramCount = 0;
        m_maxParamIndex = 0;
        m_paramLoopEnd = 1;   // this must always be 1 more than m_maxParamIndex

        for (i=0; i < m_paramInfoCount; ++i)
        {
            UINT32 index = m_paramInfo[i].index;

            m_paramCount += 1;
            if (index > m_maxParamIndex)
            {
                m_maxParamIndex = index;
                m_paramLoopEnd = m_maxParamIndex + 1;
            }

            m_parameters[index].Init(&m_paramInfo[i]);

            // check to see if this parameter is a child of any existing parameters
            for (i1=0; i1 < m_paramLoopEnd; ++i1)
            {
                // if the param is valid and its not me
                if (m_parameters[i1].m_isValid && !m_parameters[i1].m_isChild && i1 != index)
                {
                    childRelationship = m_parameters[index].IsChild(m_parameters[i1]);
                    if (childRelationship)
                    {
                        break;
                    }
                }
            }

            // if not a child or src'd from ccdl - open the IOI channel
            if (!childRelationship && m_parameters[index].m_src != PARAM_SRC_CROSS)
            {
                openStatus = ioi_open(m_parameters[index].m_ioiName,
                                      ioiWritePermission,
                                      (int*)&m_parameters[index].m_ioiChan);

                if (openStatus == ioiSuccess)
                {
                    m_parameters[index].m_ioiValid = true;
                }
                else
                {
                    // copy name for display
                    if (m_ioiOpenFailCount < (int)eIoiFailDisplay)
                    {
                        strncpy(m_openFailNames[m_ioiOpenFailCount],
                                m_parameters[index].m_name, (int)eAseParamNameSize);
                    }
                    m_ioiOpenFailCount += 1;
                    m_parameters[index].m_isValid = false;
                }
            }

            // default the display to the first eIoiMaxDisplay parameters created
            if (m_displayCount < (int)eIoiMaxDisplay && m_parameters[index].m_isValid)
            {
                m_displayIndex[m_displayCount] = index;
                m_displayCount += 1;
            }
        }

        // clear out the paramInfo
        memset((void*)m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;

        // prep the CCDL request buffer
        m_ccdl.PackRequestParams(m_parameters, m_paramLoopEnd);

        ScheduleParameters();
        m_initParams = false;
    }
}

//-------------------------------------------------------------------------------------------------
// Function: ScheduleParameters
// Description: Compute the offsets for the parameters to balance the frame load - the algorithm
// simply puts the next param at a specific rate in the next frame for that rate
//
void IoiProcess::ScheduleParameters()
{
    UINT32 _01HZ = 0;
    UINT32 _02HZ = 0;
    UINT32 _04HZ = 0;
    UINT32 _05HZ = 0;
    UINT32 _10HZ = 0;
    UINT32 _20HZ = 0;
    UINT32 _25HZ = 0;
    UINT32 _50HZ = 0;
    UINT32 _XXHZ = 0;

    for (UINT32 i = 0; i < (UINT32)eAseMaxParams; ++i)
    {
        if (m_parameters[i].m_isValid)
        {
            switch (m_parameters[i].m_rateHz)
            {
            case 1:
                m_parameters[i].m_nextUpdate = _01HZ;
                _01HZ = (_01HZ + 1) % 100;
                break;
            case 2:
                m_parameters[i].m_nextUpdate = _02HZ;
                _02HZ = (_02HZ + 1) % 100;
                break;
            case 4:
                m_parameters[i].m_nextUpdate = _04HZ;
                _04HZ = (_04HZ + 1) % 100;
                break;
            case 5:
                m_parameters[i].m_nextUpdate = _05HZ;
                _05HZ = (_05HZ + 1) % 100;
                break;
            case 10:
                m_parameters[i].m_nextUpdate = _10HZ;
                _10HZ = (_10HZ + 1) % 100;
                break;
            case 20:
                m_parameters[i].m_nextUpdate = _20HZ;
                _20HZ = (_20HZ + 1) % 100;
                break;
            case 25:
                m_parameters[i].m_nextUpdate = _25HZ;
                _25HZ = (_25HZ + 1) % 100;
                break;
            case 50:
                m_parameters[i].m_nextUpdate = _50HZ;
                _50HZ = (_50HZ + 1) % 100;
                break;
            default:
                m_parameters[i].m_nextUpdate = _XXHZ;
                _XXHZ = (_XXHZ + 1) % 100;
                break;
            }
        }
    }
}

