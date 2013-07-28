#ifndef ParamConverters_h
#define ParamConverters_h

// File: ParamConverters.h

#include "alt_stdtypes.h"
#include "AseCommon.h"

enum {
  PARAM_FMT_NONE=0,  // FMT not specified
  PARAM_FMT_BIN_A664,// FMT is IDL binary
  PARAM_FMT_FLT_A664,// FMT is IDL floating-point
  PARAM_FMT_A429,    // FMT is A429 standard data
//  PARAM_FMT_HS_ELECT,// FMT is HS Electric System data (not needed, use BIN_664)
  PARAM_FMT_MAX
} PARAM_FMT_ENUM;



/******************************************************************************
* Description: Implements the ASE parameter converters.
*
*
*/
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
    UINT32  m_gpb;
    PARAM_FMT_ENUM m_fmt; 
    UINT32  m_scale;       // the current value for the parameter
    FLOAT32 m_scaleLsb;    // the current value for the parameter
    
    UINT32 A429Converter();
    UINT32 A664Converter();    
};

#endif
