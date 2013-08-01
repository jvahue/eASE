//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
//    File: Parameter.cpp
//
//    Description: The file implements the Parameter object processing
//
// Video Display Layout
//
// Frame: xxxxxx Updated: xxxx
//
// <Param1>: <floatValue> - <rawValue>
// <Param2>: <floatValue> - <rawValue>
// <Param3>: <floatValue> - <rawValue>
// ...
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

/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
struct ParamCfg {
    UINT32 index;
    ParameterName name;
    UINT32 rateHz;
    PARAM_FMT_ENUM fmt;
    UINT32 gpa;
    UINT32 gpb;
    UINT32 gpc;
    UINT32 scale;
};

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
const char* ioiInitStatus[] = {
//   !  max length    !
    "Success          ",
    "CfgFileNotFound  ",
    "CfgFileFailure   ",
    "NoCfgInfoForProc ",
    "InsufficientRAM  ",
    "FmttLibLoadFailed",
    "BffrLibLoadFailed",
    "SMONotCreated    ",
    "CannotAttachToSMO"
};

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
IoiProcess::IoiProcess()
    : m_paramCount(0)
    , m_displayCount(0)
    , m_frames(0)
    , m_scheduled(0)
    , m_updated(0)
    , m_ioiOpenFailCount(0)
    , m_ioiWriteFailCount(0)
    , m_sgRun(false)
{
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
    // create alias for this process because some process needs to source the
    // ioi data
    processStatus ps = createProcessAlias( "IO");
    // TBD: may want this to act as io cross-channel also

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
    char rep[80];
    char outputLine[80];
    UINT32 i;
    Parameter* param;
    ioiStatus writeStatus;

    UINT32 atLine = 2;

    m_frames += 1;
    m_scheduled = m_paramCount; // need to see how many are really scheduled
    m_updated ^= 1;             // toggle the lsb

    debug_str(Ioi, 1, 0, "ePySte: %s IOI: %s SigGen: %s Frame: %d",
              IS_CONNECTED ? "Conn  " : "NoConn",
              ioiInitStatus[m_initStatus],
              m_sgRun ? "Run" : "Hold",
              m_frames);

    if ( m_initStatus == ioiSuccess)
    {
        param = &m_parameters[0];
        for (i=0; i < (m_maxParamIndex+1); ++i)
        {
            if (param->m_isValid && !param->m_isChild && param->m_ioiValid)
            {
                param->Update( GET_SYSTEM_TICK, m_sgRun);
                writeStatus = ioi_write(param->m_ioiChan, &param->m_ioiValue);
                if (writeStatus != ioiSuccess)
                {
                    // TODO : what else?
                    m_ioiWriteFailCount += 1;
                }
            }
            ++param;
        }

        // display parameter data
        for (i=0; i < m_displayCount; ++i)
        {
            UINT32 x = m_displayIndex[i];

            debug_str(Ioi, atLine, 0, "%32s(%6d): %11.4f - 0x%08x(0x%08x)",
                      m_parameters[x].m_name, m_parameters[x].m_updateCount,
                      m_parameters[x].m_value,
                      m_parameters[x].m_rawValue, m_parameters[x].m_data);
            atLine += 1;

            debug_str(Ioi, atLine, 0, "%s", m_parameters[x].Display(rep));
            atLine += 1;
        }
    }

    debug_str(Ioi, atLine, 0, "Scheduled: %4d Updated: %4d", m_scheduled, m_updated);
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

    case eSetSensorValue:
        // TBD: This option should be removed as ePySte will convert a call to SetSensor('x', 33.3)
        //      into eSetSensorSG with the type set to manual
        break;

    case eSetSensorSG:
        if ( ARRAY( itemId, m_maxParamIndex+1) && m_parameters[itemId].m_isValid)
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
                sprintf(secComm.m_response.errorMsg, "Ioi: Invalid SG Type <%d>", sgType);
                secComm.m_response.successful = FALSE;
            }
        }
        else
        {
            sprintf(secComm.m_response.errorMsg, "Ioi: Invalid Param Index <%d>", itemId);
            secComm.m_response.successful = FALSE;
        }
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
                sprintf(secComm.m_response.errorMsg, "Ioi: Invalid Param Index <%d>", itemId);
                secComm.m_response.successful = FALSE;
            }
        }

        break;

    case eRunSG:
        if (!m_sgRun)
        {
            m_sgRun = true;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            sprintf( secComm.m_response.errorMsg, "Run SG Error: SGs are Running");
            secComm.m_response.successful = FALSE;
        }
        break;

    case eHoldSG:
        if (m_sgRun)
        {
            m_sgRun = false;
            secComm.m_response.successful = TRUE;
        }
        else
        {
            sprintf( secComm.m_response.errorMsg, "Hold SG Error: SGs are Holding");
            secComm.m_response.successful = FALSE;
        }
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
// Function: InitIoi
// Description: Processing done to simulate the ioi process
//
// Note: this function checks for child relationships between parameters.  Each parameter has a
// child link, that can be followed from the first (parent) param.
void IoiProcess::InitIoi()
{
#define pCnt 5
    UINT32 i;
    UINT32 i1;
    ioiStatus m_initStatus;
    ioiStatus openStatus;

    // TODO remove and fetch from Cfg file for the real system
    ParamCfg pCfg[pCnt] = {
        {0,    "ac_aoa1_raw",   20, PARAM_FMT_A429, 0x36240, 10000, 161, 90},
        {1,    "ac_aoa2_raw",   20, PARAM_FMT_A429, 0x36640, 10000, 161, 90},
        {22,   "airspeed1_raw",  4, PARAM_FMT_A429, 0x36220, 10000, 134, 512},
        {23,   "airspeed2_raw",  4, PARAM_FMT_A429, 0x36620, 10000, 134, 512},
        {2999, "ac_type_raw",    1, PARAM_FMT_A429, 0x38268, 10000, 150, 90},
    };

    // Initialize IOI Before use
    m_initStatus = ioi_init("");
    if (m_initStatus == ioiSuccess)
    {
        m_paramCount = 0;
        m_maxParamIndex = 0;
        for (i=0; i < pCnt; ++i)
        {
            UINT32 index = pCfg[i].index;

            m_paramCount += 1;
            if (index > m_maxParamIndex)
            {
                m_maxParamIndex = index;
            }

            m_parameters[index].Reset(pCfg[i].name, pCfg[i].rateHz, pCfg[i].fmt,
                                      pCfg[i].gpa, pCfg[i].gpb, pCfg[i].gpc,
                                      pCfg[i].scale);

            // check to see if this parameter is a child of any existing parameters
            for (i1=0; i1 < m_maxParamIndex+1; ++i1)
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
                openStatus = ioi_open(m_parameters[index].m_name, ioiWritePermission, (int*)&m_parameters[index].m_ioiChan);
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
            if (m_displayCount < eIoiMaxDisplay)
            {
                m_displayIndex[m_displayCount] = index;
                m_displayCount += 1;
            }
        }

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


