/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: ParamConverters.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <math.h>
#include <stdlib.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
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
    : m_isValid(false)
    , m_gpa(0)
    , m_gpb(0)
    , m_gpc(0)
    , m_type(PARAM_FMT_NONE)
    , m_scale(0)          // the current value for the parameter
    , m_maxValue(0.0f)
    , m_scaleLsb(0.0f)    // the current value for the parameter
    , m_data(0)
{
}

//-------------------------------------------------------------------------------------------------
// Function: Convert
// Description:
//
UINT32 ParamConverter::Convert(FLOAT32 value)
{
    UINT32 rawValue;

    if (m_type == PARAM_FMT_A429)
    {
        rawValue = A429Converter(value);
    }

    return rawValue;
}

//-------------------------------------------------------------------------------------------------
// Function: A429Converter
// Description: Convert the value and pack it into a rawValue
//
UINT32 ParamConverter::A429Converter(float value)
{
    UINT32 rawValue = 0;

    if (m_a429.format == eBNR)
    {
        if (m_scaleLsb > 0)  // based on 429 format
        {
            if ( value >= m_maxValue)
            {
                value = m_maxValue - m_scaleLsb;
            }
            else if (value < -m_maxValue)
            {
                value = -m_maxValue;
            }

            m_data = UINT32(value / m_scaleLsb);

            rawValue = A429_BNRPutData(rawValue, m_data, m_a429.msb, m_a429.lsb);
            if (value < 0.0f)
            {
                rawValue = A429_BNRPutSign(rawValue, 1);
            }
        }
    }
    else if (m_a429.format == eDisc)
    {
        m_data = UINT32(value);
        rawValue = A429_BNRPutData(rawValue, m_data, m_a429.msb, m_a429.lsb);
    }
    // TODO: other formats

    rawValue |= m_a429.a429Template;

    return rawValue;
}

//-------------------------------------------------------------------------------------------------
// Function: Reset
// Description:
//
void ParamConverter::Reset( PARAM_FMT_ENUM fmt, UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale)
{
    UINT32 rawLabel;

    m_gpa = gpa;
    m_gpb = gpb;
    m_gpc = gpc;
    m_type = fmt;
    m_scale = scale;
    m_maxValue = FLOAT32(scale);
    
    if (m_type == PARAM_FMT_A429)
    {
        A429ParseGps();

        if (m_a429.format == eBNR)
        {
            m_scaleLsb = float(scale) / pow(2.0f, float(m_a429.wordSize));
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

        // set the SSM bits based on which one(s) are valid.
        if (m_a429.validSSM == 0)
        {
            m_a429.SSM = ExpectedSSM();
        }
        else
        {
            bool ssmSet = false;
            int mask = 8;
            while (!ssmSet && mask != 0)
            {
                if ( m_a429.validSSM & mask)
                {
                    m_a429.SSM = mask >> 1;
                    m_a429.SSM = min(3, m_a429.SSM);
                    ssmSet = true;
                }
                else
                {
                    mask >>= 1;
                }
            }
        }

        SetSsm( m_a429.SSM);

    }
    else if (m_type == PARAM_FMT_BIN_A664)
    {
        
    }
    else if (m_type == PARAM_FMT_FLT_A664)
    {
        
    }
}    

//--------------------------------------------------------------------------------------------------
void ParamConverter::A429ParseGps()
{
    UINT32 gpA = m_gpa;
    UINT32 gpB = m_gpb;

    m_a429.channel    = gpA & 0x3;
    m_a429.format     = A429WordFormat((gpA >> 2)  & 0x03);
    m_a429.discType   = A429DiscretTypes((gpA >> 18) & 0x03);
    m_a429.wordSize   = (gpA >> 5)  & 0x1F;
    m_a429.sdBits     = (gpA >> 10) & 0x03;
    m_a429.ignoreSDI  = (gpA >> 12) & 0x01;
    m_a429.msb        = ((gpA >> 13) & 0x1F);
    m_a429.validSSM   = (gpA >> 20) & 0x0F;
    m_a429.sdiAllCall = (gpA >> 24) & 0x01;
    m_a429.label      = m_gpc;

    if (m_a429.format == eBCD)
    {
        // MSB BCD is only 3 bits not 4
        m_a429.lsb = (m_a429.msb)-(((m_a429.wordSize-1)*4)+2);
    }
    else if ( m_a429.format == eDisc)
    {
        m_a429.lsb = m_a429.msb - (m_a429.wordSize-1);
    }
    else // eBNR
    {
        m_a429.lsb = (m_a429.msb)-(m_a429.wordSize-1);
    }

    // Parse General Purpose B parameter
    m_a429.dataLossTime = gpB;
}

//--------------------------------------------------------------------------------------------------
UINT32 ParamConverter::ExpectedSSM()
{
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
    default:
        expected = BNR_VALID_SSM;
        break;
    }
    return expected;
}

//--------------------------------------------------------------------------------------------------
void ParamConverter::SetSdi( INT32 value)
{
    m_a429.sdBits = value;
    m_a429.a429Template = A429_FldPutSDI( m_a429.a429Template, m_a429.sdBits);
}

//--------------------------------------------------------------------------------------------------
void ParamConverter::SetSsm( INT32 value)
{
    m_a429.SSM = value;
    // we use BCD here as it only packs the SSM bits not the sign bit in BNR words
    m_a429.a429Template = A429_BCDPutSSM( m_a429.a429Template, m_a429.SSM);
}

//-------------------------------------------------------------------------------------------------
// Function: SetLabel
// Description: Set the label for a A429 parameter
//
// Note: this function sets validity to allow the script to control sending the parameter to the
// ADRF.  If the label is changed from the value the parameter was initialized with the parameter
// is set to invlaid so no updates occur via IOI.  When the label is restored to its original
// value validity is restored.
//
void ParamConverter::SetLabel( INT32 value)
{
    m_a429.label = value;
    // we use BCD here as it only packs the SSM bits not the sign bit in BNR words
    m_a429.a429Template = A429_FldPutLabel( m_a429.a429Template, value);

    if (value == m_a429.label0)
    {
        m_isValid = true;
    }
    else
    {
        m_isValid = false;
    }
}