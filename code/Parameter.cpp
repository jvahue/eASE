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
    "A429 "
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
{
    Reset();
}

//-------------------------------------------------------------------------------------------------
// Function: Reset
// Description: Reset the parameter values
//
void Parameter::Reset()
{
    m_ioiValid = false;
    m_index = eAseMaxParams;
    m_value = 0.0f;            // the current value for the parameter
    m_rawValue = 0;
    m_ioiValue = 0;            // current ioi value after Update
    //m_ioiValueZ1 = 0xffffffff; // the last IOI value
    m_rateHz = 0;              // ADRF update rate for the parameter in Hz
    m_updateMs = 0;
    m_updateIntervalTicks = 0; // ASE update rate for the parameter in Hz = 2x m_rateHz
    m_offset = 0;              // frame offset 0-90 step 10
    m_nextUpdate = 0;
    m_updateCount = 0;
    m_link = NULL;
    m_isChild = false;
    m_name[0] = '\0';
    m_childCount = 0;
}

//-------------------------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the parameter based on the configuration values
//
void Parameter::Init(ParamCfg* paramInfo)
{
    UINT32 extraMs;

    strncpy(m_name, paramInfo->name, eAseParamNameSize);
    m_index = paramInfo->index;

    m_rateHz = paramInfo->rateHz;

    m_updateMs = 1000 / (paramInfo->rateHz * 2.0);
    extraMs = m_updateMs % 10;
    m_updateIntervalTicks = m_updateMs - extraMs;
    m_updateIntervalTicks /= 10;  // turn this into system ticks

    ParamConverter::Reset(paramInfo);
    m_isValid = true;

    m_ioiValid = false;
}

//-------------------------------------------------------------------------------------------------
// Function: IsChild
// Description: Indicate if this parameter is a child of the other parameter.
// If it is hook up this parameter to the other
//
bool Parameter::IsChild(Parameter& other)
{
    if (m_src == other.m_src)
    {
        if (m_src == PARAM_SRC_A664)
        {
            m_isChild = m_masterId == other.m_masterId;
        }
        else if (m_type == PARAM_FMT_A429 && other.m_type == PARAM_FMT_A429)
        {
            if (//m_a429.channel == other.m_a429.channel &&  // same channel
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
        // TODO check A664 children?
    }

    return m_isChild;
}

//-------------------------------------------------------------------------------------------------
// Function: Update
// Description: Update the parameter value
// sysTick: what time is it now
// sgRun: is the Sig Gen running
// Returns: count of params updated under this one
//
UINT32 Parameter::Update(UINT32 sysTick, bool sgRun)
{
    UINT32 start;
    UINT32 count = 0;
    UINT32 children = 0;

    // see if it is time for an update
    if (m_nextUpdate < sysTick)
    {
        start = HsTimer();

        // only the 'root' parent updates its children
        if (!m_isChild)
        {
            // compute the children of this parameter
            Parameter* cp = m_link;

            m_childCount = 0;
            while (cp != NULL)
            {
                m_childCount += 1;
                count += cp->Update(sysTick, sgRun);
                children |= cp->m_rawValue;
                cp = cp->m_link;
            }
        }

        // Update the value with the SigGen
        m_value = m_sigGen.Update(m_value, sgRun);

        // Convert it to the raw ioi format
        m_rawValue = Convert(m_value);

        m_ioiValue = m_rawValue | children;

        m_nextUpdate = sysTick + m_updateIntervalTicks;
        m_updateCount += 1;
        count += 1;
        m_updateDuration = HsTimeDiff(start);
    }

    return count;
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
        sprintf(buffer, "%s(%s) %dHz %s %s - %d in %d",
            paramType[m_type],
            a429Fmt[m_a429.format],
            m_rateHz,
            m_isChild ? "Child" : "",
            sgRep,
            m_childCount+1, m_updateDuration
        );
    }
    else
    {
        sprintf(buffer, "%s - oops not supported yet", paramType[m_type]);
    }

    return buffer;
}


