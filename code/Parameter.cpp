/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: Parameter.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

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
    : m_gpa(0)
    , m_gpb(0)
    , m_gpc(0)
    , m_fmt(PARAM_FMT_NONE)
    , m_value(0.0f)     // the current value for the parameter
    , m_scale(0)        // the current value for the parameter
    , m_scaleLsb(0.0f)
    , m_rateHz(0)       // ADRF update rate for the parameter in Hz
    , m_updateHz(0)     // ASE update rate for the parameter in Hz = 2x m_rateHz
    , m_offset(0)       // frame offset 0-90 step 10
    , m_converter(NULL)
{
    m_name[0] = '\0';
}


//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
void Parameter::Reset( char* name, PARAM_FMT_ENUM fmt, 
                       UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale)
{
    strncpy(name, m_name, eAseSensorNameSize);
    m_converter.Reset( fmt, gpa, gpb, gpc, scale)
}    

