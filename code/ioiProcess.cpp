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

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
IoiProcess::IoiProcess()
    : m_paramCount(0)
    , m_displayCount(0)
    , m_frames(0)
    , m_scheduled(0)
    , m_updated(0)
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
    processStatus ps = createProcessAlias( "ioi");
    // TBD: may want this to act as io cross-channel also

    // create all of the IOI items
    InitIoi();

    // Create the thread thru the base class method.
    // Use the default Ase template
    Launch("IoiProcess", "StdThreadTemplate");
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
            for (UINT32 i = 0; i < eIoiMaxParams; ++i)
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

    snsNames.pBaseIndex = eIoiMaxParams;
    for (i = start; (filledIn < eSecNumberOfSensors && i < eIoiMaxParams); ++i)
    {
        if (snsNames.pBaseIndex != eIoiMaxParams || m_parameters[i].m_isValid)
        {
            if (snsNames.pBaseIndex == eIoiMaxParams)
            {
                snsNames.pBaseIndex = i;
            }
            strncpy(snsNames.names[filledIn], m_parameters[i].m_name, eAseSensorNameSize);
            filledIn += 1;
        }
    }
}

/****************************************************************************
 protected methods for IoIProcess
****************************************************************************/

//-------------------------------------------------------------------------------------------------
// Function: RunSimulation
// Description: Processing done to simulate the ioi process
//
void IoiProcess::RunSimulation()
{
    UNSIGNED32* pSystemTickTime;
    UNSIGNED32  nextRequestTime;
    UNSIGNED32  nowTime;
    UNSIGNED32  interval = 100;  // 100 X 10 millisecs/tick = 1 sec interval

    // Grab the system tick pointer
    pSystemTickTime = systemTickPointer();


    while (1)
    {
        UpdateIoi();

        // TODO: at 50 Hz pack the CCDL message and send it out

        waitUntilNextPeriod();
    }
}

//-------------------------------------------------------------------------------------------------
// Function: InitIoi
// Description: Processing done to simulate the ioi process
//
void IoiProcess::InitIoi()
{
    UINT32 i;

    // TODO - set these up for real form the cfg file
    m_paramCount = 5;
    m_maxParamIndex = 2999;
    m_parameters[   0].Reset("P0",    20, PARAM_FMT_A429, 0x36240, 10000, 161, 90);
    m_parameters[  10].Reset("P10",   10, PARAM_FMT_A429, 0x36640, 10000, 161, 90);
    m_parameters[ 256].Reset("P256",  20, PARAM_FMT_A429, 0x36a40, 10000, 161, 90);
    m_parameters[ 311].Reset("P311",  10, PARAM_FMT_A429, 0x36e40, 10000, 161, 90);
    m_parameters[2999].Reset("P2999", 50, PARAM_FMT_A429, 0x38268, 10000, 150, 90);

    // TODO - default to displaying the first 20 parameters (or max param count)
    m_displayIndex[0] = 0;
    m_displayIndex[1] = 10;
    m_displayIndex[2] = 256;
    m_displayIndex[3] = 311;
    m_displayIndex[4] = 2999;
    m_displayCount = 5;

    m_updated = 3;
}

//-------------------------------------------------------------------------------------------------
// Function: UpdateIoi
// Description: Update all of the "output" parameters from the ioi process
//
void IoiProcess::UpdateIoi()
{
    UINT32 i;
    char outputLine[80];
    char sgType[80];
    UINT32 atLine = 2;

    m_frames += 1;
    m_scheduled = m_paramCount; // need to see how many are really scheduled
    m_updated ^= 1;             // toggle the lsb

    debug_str(Ioi, 1, 0, "Frame: %6d Scheduled: %4d Updated: %4d SigGen: %s         ",
              m_frames, m_scheduled, m_updated,
              m_sgRun ? "Run" : "Hold");

    UNSIGNED32 *systemTickPtr = systemTickPointer();
    for (i=0; i < (m_maxParamIndex+1); ++i)
    {
        m_parameters[i].Update( *systemTickPtr, m_sgRun);
    }

    // display parameter data
    for (i=0; i < m_displayCount; ++i)
    {
        UINT32 x = m_displayIndex[i];
        m_parameters[x].m_sigGen.GetRepresentation(sgType);

        debug_str(Ioi, atLine, 0, "%32s(%6d): %10.4f - 0x%08x - %d      ",
                  m_parameters[x].m_name, m_parameters[x].m_updateCount,
                  m_parameters[x].m_value,
                  m_parameters[x].m_rawValue, m_parameters[x].m_rawValue, sgType);
        atLine += 1;

        debug_str(Ioi, atLine, 0, "%39s: %s        ",
                  "", sgType);
        atLine += 1;
    }
}


