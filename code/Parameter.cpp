/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: Parameter.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <stdio.h>
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
const char* paramType[] = {
    "None ",
    "A664B",
    "A664F",
    " A429"
};

const char* a429Fmt[] = {
    "BNR  ",
    "BCD  ",
    "Disc ",
    "Other"
};

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
    : m_ioiValid(false)
    , m_value(0.0f)     // the current value for the parameter
    , m_rawValue(0)
    , m_ioiValue(0)    // current ioi value after Update
    , m_ioiValueZ1(0)  // the last IOI value
    , m_rateHz(0)       // ADRF update rate for the parameter in Hz
    , m_updateMs(0)
    , m_updateIntervalTicks(0)     // ASE update rate for the parameter in Hz = 2x m_rateHz
    , m_offset(0)       // frame offset 0-90 step 10
    , m_nextUpdate(0)
    , m_updateCount(0)
    , m_link(NULL)
    , m_isChild(false)
{
    m_name[0] = '\0';
}

//-------------------------------------------------------------------------------------------------
// Function: Reset
// Description: Reset the parameter based on the configuraiton values
//
void Parameter::Reset( char* name, UINT32 rate, PARAM_FMT_ENUM fmt,
                       UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale)
{
    UINT32 extraMs;

    strncpy(m_name, name, eAseParamNameSize);

    m_rateHz = rate;

    m_updateMs = 1000 / (rate * 2.0);
    extraMs = m_updateMs % 10;
    m_updateIntervalTicks = m_updateMs - extraMs;
    m_updateIntervalTicks /= 10;  // turn this into system ticks

    ParamConverter::Reset(fmt, gpa, gpb, gpc, scale);
    m_isValid = true;
}    

//-------------------------------------------------------------------------------------------------
// Function: IsChild
// Description: Indicate if this parameter is a child of the other parameter.
// If it is hook up this parameter to the other
//
bool Parameter::IsChild(Parameter& other)
{
    if (m_type == PARAM_FMT_A429 && other.m_type == PARAM_FMT_A429)
    {
        if (m_a429.channel == other.m_a429.channel &&  // same channel
            m_a429.label   == other.m_a429.label   &&  // same label
            m_a429.sdBits  == other.m_a429.sdBits)     // same SDI
        {
            m_isChild = true;
            // walk down the child list and attach this
            Parameter* parent = &other;
            while (parent->m_link != NULL)
            {
                parent = parent->m_link;
            }
            parent->m_link = this;
        }
    }

    return m_isChild;
}

//-------------------------------------------------------------------------------------------------
// Function: Update
// Description: Update the parameter value
//
void Parameter::Update(UINT32 sysTick, bool sgRun)
{
    if (m_isValid)
    {
        // see if it is time for an update
        if (m_nextUpdate < sysTick)
        {
            // compute the children of this parameter
            UINT32 children = 0;
            Parameter* cp = m_link;

            while (cp != NULL)
            {
              cp->Update(sysTick, sgRun);
              children |= cp->m_rawValue;
              cp = cp->m_link;
            }

            // Update the value with the SigGen
            m_value = m_sigGen.Update(m_value, sgRun);

            // Convert it to the raw ioi format
            m_rawValue = Convert(m_value);

            m_ioiValue = m_rawValue | children;

            if (m_ioiValue != m_ioiValueZ1)
            {
                // TODO: issue ioi update call
            }

            m_nextUpdate = sysTick + m_updateIntervalTicks;
            m_updateCount += 1;
        }
    }
}


//-------------------------------------------------------------------------------------------------
// Function: Display
// Description: Display some info about the param
//
char* Parameter::Display(char* buffer)
{
    char sgRep[80];
    m_sigGen.GetRepresentation(sgRep);

    if (m_type == PARAM_FMT_A429)
    {
        //              Type(Fmt) Rate Child SigGen
        sprintf(buffer, "%s(%s) %dHz %s %s",
            paramType[m_type],
            a429Fmt[m_a429.format],
            m_rateHz,
            m_isChild ? "Child" : "",
            sgRep
        );
    }
    else
    {
        sprintf(buffer, "%s - oops not supported yet", paramType[m_type]);
    }

    return buffer;
}


