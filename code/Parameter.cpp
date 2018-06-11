/******************************************************************************
          Copyright (C) 2013-2017 Knowlogic Software Corp.
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
#define _10msTick(period, step) int(((period) * (step)) / 10.0)

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
    "A429 ",
    "Int  ",
    "Float",
    "DISC "
};

const char* a429Fmt5[] = {
    "BNR  ",
    "BCD  ",
    "Disc ",
    "Other"
};

static FlexSeq1Tbl flexSeq1Table[MAX_FLEX_SEQ1_TBLS];
static bool flexSeq1TblsInit = false;

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/

// search for MGBH and move these back to local scope when ready
FLOAT32 wordOffset;
FLOAT32 msPerWord;
FLOAT32 minWord;
FLOAT32 maxWord;
FLOAT32 baseWord;

/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Class Definitions                                                         */
/*****************************************************************************/
Parameter::Parameter()
    : m_flexDataTbl(-1)
{
    Reset();

    if (!flexSeq1TblsInit)
    {
        for (int t = 0; t < MAX_FLEX_SEQ1_TBLS; ++t)
        {
            flexSeq1Table[t].assignIndex = -1;
            memset(flexSeq1Table[t].data, 0, sizeof(FlexSeq1Data));
        }

        flexSeq1TblsInit = true;
    }
}

//---------------------------------------------------------------------------------------------
// Function: Reset
// Description: Reset the parameter values
//
void Parameter::Reset()
{
    m_ioiValid = false;
    m_isRunning = true;        // parameter comes up running
    m_useUint = false;

    m_name[0] = '\0';
    m_shortName[0] = '\0';

    m_index = eAseMaxParams;
    m_nextIndex = eAseMaxParams;
    m_uintValue = 0;
    m_value = 0.0f;            // the current value for the parameter
    m_rawValue = 0;

    m_ioiChan = -1;
    m_ioiValue = 0;            // current ioi value after Update
    m_idl = NULL;

    m_rateHz = 0;              // ADRF update rate for the parameter in Hz
    m_updateMs = 0;
    m_updateIntervalTicks = 0; // ASE update rate for the parameter in Hz = 2x m_rateHz*
    m_offset = 0;              // frame offset 0-90 step 10
    m_nextUpdate = 0;
    m_updateCount = 0;
    m_ioiWrSucc = 0;
    m_ioiWrFail = 0;
    m_ccdlId = 0xffff;

    m_link = NULL;
    m_isChild = false;
    m_childCount = 0;

    if (m_flexDataTbl != -1)
    {
        // clear assignments
        flexSeq1Table[m_flexDataTbl].assignIndex = -1;
        memset(&flexSeq1Table[m_flexDataTbl].data, 0, sizeof(FlexSeq1Data));
    }
    // flex roots
    m_flexType = eFlexNone;
    m_flexDataTbl = -1;
    // flex children
    m_flexRootIdx = -1;
    m_flexSeq = -1;
    m_flexRoot = NULL;

    // QAR Data stream parameters
    m_qar = NULL;           // this can either be a A664 or A717 QAR pointer
    m_qarSfMask = 0;        // Sub-frame mask 0:SF1, 1:SF2, 2:SF3, 3:SF4
    m_qarRateHz = 0;        // actual QAR rate Hz 1, 2, 4, 8, 16, 32, 64 (ONLY)
    m_qarSlot = 0;          // which slot are we computing 0 .. (m_qarRateHz-1)
    m_qarLenA = 0;
    m_qarLsbA = 0;
    m_gpaMask = 0;          // field mask for value (or LSB)
    m_gpaWord = 0;          // base index for value (or LSB)
    m_qarLenE = 0;
    m_qarLsbE = 0;
    m_gpeMask = 0;          // field mask for MSB
    m_gpeWord = 0;          // base index for MSB
    m_qarPeriod_ms = 0.0f;  // actual QAR period (1.0/Hz)

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
// Set the raw data field portion of the <index> word in the sequence
void Parameter::SetFlex(UINT32 rawValue, int index)
{
    if (m_flexDataTbl != -1)
    {
        flexSeq1Table[m_flexDataTbl].data[index] = rawValue;
    }
}

//---------------------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the parameter based on the configuration values.  Params are mostly
// organized by their source.  Each source should handle all the setup required for the 
// parameter to 
//   1. updates its value in its value stream IOI, FLEX, QAR, etc
//   2. compute the rate to send the value (i.e., it m_updateIntervalTicks
//
void Parameter::Init(ParamCfg* paramInfo, StaticIoiContainer& ioiStatic)
{
    UINT32 extraMs;
    bool status = true;

    ParamConverter::Init(paramInfo);

    strncpy(m_name, paramInfo->name, eAseParamNameSize);
    strncpy(m_shortName, paramInfo->name, eAseParamNameSize);
    CompressName(m_shortName, eParamShort);

    m_index = paramInfo->index;
    m_rateHz = paramInfo->rateHz;

    // check for FLEX params root and child
    if (paramInfo->src == PARAM_SRC_FLEX)
    {
        // ok we have a flex child - get its seq #, caller will link the parent (m_flexParent)
        m_flexRootIdx = (m_gpe >> 16) & 0xffff;
        m_flexSeq = (m_gpe >> 8) & 0xff;
    }
    else if ((paramInfo->src == PARAM_SRC_QAR_A664) || (paramInfo->src == PARAM_SRC_QAR_A717))
    {
        m_qarLenA = EXTRACT(paramInfo->gpa, 0, 4);
        m_qarLsbA = EXTRACT(paramInfo->gpa, 4, 4) - (m_qarLenA - 1);
        m_gpaMask = ~MASK(m_qarLsbA, m_qarLenA);        // field mask for LSdata
        m_gpaWord = EXTRACT(paramInfo->gpa, 16, 10);   // base index for LSdata

        m_qarLenE = EXTRACT(paramInfo->gpe, 0, 4);
        if (m_qarLenE > 0)
        {
            m_qarLsbE = EXTRACT(paramInfo->gpe, 4, 4) - (m_qarLenE - 1);
            m_gpeMask = ~MASK(m_qarLsbE, m_qarLenE);       // field mask for MSdata
            m_gpeWord = EXTRACT(paramInfo->gpe, 16, 10);  // base index for MSdata
        }
        else
        {
            m_qarLsbE = 0;
            m_gpeMask = 0;       // field mask for MSdata = 0 does not exist
            m_gpeWord = 0xffff;  // base index for MSdata
        }

        // items associated with the parameter's update rate
        m_qarSfMask = EXTRACT(paramInfo->gpa, 8, 4); // mask 0:SF1, 1:SF2, 2:SF3, 3:SF4
        m_qarRateHz = 1 << EXTRACT(paramInfo->gpe, 28, 4);

        // limit this to 1Hz even if the value only appears in one or two SF for 0.25|0.5 Hz
        m_qarPeriod_ms = 1000.0/(float)m_qarRateHz;  // actual QAR period (1.0/Hz) 
        m_qarSlot = 0;  // SF slot are we computing 0 .. (m_qarRateHz-1)

        // attach the parameter to the QAR data stream
        m_qar = (paramInfo->src == PARAM_SRC_QAR_A664
                 ? &ioiStatic.m_a664Qar 
                 : &ioiStatic.m_a717Qar);
    }
    else if ((paramInfo->src == PARAM_SRC_A429) || (paramInfo->src == PARAM_SRC_A664) ||
             (paramInfo->src == PARAM_SRC_A429_A) || (paramInfo->src == PARAM_SRC_CROSS))
    {
        if ((paramInfo->gpe & 0xFF) == eFlexSeq1)
        {
            m_flexType = eFlexSeq1;

            // we need to allocate a seq word table, scan our tables and find an open one
            for (int t = 0; t < MAX_FLEX_SEQ1_TBLS; ++t)
            {
                if (flexSeq1Table[t].assignIndex == -1)
                {
                    flexSeq1Table[t].assignIndex = t;
                    m_flexDataTbl = t;
                    m_flexSeq = 0;  // start by transmitting on the 0th word
                    break;
                }
            }

            if (m_flexDataTbl == -1)
            {
                // too many FLEX roots
                status = false;
            }
        }
        // ADD new FLEX protocols (like eFlexSeq1) here

        // if not a known FLEX protocol it better be 0
        else if ((paramInfo->gpe & 0xFF) != 0)
        {
            // invalid type fail
            status = false;
        }
    }

    // Note: none of this is used for QAR parameter scheduling
    if (m_flexDataTbl != -1)
    {
        // reduce the rate by a factor of 2 for flex so we don't overwrite the flex data
        m_updateMs = 10000 / ((paramInfo->rateHz * 10) / 2);
    }
    else if (paramInfo->src == PARAM_SRC_CROSS)
    {
        m_updateMs = 1000 / paramInfo->rateHz;
    }
    else
    {
        m_updateMs = 1000 / (paramInfo->rateHz * 2);
    }

    extraMs = m_updateMs % 10;
    m_updateIntervalTicks = m_updateMs - extraMs;
    m_updateIntervalTicks /= 10;  // turn this into system ticks

    m_idl = ioiStatic.FindIoi(m_ioiName);

    m_isValid = status;

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

        else if (m_src == PARAM_SRC_FLEX)
        {
            // see if this is the same root parameter and seq index
            m_isChild = (m_gpe == other.m_gpe);
        }

        else if (m_type == PARAM_FMT_A429 && other.m_type == PARAM_FMT_A429)
        {
            if (m_src == PARAM_SRC_A429_A)
            {
                m_isChild = m_gpc == other.m_gpc;
            }
            else
            {
                m_isChild = (//m_a429.channel == other.m_a429.channel &&  // same channel
                             m_a429.label == other.m_a429.label   &&  // same label
                             m_a429.sdBits == other.m_a429.sdBits);
            }
        }

        if (m_isChild)
        {
            childRelationship = true;

            // If we run faster than the other we become the parent
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
    bool updateNow;

    int lsbSrc;
    int lsbDst;

    UINT8 destByte;
    UINT8 destBit0;
    UINT8 msbDst;
    UINT8 moveSize;
    UINT8 bitsMoved;  // the first move will move n bits
    UINT8 dMask;
    UINT8 src;
    UINT8 insert;

    UINT8* dest;

    UINT16 destSize;

    UINT32 start;
    UINT32 count = 0;
    UINT32 children = 0;

    start = HsTimer();

    // compute if it time for a parameter update
    if (!m_qar)
    {
        // normal parameter based on system tick
        updateNow = m_nextUpdate <= sysTick;
    }
    else
    {
        // look at the SF and where we are in the one second frame vs word slot positions
        if (BIT(m_qarSfMask, m_qar->m_sf))
        {
            // MOVE GLOBALS BACK HERE MGBH
            // figure out what word we are on in the frame based on the QAR timer
            msPerWord = m_qar->m_qarSfWordCount / 100.0f; // words/MIF
            minWord = m_qar->m_oneSecondClk * msPerWord;
            maxWord = minWord + msPerWord;
            baseWord = (FLOAT32)MIN(m_gpaWord, m_gpeWord);

            wordOffset = baseWord + ((m_qar->m_qarSfWordCount / m_qarRateHz) * m_qarSlot);

            updateNow = MEMBER(wordOffset, minWord, maxWord);
        }
    }

    // see if it is time for an update
    if (updateNow)
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
                if (cp->m_isRunning)
                {
                    children |= cp->m_rawValue;
                }
                cp = cp->m_link;
            }
        }

        if (m_useUint)
        {
            m_rawValue = m_uintValue;
        }

        else if (m_flexType == eFlexSeq1)
        {
            UINT32 seqX = (m_flexSeq & 0xF) << 24;
            m_rawValue = flexSeq1Table[m_flexDataTbl].data[m_flexSeq];

            // pack the seq index
            m_rawValue |= seqX;
            m_rawValue |= m_a429.a429Template;

            if (m_type == PARAM_FMT_A429)
            {
                m_rawValue |= 0x80000000;
            }

            // move to the next word
            m_flexSeq = INC_WRAP(m_flexSeq, MAX_FLEX_SEQ1_WORDS);
        }

        else
        {
            // Update the value with the SigGen
            m_value = m_sigGen.Update(m_value, sgRun);

            // Convert it to the raw ioi format
            m_rawValue = Convert(m_value);
        }

        if (m_qar)
        {
            UINT16 value;

            // write new data to the SF data buffer
            // Get the value or the LS part
            value = FIELD(EXTRACT(m_rawValue, 0, m_qarLenA), m_qarLsbA, m_qarLenA);
            m_qar->SetData(value, m_gpaMask, m_qarSfMask, m_qarRateHz, m_gpaWord, m_qarSlot);
            if (m_qarLenE > 0)
            {
                // get the MSdat part
                value = FIELD(EXTRACT(m_rawValue, m_qarLenA, m_qarLenE), 
                              m_qarLsbE, m_qarLenE);
                m_qar->SetData(value, 
                               m_gpeMask, m_qarSfMask, m_qarRateHz, m_gpeWord, m_qarSlot);
            }

            m_qarSlot = INC_WRAP(m_qarSlot, m_qarRateHz);
        }
        else if (m_idl)
        {
            // we need to position the data in the IDL, m_rawValue holds the data which
            // cannot be bigger than 32 bits for our purposes as a parameter
            destByte = a664Offset / 8;
            destBit0 = a664Offset % 8;
            msbDst = (8 - destBit0);         // re-align with little-endian bit addressing

            moveSize = MIN(a664Size, msbDst);  // the first move will move n bits
            lsbSrc = a664Size - moveSize;
            lsbDst = msbDst - moveSize;

            bitsMoved = 0;

            dest = m_idl->Data(&destSize);

            while (bitsMoved < a664Size)
            {
                // clear the destination
                dMask = MASK(lsbDst, moveSize);
                dest[destByte] = RESET_BITS(dest[destByte], dMask);
                // extract the insert bits and repositions them
                src = EXTRACT(m_rawValue, lsbSrc, moveSize);
                insert = FIELD(src, lsbDst, moveSize);
                // set the destination bits
                dest[destByte] = SET_BITS(dest[destByte], insert);

                // compute next move
                bitsMoved += moveSize;
                moveSize = MIN(8, a664Size - bitsMoved);

                lsbSrc = lsbSrc - moveSize;
                lsbSrc += (lsbSrc < 0) ? 8 : 0;
                lsbDst = lsbDst - moveSize;
                lsbDst += (lsbDst < 0) ? 8 : 0;

                destByte += 1;
            }
        }
        else
        {
            m_ioiValue = m_rawValue | children;
        }

        // for FLEX, move the final value over to the FLEX Table
        if (m_src == PARAM_SRC_FLEX && !m_isChild)
        {
            m_flexRoot->SetFlex(m_ioiValue, m_flexSeq);
        }

        // reschedule non-QAR parameters
        if (m_updateIntervalTicks > 0)
        {
            m_nextUpdate += m_updateIntervalTicks;
        }

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
        if (row == 0)
        {
            //               #   Type(Fmt) Rate Child SigGen
            //               0   5  11  18      25     31  34
            //               v   v  v   v       v      v   v
            sprintf(buffer, "%4d:%s(%s) %2dHz - %2d in %2d %s",
                    m_index,
                    paramType5[m_type],
                    a429Fmt5[m_a429.format],
                    m_rateHz,
                    m_childCount + 1, m_updateDuration,
                    m_isChild ? "Kid" : ""
            );
        }
        else if (row == 1)
        {
            sprintf(buffer, "     0x%08x - %d %s", m_rawValue, m_data, m_ioiName);
            buffer[39] = '\0';
        }
        else if (row == 2)
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
        if (row == 0)
        {
            //               #   Type(Fmt) Rate Child SigGen
            //               0   5  11  18      25     31  34
            //               v   v  v   v       v      v   v
            sprintf(buffer, "%4d:%s %2dHz - %2d in %2d %s",
                    m_index,
                    paramType5[m_type],
                    m_rateHz,
                    m_childCount + 1, m_updateDuration,
                    m_isChild ? "Kid" : ""
            );
        }
        else if (row == 1)
        {
            sprintf(buffer, "     0x%08x - %d %s", m_rawValue, m_data, m_ioiName);
            buffer[39] = '\0';
        }
        else if (row == 2)
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


