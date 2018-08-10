/******************************************************************************
Copyright (C) 2013-2017 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File: ParamConverters.cpp

Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "AseCommon.h"

#define ALLOW_A429_NAMES
#include "ParamA429Names.h"
#include "ParamConverters.h"

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
ParamConverter::ParamConverter()
{
    Reset();
}

//-------------------------------------------------------------------------------------------------
// Function: A429Converter
// Description: Convert the value and pack it into a rawValue
//
void ParamConverter::Reset()
{
    m_isValid = false;
    m_masterId = 0;
    m_gpa = 0;
    m_gpb = 0;
    m_gpc = 0;
    m_gpe = 0;
    m_src = PARAM_SRC_MAX;
    m_type = PARAM_FMT_NONE;
    m_scale = 0.0f;
    m_maxValue = 0.0f;
    m_scaleLsb = 0.0f;
    m_data = 0;

    memset(m_ioiName, 0, sizeof(m_ioiName));
    memset(&m_a429, 0, sizeof(m_a429));
}

//---------------------------------------------------------------------------------------------
// Function: Init
// Description: Initialize a converter with the parameter info
//
void ParamConverter::Init(ParamCfg* paramInfo)
{
    UINT32 rawLabel;

    m_gpa = paramInfo->gpa;
    m_gpb = paramInfo->gpb;
    m_gpc = paramInfo->gpc;
    m_gpe = paramInfo->gpe;

    m_src = paramInfo->src;
    m_type = paramInfo->fmt;

    m_masterId = paramInfo->masterId;
    m_scale = paramInfo->scale;
    m_maxValue = paramInfo->scale;

    if (m_type == PARAM_FMT_A429)
    {
        A429ParseGps();

        if (m_a429.format == eBNR  ||
            m_a429.format == eBNR1 ||
            m_a429.format == eBNRU)
        {
            m_scaleLsb = m_maxValue / pow(2.0f, float(m_a429.wordSize));
        }
        else if (m_a429.format == eBCD)
        {
            m_scaleLsb = m_scale/2.0f;
        }
        else
        {
            m_scaleLsb = 1.0;
        }
        // TODO: other formats

        // reverse the label
        int ms = 0x01;
        int md = 0x80;
        int cnt = 8;

        rawLabel = 0;
        while (cnt > 0)
        {
            if (m_a429.label & ms)
            {
                rawLabel |= md;
            }
            ms <<= 1;
            md >>= 1;
            --cnt;
        }

        // construct the basic message without data
        m_a429.a429Template = A429_FldPutLabel( 0, rawLabel);
        SetSdi( m_a429.sdBits);

        m_a429.SSM = ExpectedSSM();

        SetSsm( m_a429.SSM);

    }
    else if (m_type == PARAM_FMT_BIN_A664)
    {
        // Turn this into a A429 DISCRETE and use its discrete packing code
        a664Offset = m_gpa & 0xffff;
        a664Size = (m_gpa >> 16) & 0x7fff; // clear PySte indication that this is an IDL
    }
    else if (m_type == PARAM_FMT_FLT_A664)
    {
        // Turn this into a A429 DISCRETE and use its discrete packing code
        a664Offset = m_gpa & 0xffff;
        a664Size = (m_gpa >> 16) & 0x7fff; // clear PySte indication that this is an IDL
    }

    SetIoiName();
}

//---------------------------------------------------------------------------------------------
void ParamConverter::SetIoiName()
{
    if ((m_src == PARAM_SRC_A429) || (m_src == PARAM_SRC_A429_A))
    {
        SetIoiA429Name();
    }
    else if (m_src == PARAM_SRC_A664)
    {
        SetIoiA664Name();
    }
    else if (m_src == PARAM_SRC_CROSS)
    {
        sprintf(m_ioiName, "CROSS");
    }
}

//---------------------------------------------------------------------------------------------
void ParamConverter::SetIoiA429Name()
{
    if (m_src == PARAM_SRC_A429)
    {
        UINT32 i;
        UINT8 sdiMatch = m_a429.ignoreSDI ? A429_IOI_NAME_SDI_IGNORE_VAL : m_a429.sdBits;

        for (i=0; ioiA429Names[i].name[0] != NULL; ++i)
        {
            if (m_a429.label == ioiA429Names[i].octal && sdiMatch == ioiA429Names[i].sdi)
            {
                strcpy( m_ioiName, ioiA429Names[i].name);
                break;
            }
        }
    }
    else
    {
        char* sdiMap[] = { "00", "01", "10", "11", "XX" };
        UINT8 sdi = m_a429.ignoreSDI ? 4 : m_a429.sdBits;
        sprintf(m_ioiName, "%d_%03o_%s", m_a429.channel, m_a429.label, sdiMap[sdi]);
    }
}

//---------------------------------------------------------------------------------------------
void ParamConverter::SetIoiA664Name()
{
    UINT32 src;
    UINT32 ndo;

    src = m_masterId & 0xff;
    ndo = (m_masterId >> 8) & 0xffffff;
    sprintf(m_ioiName, "%d_%d", ndo, src);
}

//---------------------------------------------------------------------------------------------
void ParamConverter::A429ParseGps()
{
    UINT32 gpA = m_gpa;
    UINT32 gpB = m_gpb;

    m_a429.format     = A429WordFormat(gpA & 0x0F);
    m_a429.discType   = A429DiscretTypes((gpA >> 18) & 0x03);
    m_a429.wordSize   = (gpA >> 5)  & 0x1F;
    m_a429.msb        = (gpA >> 13) & 0x1F;
    m_a429.validSSM   = (gpA >> 20) & 0x0F;
    m_a429.sdiAllCall = (gpA >> 24) & 0x01;
    m_a429.label      = m_gpc & 0xff;

    if (m_src == PARAM_SRC_A429)
    {
        m_a429.channel   = gpA & 0x3;
        m_a429.sdBits    = (gpA >> 10) & 0x03;
        m_a429.ignoreSDI = (gpA >> 12) & 0x01;
    }
    else if (m_src == PARAM_SRC_A429_A)
    {
        m_a429.channel   = (m_gpc >> 12) & 0xF;
        m_a429.ignoreSDI = (m_gpc >> 10) & 0x1;
        m_a429.sdBits    = (m_gpc >>  8) & 0x3;
    }

    if (m_a429.format == eBCD)
    {
        // MSB BCD is only 3 bits not 4
        m_a429.lsb = (m_a429.msb) - (((m_a429.wordSize - 1) * 4) + 2);
    }
    else if ( m_a429.format == eDisc)
    {
        m_a429.lsb = m_a429.msb - (m_a429.wordSize - 1);
        if (m_a429.wordSize == 1)
        {
            m_a429.msb += 1;
        }
    }
    else // eBNR, eBNR1, eBNRU
    {
        m_a429.lsb = (m_a429.msb)-(m_a429.wordSize - 1);
    }

    // Parse General Purpose B parameter
    m_a429.dataLossTime = gpB;
}

//--------------------------------------------------------------------------------------------------
UINT32 ParamConverter::ExpectedSSM()
{
#define BNR_VALID_SSM   0x03 // -- this value will be packed into the 2 bit SSM field
#define BCD_VALID_SSM   0x00 // -- valid positive
#define DISC_VALID_SSM  0x00 //

    unsigned int expected = BNR_VALID_SSM;

    // Store the Default Valid SSM
    switch (m_a429.format)
    {
    case eDisc:
        switch (m_a429.discType)
        {
        case eDiscBNR:
            expected = BNR_VALID_SSM;
            break;
        case eDiscBCD:
            expected = BCD_VALID_SSM;
            break;
        case eDiscStandard:
        default:
            expected = DISC_VALID_SSM;
            break;
        }
        break;
    case eBCD:
        expected = BCD_VALID_SSM;
        break;
    case eBNR:
    case eBNR1:
    case eBNRU:
    default:
        expected = BNR_VALID_SSM;
        break;
    }
    return expected;
}

//-------------------------------------------------------------------------------------------------
// Function: Convert
// Description:
//
UINT32 ParamConverter::Convert(FLOAT32 value)
{
    UINT32 rawValue = 0;

    if (m_type == PARAM_FMT_A429)
    {
        rawValue = A429Converter(value);
    }
    else if (m_type == PARAM_FMT_BIN_A664 || m_type == PARAM_FMT_INTEGER)
    {
        UINT32 data = (UINT32)value;
        rawValue = data;
    }
    else if (m_type == PARAM_FMT_FLT_A664 || m_type == PARAM_FMT_FLOAT32)
    {
        // just pack the float : what about moving it around inside and big data set?
        UINT32 *data = (UINT32*)&value;
        rawValue = *data;
    }

    return rawValue;
}

//---------------------------------------------------------------------------------------------
// Function: A429Converter
// Description: Convert the value and pack it into a rawValue
//
UINT32 ParamConverter::A429Converter(float value)
{
    float bias = 0.5f;
    UINT32 rawValue = 0;

    if (m_a429.format == eBNR || m_a429.format == eBNR1)
    {
        if (m_scaleLsb > 0)  // based on 429 format
        {
            if (value >= (m_maxValue - m_scaleLsb))
            {
                value = m_maxValue - m_scaleLsb;
            }
            else if (value < -m_maxValue)
            {
                value = -m_maxValue;
            }

            if (value < 0.0f)
            {
                if (-value > m_scaleLsb)
                {
                    if (m_a429.format == eBNR1)
                    {
                        rawValue = A429_BNRPutSign(rawValue, 1);
                    }
                    else // eBNR
                    {
                        rawValue = SetBit(rawValue, (m_a429.msb + 1));
                    }
                    bias = -bias;
                }
                else
                {
                    value = 0.0f;
                }
            }
            else if (value < m_scaleLsb)
            {
                value = 0.0f;
            }

            m_data = UINT32( (value / m_scaleLsb) + bias);

            rawValue = A429_BNRPutData(rawValue, m_data, m_a429.msb, m_a429.lsb);
        }
    }
    else if (m_a429.format == eBNRU)
    {
        // unsigned binary value, bound it to the specified scale
        if (value < 0)
        {
            value = 0.0f;
        }
        else if (value >= (m_maxValue - m_scaleLsb))
        {
            value = m_maxValue - m_scaleLsb;
        }

        m_data = UINT32( (value / m_scaleLsb) + bias);
        rawValue = A429_BNRPutData(rawValue, m_data, m_a429.msb, m_a429.lsb);
    }
    else if (m_a429.format == eBCD)
    {
        UINT32 bcdShift = 0;
        UINT32 bcdValue = 0;

        if (value < 0.0f)
        {
            rawValue = A429_BCDPutSign(rawValue, 3);
            value = -value;
        }

        m_data = UINT32(value);

        for (int i=0; i < m_a429.wordSize; ++i)
        {
            UINT32 digit = m_data % 10;
            bcdValue |= digit << bcdShift;
            bcdShift += 4;
            m_data /= 10;
        }

        // Use the BNR packer as it just positions everything
        rawValue = A429_BNRPutData(rawValue, bcdValue, m_a429.msb, m_a429.lsb);
    }
    else if (m_a429.format == eDisc)
    {
        m_data = UINT32(value);
        rawValue = A429_BNRPutData(rawValue, m_data, m_a429.msb, m_a429.lsb);
    }

    rawValue |= m_a429.a429Template;

    if (m_type == PARAM_FMT_A429)
    {
        rawValue |= 0x80000000;
    }

    return rawValue;
}

//---------------------------------------------------------------------------------------------
void ParamConverter::SetSdi( INT32 value)
{
    m_a429.sdBits = value;
    m_a429.a429Template = A429_FldPutSDI( m_a429.a429Template, m_a429.sdBits);
}

//---------------------------------------------------------------------------------------------
void ParamConverter::SetSsm( INT32 value)
{
    m_a429.SSM = value;
    // we use BCD here as it only packs the SSM bits not the sign bit in BNR words
    m_a429.a429Template = A429_BCDPutSSM( m_a429.a429Template, m_a429.SSM);
}

//---------------------------------------------------------------------------------------------
// Function: SetLabel
// Description: Set the label for a A429 parameter
//
// Note: this function sets validity to allow the script to control sending the parameter to
// the ADRF.  If the label is changed from the value the parameter was initialized with the
// parameter is set to invalid so no updates occur via IOI.  When the label is restored to its
// original value validity is restored.
//
void ParamConverter::SetLabel( INT32 value)
{
    m_a429.label = value;
    // we use BCD here as it only packs the SSM bits not the sign bit in BNR words
    m_a429.a429Template = A429_FldPutLabel( m_a429.a429Template, value);

    //if (value == m_a429.label0)
    //{
    //    m_isValid = true;
    //}
    //else
    //{
    //    m_isValid = false;
    //}
}
