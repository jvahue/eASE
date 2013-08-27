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
#include "ioiProcess.h"
#include "video.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define PARAM_CFG "param.cfg"

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
const char* ioiInitStatus[] = {
//   !  max length    !
    "Success",
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
    , m_frames(0)
    , m_scheduled(0)
    , m_updated(0)
    , m_ioiOpenFailCount(0)
    , m_ioiWriteFailCount(0)
    , m_sgRun(false)
{
    // clear out the paramInfo
    memset((void*)&m_paramInfo, 0, sizeof(m_paramInfo));

    // clear out our display index
    memset((void*)&m_displayIndex, 0, sizeof(m_displayIndex));
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
    // TBD: may want this to act as io cross-channel also

    //m_paramCfg.Open(PARAM_CFG, File::ePartCmProc, 'r');
    //m_paramCfg.Read(m_paramInfo, sizeof(m_paramInfo));
    //m_paramCfg.Close();

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
    if (m_systemTick & 1 == 1)
    {
        UpdateCCDL();
    }

}

//-------------------------------------------------------------------------------------------------
// Function: UpdateIoi
// Description: Update all of the "output" parameters from the ioi process
//
void IoiProcess::UpdateIoi()
{
    UINT32 i;
    Parameter* param;
    ioiStatus writeStatus;


    m_scheduled = 0; // need to see how many are really scheduled
    m_updated = 0;   // toggle the lsb

    if ( m_initStatus == ioiSuccess)
    {
        param = &m_parameters[0];
        for (i=0; i < m_paramLoopEnd; ++i)
        {
            if (param->m_isValid && !param->m_isChild && param->m_ioiValid)
            {
                m_scheduled += param->Update( GET_SYSTEM_TICK, m_sgRun) ? 1 : 0;
                writeStatus = ioi_write(param->m_ioiChan, &param->m_ioiValue);
                if (writeStatus != ioiSuccess)
                {
                    // TODO : what else?
                    m_ioiWriteFailCount += 1;
                }
                else
                {
                    m_updated += 1; // count how many we updated
                }
            }
            ++param;
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
    // TODO: reset mailboxes and IOI

}

//-------------------------------------------------------------------------------------------------
// Function: UpdateDisplay
// Description: Update the video output display
//
// Video Display Layout
// 0:
// 1: Frame: xxxxxx Updated: xxxx
// 2:
// 3: <Param1>: <floatValue> - <rawValue>
// 4: <--- parameter info data>
// 5: <Param3>: <floatValue> - <rawValue>
// 6: <--- parameter info data>
// ...
// n: Scheduled: <x> Updated: <y>
//-----------------------------------------------------------------------------
void IoiProcess::UpdateDisplay(VID_DEFS who)
{
    char rep[80];
    char outputLine[80];
    UINT32 i;

    UINT32 atLine = eFirstDisplayRow;

    CmdRspThread::UpdateDisplay(Ioi);

    //debug_str(Ioi, atLine, 0,"%s", m_blankLine);
    debug_str(Ioi, atLine, 0, "IOI: %s SigGen: %s Params: %d ParamInfo: %d",
              ioiInitStatus[m_initStatus],
              m_sgRun ? "Run" : "Hold",
              m_paramCount,
              m_paramInfoCount);
    atLine += 1;

    //debug_str(Ioi, atLine, 0, "%s", m_blankLine);
    debug_str(Ioi, atLine, 0, "Failed Setup %d Write Fail %d",
              m_ioiOpenFailCount,
              m_ioiWriteFailCount);
    atLine += 1;

    // display parameter data
    for (i=0; i < m_displayCount; ++i)
    {
        UINT32 x = m_displayIndex[i];

        //debug_str(Ioi, atLine, 0,"%s", m_blankLine);
        debug_str(Ioi, atLine, 0, "%32s(%6d): %11.4f - 0x%08x(0x%08x)",
                  m_parameters[x].m_name, m_parameters[x].m_updateCount,
                  m_parameters[x].m_value,
                  m_parameters[x].m_rawValue, m_parameters[x].m_data);
        atLine += 1;

        //debug_str(Ioi, atLine, 0,"%s", m_blankLine);
        debug_str(Ioi, atLine, 0, "%s", m_parameters[x].Display(rep));
        atLine += 1;
    }

    //debug_str(Ioi, atLine, 0,"%s", m_blankLine);
    debug_str(Ioi, atLine, 0, "Scheduled: %4d Updated: %4d", m_scheduled, m_updated);
}

//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Update all of the "output" parameters from the ioi process
//
BOOLEAN IoiProcess::CheckCmd( SecComm& secComm)
{
    BOOLEAN serviced = TRUE;
    ResponseType rType = eRspNormal;
    int port;  // 0 = gse, 1 = ms
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
        if (ARRAY( itemId, m_paramLoopEnd) && m_parameters[itemId].m_isValid)
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
        if ( ARRAY( itemId, m_paramLoopEnd) && m_parameters[itemId].m_isValid)
        {
            UINT32 sgType = request.sigGenId;
            if ( ARRAY( sgType, eMaxSensorMode))
            {
                Parameter& theParam = m_parameters[itemId];

                theParam.m_sigGen.SetParams(sgType, theParam.m_updateMs,
                                            request.param1, request.param2,
                                            request.param3, request.param4);
                if (sgType == eSGmanual)
                {
                    theParam.m_value = request.param1;
                }

                // Init the value
                theParam.m_value =  theParam.m_sigGen.Reset(theParam.m_value);
                secComm.m_response.successful = TRUE;
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
            if ( ARRAY( itemId, m_maxParamIndex) && m_parameters[itemId].m_isValid)
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
            for ( int i = 0; i < m_paramLoopEnd; ++i)
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
        for ( int i = 0; i < m_paramLoopEnd; ++i)
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
        if (m_parameters[request.variableId].m_isValid)
        {
            m_displayIndex[request.sigGenId] = request.variableId;
            m_displayCount = eIoiMaxDisplay;
        }
        secComm.m_response.successful = true;
        serviced = TRUE;
        break;

    //----------------------------------------------------------------------------------------------
    case eResetIoi:
        // this is the first set ePySte is sending down - clear out the paramInfo
        memset((void*)&m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;
        InitIoi();
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
//
void IoiProcess::FillSensorNames(INT32 start, SensorNames& snsNames)       // Send the sensor names to ePySte
{
    UINT32 i;
    UINT32 filledIn = 0;

    snsNames.maxIndex = m_maxParamIndex;

    // clear all the names
    for (i = 0; i < eSecNumberOfSensors; ++i)
    {
        snsNames.names[i][0] = '\0';
    }

    snsNames.pBaseIndex = eAseMaxParams;
    for (i = start; (filledIn < eSecNumberOfSensors && i < eAseMaxParams); ++i)
    {
        if (snsNames.pBaseIndex != eAseMaxParams || m_parameters[i].m_isValid)
        {
            if (snsNames.pBaseIndex == eAseMaxParams)
            {
                snsNames.pBaseIndex = i;
            }
            strncpy(snsNames.names[filledIn], m_parameters[i].m_name, eAseParamNameSize);
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
        memset((void*)&m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;
    }

    if ( (paramSetCount + m_paramInfoCount) <= eAseMaxParams)
    {
        for (i=0; i < paramSetCount; ++i)
        {
            m_paramInfo[i+m_paramInfoCount].index    = pCfg->index;
            m_paramInfo[i+m_paramInfoCount].masterId = pCfg->masterId;
            strncpy(m_paramInfo[i+m_paramInfoCount].name, pCfg->name, eAseParamNameSize);
            m_paramInfo[i+m_paramInfoCount].rateHz = pCfg->rateHz;
            m_paramInfo[i+m_paramInfoCount].fmt = pCfg->fmt;
            m_paramInfo[i+m_paramInfoCount].gpa = pCfg->gpa;
            m_paramInfo[i+m_paramInfoCount].gpb = pCfg->gpb;
            m_paramInfo[i+m_paramInfoCount].gpc = pCfg->gpc;
            m_paramInfo[i+m_paramInfoCount].scale = pCfg->scale;
            ++pCfg;
        }

        m_paramInfoCount += paramSetCount;
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
    UINT32 i;
    UINT32 i1;
    ioiStatus m_initStatus;
    ioiStatus openStatus;
    ioiStatus closeStatus;

    if (m_initStatus == ioiSuccess)
    {
        // close any open ioi
        for (i=0; i < eAseMaxParams; ++i)
        {
            if (m_parameters[i].m_ioiValid)
            {
                closeStatus = ioi_close(m_parameters[i].m_ioiChan);
                if (closeStatus == ioiSuccess)
                {
                    m_parameters[i].m_ioiValid = false;
                    m_parameters[i].m_ioiChan = 0;
                }
            }
        }

        m_paramCount = 0;
        m_maxParamIndex = 0;
        for (i=0; i < m_paramInfoCount; ++i)
        {
            UINT32 index = m_paramInfo[i].index;

            m_paramCount += 1;
            if (index > m_maxParamIndex)
            {
                m_maxParamIndex = index;
                m_paramLoopEnd = m_maxParamIndex + 1;
            }

            m_parameters[index].Reset(m_paramInfo[i].name, m_paramInfo[i].masterId, m_paramInfo[i].rateHz,
                                      m_paramInfo[i].fmt,
                                      m_paramInfo[i].gpa,  m_paramInfo[i].gpb, m_paramInfo[i].gpc,
                                      m_paramInfo[i].scale);

            // check to see if this parameter is a child of any existing parameters
            for (i1=0; i1 < m_paramLoopEnd; ++i1)
            {
                // if the param is valid and its not me
                if (m_parameters[i1].m_isValid && i1 != index)
                {
                    if (m_parameters[index].IsChild(m_parameters[i1]))
                    {
                        break;
                    }
                }
            }

            // if not a child open the IOI channel
            if (!m_parameters[index].m_isChild)
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
                    m_ioiOpenFailCount += 1;
                }
            }

            // default the display to the first eIoiMaxDisplay parameters created
            if (m_displayCount < eIoiMaxDisplay && m_parameters[index].m_ioiValid)
            {
                m_displayIndex[m_displayCount] = index;
                m_displayCount += 1;
            }
        }

        // save the param cfg
        //m_paramCfg.Open(PARAM_CFG, File::ePartCmProc, 'w');
        //m_paramCfg.Write(&m_paramInfo, sizeof(m_paramInfo[0])*m_paramInfoCount);
        //m_paramCfg.Close();

        // clear out the paramInfo
        memset((void*)&m_paramInfo, 0, sizeof(m_paramInfo));
        m_paramInfoCount = 0;

        ScheduleParameters();

        m_updated = 3; // TODO: delete this line
    }

}

//-------------------------------------------------------------------------------------------------
// Function: ScheduleParameters
// Description: Compute the offsets for the parameters to balance the frame load
//
void IoiProcess::ScheduleParameters()
{
    // TODO: set the parameter offsets

}


