/******************************************************************************
            Copyright (C) 2013 Knowlogic Software Corp.
         All Rights Reserved. Proprietary and Confidential.

    File: ParamConverters.cpp

    Description: The file implements the Parameter object processing

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "ParamConverters.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
#define GPA_RX_CHAN(gpa)          ((gpa >>  0) & 0x03)
#define GPA_WORD_FORMAT(gpa)      ((gpa >>  2) & 0x03)
#define GPA_WORD_SIZE(gpa)        ((gpa >>  5) & 0x1F)
#define GPA_SDI_DEFINITION(gpa)   ((gpa >> 10) & 0x03)
#define GPA_IGNORE_SDI(gpa)       ((gpa >> 12) & 0x01)
#define GPA_WORD_POSITION(gpa)    ((gpa >> 13) & 0x1F)
#define GPA_PACKED_DISCRETE(gpa)  ((gpa >> 18) & 0x03)
#define GPA_SSM_FILTER(gpa)       ((gpa >> 20) & 0x0F)
#define GPA_ALL_CALL(gpa)         ((gpa >> 24) & 0x01)

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
    : m_gpa(0)
    , m_gpb(0)
    , m_gpc(0)
    , m_fmt(PARAM_FMT_NONE)
    , m_scale(0.0f)       // the current value for the parameter
    , m_scaleLsb(0.0f)    // the current value for the parameter
{
}


//-------------------------------------------------------------------------------------------------
// Function: CheckCmd
// Description: Detemrine if the current command is intended for this thread
//
void ParamConverter::Reset( PARAM_FMT_ENUM fmt, UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale)
{
    m_gpa = gpa;
    m_gpb = gpb;
    m_gpc = gpc;
    m_fmt = fmt;
    m_scale = scale;
    
    if (m_fmt == PARAM_FMT_A429)
    {
        A429ParseGps();
    }
    else if (m_fmt == PARAM_FMT_BIN_A664)
    {
        
    }
    else if (m_fmt == PARAM_FMT_FLT_A664)
    {
        
    }
}    

UINT32 ParamConverter::Convert(FLOAT32 value)
{
    // based on the format, call a converter
    return 0;
}


//-------------------------------------------------------------------------------------------------
// Function: A429ParseGps
// Description: Parse the GPA data
//
void ParamConverter::A429ParseGps()
{
    // Parse General Purpose A parameter
   m_a429.rxChan  = (UINT8)GPA_RX_CHAN(m_gpa);
   m_a429.format  = (ARINC_FORM) GPA_WORD_FORMAT(m_gpa);
   m_a429.wordSize   = GPA_WORD_SIZE(m_gpa);
   m_a429.sdBits     = GPA_SDI_DEFINITION(m_gpa);
   m_a429.ignoreSDI  = (GPA_IGNORE_SDI(m_gpa) == TRUE) ? TRUE : FALSE;
   m_a429.wordPos    = GPA_WORD_POSITION(m_gpa);
   m_a429.discType   = (DISC_TYPE)GPA_PACKED_DISCRETE(m_gpa);
   m_a429.validSSM   = GPA_SSM_FILTER(m_gpa);
   m_a429.sdiAllCall = (GPA_ALL_CALL(m_gpa) == TRUE) ? TRUE : FALSE;
   
   // Parse General Purpose B parameter from 1 to 2^32 where lsb = 1 sec
   m_a429.dataLossTime = m_gpb;
   
   // Parse General Purpose C parameter for label
   m_a429.label = m_gpc;

   // Calculate the convert lsb used to create ENG value
   m_scaleLsb = (DISCRETE == m_a429.format) ? 1.0f :
                ((FLOAT32)((FLOAT32) m_scale / (FLOAT32) (1 << m_a429.wordSize)));

   if (m_a429.validSSM == 0)
   {
      // Store the Default Valid SSM
      switch (m_a429.format)
      {
         default:
         case BNR:
            m_a429.validSSM = BNR_VALID_SSM;
            break;
         case DISCRETE:
            switch (m_a429.discType)
            {
               case DISC_BNR:
                  m_a429.validSSM = BNR_VALID_SSM;
                  break;
               case DISC_BCD:
                  m_a429.validSSM = BCD_VALID_SSM;
                  break;
              case DISC_STANDARD:
              default:
                 m_a429.validSSM = DISC_VALID_SSM;
                 break;
            }
            break;
         case BCD:
            m_a429.validSSM = BCD_VALID_SSM;
            break;
      }
   }
}

UINT32 ParamConverter::A429Converter()
{
    return 0;
}
