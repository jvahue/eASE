/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: Parameter.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
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
Parameter::Parameter()
    : m_isValid(FALSE)
    , m_value(0.0f)     // the current value for the parameter
    , m_rawValue(0)
    , m_rateHz(0)       // ADRF update rate for the parameter in Hz
    , m_updateMs(0)
    , m_updateIntervalTicks(0)     // ASE update rate for the parameter in Hz = 2x m_rateHz
    , m_offset(0)       // frame offset 0-90 step 10
    , m_nextUpdate(0)
    , m_updateCount(0)
{
    m_name[0] = '\0';
}


//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
void Parameter::Reset( char* name, UINT32 rate, PARAM_FMT_ENUM fmt,
                       UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale)
{
    UINT32 extraMs;

    strncpy(m_name, name, eAseSensorNameSize);

    m_rateHz = rate;

    m_updateMs = 1000 / (rate * 2.0);
    extraMs = m_updateMs % 10;
    m_updateIntervalTicks = m_updateMs - extraMs;
    m_updateIntervalTicks /= 10;  // turn this into system ticks

    ParamConverter::Reset(fmt, gpa, gpb, gpc, scale);
    m_isValid = TRUE;

    UNSIGNED32 *systemTickPtr = systemTickPointer();
    m_nextUpdate = *systemTickPtr + m_updateIntervalTicks;
}    

void Parameter::Update(UINT32 sysTick, bool sgRun)
{
    if (m_isValid)
    {
        // see if it is time for an update
        if (m_nextUpdate < sysTick)
        {
            m_value = m_sigGen.Update(m_value, sgRun);
            m_nextUpdate += m_updateIntervalTicks;
            m_updateCount += 1;
        }
    }
}
