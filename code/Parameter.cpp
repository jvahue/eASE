/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: Parameter.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
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
    , m_updateMs(0)     // ASE update rate for the parameter in Hz = 2x m_rateHz
    , m_offset(0)       // frame offset 0-90 step 10
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
    m_updateMs -= extraMs;
    ParamConverter::Reset(fmt, gpa, gpb, gpc, scale);
    m_isValid = TRUE;
}    

void Parameter::Update()
{
    if (m_isValid)
    {
    }
}
