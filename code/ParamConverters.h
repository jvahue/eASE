#ifndef ParamConverters_h
#define ParamConverters_h

//-----------------------------------------------------------------------------
//            Copyright (C) 2013 Knowlogic Software Corp.
//         All Rights Reserved. Proprietary and Confidential.
//
// File: ParamConverters.h
// Description: Implements the ASE parameter converters.
//
//----------------------------------------------------------------------------/
// Compiler Specific Includes                                                -/
//----------------------------------------------------------------------------/

//----------------------------------------------------------------------------/
// Software Specific Includes                                                -/
//----------------------------------------------------------------------------/
#include "alt_stdtypes.h"
#include "AseCommon.h"

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define BNR_VALID_SSM               0x08
#define BCD_VALID_SSM               0x09
#define DISC_VALID_SSM              0x01


#define ARINC_MSG_PARITY_BIT        0x80000000UL
#define ARINC_MSG_SSM_BITS          0x60000000UL
#define ARINC_MSG_SIGN_BIT          0x10000000UL
#define ARINC_MSG_DATA_BITS         0x0FFFFF00UL
#define ARINC_MSG_SDI_BITS          0x00000300UL
#define ARINC_MSG_LABEL_BITS        0x000000FFUL

#define ARINC_MSG_SIGN_DATA_BITS    (ARINC_MSG_SIGN_BIT | ARINC_MSG_DATA_BITS)
#define ARINC_MSG_VALID_BIT         ARINC_MSG_PARITY_BIT

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/
enum ARINC_FORM
{
  BNR,
  BCD,
  DISCRETE,
  OTHER,
  END_OF_ARINC_FORMATS
} ;

enum DISC_TYPE
{
  DISC_STANDARD,
  DISC_BNR,
  DISC_BCD,
  END_OF_DISC_TYPES
} ;

struct ARINC429_WORD_INFO
{
   UINT8               label;
   // GPA Data
   UINT8               rxChan;       // Rx Chan 0,1,2 or 3 from FPGA
   ARINC_FORM          format;       // Format (BNR, BCD, Discrete)
   UINT8               wordSize;     // Arinc Word Size 1 - 32 bits
   UINT8               sdBits;       // SDBit definition
   BOOLEAN             ignoreSDI;    // Ignore SDI bits
   UINT8               wordPos;      // Arinc Word Start Pos within 32 bit word (Start 0)
   DISC_TYPE           discType;     // Discrete Type (Standard, BNR, BCD)
   UINT8               validSSM;     // Valid SSM for Word
                                     //   NOTE: Valid SSM is directly related to the
                                     //         FailureCondition[] array index.
                                     //   (0 = Invalid 1 = Valid)
                                     //   bit 0 = SSM '00'
                                     //   bit 1 = SSM '01'
                                     //   bit 2 = SSM '10'
                                     //   bit 3 = SSM '11'
   BOOLEAN             sdiAllCall;   // SDI All Call Mode
   FLOAT32             convert_lsb;  //   cfg.scale / sizeofWord to get scaling
   // GPB Data
   UINT32              dataLossTime; // Data Loss Timeout in milliseconds

   // Index into A429_SRC_DATA
   UINT16              index;

   UINT32              rxTime;  // Rx Time indicate new update from ParamSrcA429.  Note
                                //   this val is used differently then the
                                //   PARAM_DATA.rxTime, to support SSM Filter processing.
                                //   When new update is Rx from ParamSrcA429, this rxTime
                                //   is updated and is use as flag wether to call ioi_read()
                                //   The Param could still be invalid due to not passing
                                //   SSM filtering, thus PARAM_DATA.rxTime is updated to
                                //   identify validity of Param.

};

//==================================================================================================
class ParamConverter
{
public:
    ParamConverter();
    void Reset(PARAM_FMT_ENUM fmt, UINT32 gpa, UINT32 gpb, UINT32 gpc, UINT32 scale);
    virtual UINT32 Convert(FLOAT32 value);
    
protected:
    UINT32  m_gpa;
    UINT32  m_gpb;
    UINT32  m_gpc;
    PARAM_FMT_ENUM m_fmt; 
    UINT32  m_scale;       // the current value for the parameter
    FLOAT32 m_scaleLsb;    // the current value for the parameter
    
    // A429 Parameter Attributes
    ARINC429_WORD_INFO m_a429;

    void A429ParseGps();
    UINT32 A429Converter();

    //UINT32 A664Converter();

};

#endif
