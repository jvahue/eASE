/******************************************************************************
          Copyright (C) 2013-2016 Knowlogic Software Corp.
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
#include "AseCommon.h"

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
const char* paramType5[] = {
    "None ",
    "A664B",
    "A664F",
    "A429 "
};

const char* a429Fmt5[] = {
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

//---------------------------------------------------------------------------------------------
// Function: Reset
// Description: Reset the parameter values
//
void Parameter::Reset()
{
    m_ioiValid = false;
    m_ioiChan = -1;
    m_isRunning = true;        // parameter comes up running
    m_index = eAseMaxParams;
    m_nextIndex = eAseMaxParams;
    m_value = 0.0f;            // the current value for the parameter
    m_rawValue = 0;
    m_ioiValue = 0;            // current ioi value after Update
    m_rateHz = 0;              // ADRF update rate for the parameter in Hz
    m_updateMs = 0;
    m_updateIntervalTicks = 0; // ASE update rate for the parameter in Hz = 2x m_rateHz*
    m_offset = 0;              // frame offset 0-90 step 10
    m_nextUpdate = 0;
    m_updateCount = 0;
    m_link = NULL;
    m_isChild = false;
    m_name[0] = '\0';
    m_childCount = 0;

    ParamConverter::Reset();

    // clear out the SigGen
    m_sigGen.SetParams(eSGmanual, 0, 0.0f, 0.0f, 0.0f, 0.0f);
    m_sigGen.Reset(m_value);
}

//---------------------------------------------------------------------------------------------
// Function: CompressName
// Description: remove vowels from the end until strlen(src) <= size
//
char* Parameter::CompressName(char* src, int size)
{
    char* vowel;
    char* from;
    char* to;
    int at = strlen(src) - 1;

    // remove vowels from the back until less than 24 chars long
    while (strlen(src) > size && at >= 0)
    {
        vowel = strpbrk(&src[at], "aeiouAEIOU");
        if (vowel != NULL)
        {
            to = vowel;
            from = ++vowel;
            while (*from != NULL && from != &src[at])
            {
                *to++ = *from++;
            }
            *to = '\0';
        }
        at -= 1;
    }

    if (strlen(src) > size)
    {
        src[size] = '\0';
    }

    return src;
}


//---------------------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the parameter based on the configuration values
//
void Parameter::Init(ParamCfg* paramInfo)
{
    //UINT32 at;
    //UINT32 dst;
    UINT32 extraMs;

    strncpy(m_name, paramInfo->name, eAseParamNameSize);
    strncpy(m_shortName, paramInfo->name, eAseParamNameSize);
    CompressName(m_shortName, eParamShort);

    m_index = paramInfo->index;

    m_rateHz = paramInfo->rateHz;

    if (paramInfo->src != PARAM_SRC_CROSS)
    {
        m_updateMs = 1000 / (paramInfo->rateHz * 2);
    }
    else
    {
        m_updateMs = 1000 / paramInfo->rateHz;
    }
    extraMs = m_updateMs % 10;
    m_updateIntervalTicks = m_updateMs - extraMs;
    m_updateIntervalTicks /= 10;  // turn this into system ticks

    ParamConverter::Init(paramInfo);

    m_isValid = true;

    m_ioiValid = false;
}

//---------------------------------------------------------------------------------------------
// Function: IsChild
// Description: Indicate if this parameter is a child of the other parameter.
// If it is hook up this parameter to the other
//
bool Parameter::IsChild(Parameter& other)
{
    bool childRelationship = false;

    // verify the src is the same
    if (m_src == other.m_src)
    {
        if (m_src == PARAM_SRC_A664 || m_src == PARAM_SRC_CROSS)
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
            }
        }

        if (m_isChild)
        {
            childRelationship = true;

            // If we run faster than the parent we become the parent
            if (m_rateHz > other.m_rateHz)
            {
                // reset the parent/child indicators
                other.m_isChild = true;
                m_isChild = false;

                // relink/move ioi assignments to the new parent
                m_link = &other;
                m_ioiChan = other.m_ioiChan;
                m_ioiValid = other.m_ioiValid;
                m_isValid = other.m_isValid;

                // clear old parent ioi data
                other.m_ioiChan = -1;
                other.m_ioiValid = false;
            }

            // else walk down the child list and attach this
            else
            {
                Parameter* parent = &other;
                while (parent->m_link != NULL)
                {
                    parent = parent->m_link;
                }
                parent->m_link = this;
            }
        }
    }

    return childRelationship;
}

//---------------------------------------------------------------------------------------------
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

    start = HsTimer();
    // see if it is time for an update
    if (m_nextUpdate <= sysTick)
    {
        // only the 'root' parent collects the children's data 
        if (!m_isChild && m_link != NULL)
        {
            // compute the children of this parameter
            Parameter* cp = m_link;

            m_childCount = 0;
            while (cp != NULL)
            {
                m_childCount += 1;
                // each param updates itself at it's rate 
                // here we just pick up the value
                //count += cp->Update(sysTick, sgRun);
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
    }
    m_updateDuration = HsTimeDiff(start);

    return count;
}


//---------------------------------------------------------------------------------------------
// Function: Display
// Description: Display some info about the param
//
char* Parameter::Display(char* buffer)
{
    sprintf(buffer, "%4d:%-23s:%10.3f", m_index, m_shortName, m_value);
    return buffer;
}

//---------------------------------------------------------------------------------------------
// Function: ParamInfo
// Description: Display some info about the param
// row = 0: param type, child, rate, etc.
// row = 1: sigGen info
//
char* Parameter::ParamInfo(char* buffer, int row)
{
    char sgRep[80];

    if (m_type == PARAM_FMT_A429)
    {
        if ( row == 0)
        {
            //               #   Type(Fmt) Rate Child SigGen
            //               0   5  11  18      25     31  34
            //               v   v  v   v       v      v   v
            sprintf(buffer, "%4d:%s(%s) %2dHz - %2d in %2d %s",
                m_index,
                paramType5[m_type],
                a429Fmt5[m_a429.format],
                m_rateHz,
                m_childCount+1, m_updateDuration,
                m_isChild ? "Kid" : ""
            );
        }
        else if ( row == 1)
        {
            sprintf(buffer, "     0x%08x - %d %s", m_rawValue, m_data, m_ioiName);
            buffer[39] = '\0';
        }
        else if ( row == 2)
        {
            m_sigGen.GetRepresentation(sgRep);
            sprintf(buffer, "     %s", sgRep);
        }
        else
        {
            sprintf(buffer, "Invalid row %d", row);
        }
    }
    else
    {
        if ( row == 0)
        {
            //               #   Type(Fmt) Rate Child SigGen
            //               0   5  11  18      25     31  34
            //               v   v  v   v       v      v   v
            sprintf(buffer, "%4d:%s %2dHz - %2d in %2d %s",
                m_index,
                paramType5[m_type],
                m_rateHz,
                m_childCount+1, m_updateDuration,
                m_isChild ? "Kid" : ""
                );
        }
        else if ( row == 1)
        {
            sprintf(buffer, "     0x%08x - %d %s", m_rawValue, m_data, m_ioiName);
            buffer[39] = '\0';
        }
        else if ( row == 2)
        {
            m_sigGen.GetRepresentation(sgRep);
            sprintf(buffer, "     %s", sgRep);
        }
        else
        {
            sprintf(buffer, "Invalid row %d", row);
        }
    }

    return buffer;
}


