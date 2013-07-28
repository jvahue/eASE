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
#include "Parameter.h"

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
UINT32 A429Converter();

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

//-------------------------------------------------------------------------------------------------
// Function: A429ParseGps
// Description: Parse the GPA data
//
ParamConverter::A429ParseGps()
{
    // Parse General Purpose A parameter
   m_rxChan  = (UINT8)GPA_RX_CHAN(m_gpa);
   m_format  = (ARINC_FORM) GPA_WORD_FORMAT(m_gpa);
   m_wordSize   = GPA_WORD_SIZE(m_gpa);
   m_SDBits     = GPA_SDI_DEFINITION(m_gpa);
   m_ignoreSDI  = (GPA_IGNORE_SDI(m_gpa) == TRUE) ? TRUE : FALSE;
   m_wordPos    = GPA_WORD_POSITION(m_gpa);
   m_discType   = (DISC_TYPE)GPA_PACKED_DISCRETE(m_gpa);
   m_validSSM   = GPA_SSM_FILTER(m_gpa);
   m_SDIAllCall = (GPA_ALL_CALL(m_gpa) == TRUE) ? TRUE : FALSE;
   
   // Parse General Purpose B parameter from 1 to 2^32 where lsb = 1 sec
   m_dataLossTime = m_gpb;
   
   // Parse General Purpose C parameter for label
   m_label = m_gpc;

   // Calculate the convert lsb used to create ENG value
   m_scaleLsb = (DISCRETE == m_format) ? 1.0f :
             ((FLOAT32)((FLOAT32) m_scale / (FLOAT32) (1 << m_wordSize)));

   if (m_validSSM == 0)
   {
      // Store the Default Valid SSM
      switch (m_format)
      {
         default:
         case BNR:
            m_validSSM = BNR_VALID_SSM;
            break;
         case DISCRETE:
            switch (m_discType)
            {
               case DISC_BNR:
                  m_validSSM = BNR_VALID_SSM;
                  break;
               case DISC_BCD:
                  m_validSSM = BCD_VALID_SSM;
                  break;
              case DISC_STANDARD:
              default:
                 m_validSSM = DISC_VALID_SSM;
                 break;
            }
            break;
         case BCD:
            m_validSSM = BCD_VALID_SSM;
            break;
      }
   }
}