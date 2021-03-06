#ifndef ParamConverters_h
#define ParamConverters_h

//-----------------------------------------------------------------------------
//          Copyright (C) 2013-2017 Knowlogic Software Corp.
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

//----------------------------------------------------------------------------/
// Local Defines                                                             -/
//----------------------------------------------------------------------------/
#define ARINC_MSG_PARITY_BIT        0x80000000UL
#define ARINC_MSG_SSM_BITS          0x60000000UL
#define ARINC_MSG_SIGN_BIT          0x10000000UL
#define ARINC_MSG_DATA_BITS         0x0FFFFF00UL
#define ARINC_MSG_SDI_BITS          0x00000300UL
#define ARINC_MSG_LABEL_BITS        0x000000FFUL

#define ARINC_MSG_SIGN_DATA_BITS    (ARINC_MSG_SIGN_BIT | ARINC_MSG_DATA_BITS)
#define ARINC_MSG_VALID_BIT         ARINC_MSG_PARITY_BIT

#define A429_MASK(s) ((1 << (s)) - 1)
#define A429_FIELD(m,l) (A429_MASK((m+1)-(l)) << (l))

#define A429_FldPutLabel(d,l) ((d & 0xFFFFFF00) | (l & 0xFF))
#define A429_BNRPutSign(d,s)  ((d & 0x6FFFFFFF) | ((s & 1) << 28))
#define A429_BCDPutSign(d,s)  ((d & 0x1FFFFFFF) | ((s & 3) << 29))
#define A429_FldPutData(d,v)  (d | (v << 10))

#define A429_BNRPutData(d,v,m,l) ((d & ~A429_FIELD((m),(l))) | ((v & A429_FIELD((m)-(l),0)) << (l)))

#define A429_BNRGetData(d,m,l) ((d >> l) & A429_FIELD( m,l))

#define A429_BCDPutSSM(d,s) ((d & 0x1fffffff) | ((s & 3) << 29))
#define A429_FldPutSDI(d,s) ((d & ~0x300) | ((s & 3) << 8))

#define SetBit(v, b) ((v) | (1 << b))

//----------------------------------------------------------------------------/
// Local Typedefs                                                            -/
//----------------------------------------------------------------------------/
enum A429WordFormat {
    eBNR   = 0x00,  // contiguous sign and data BNR
    eBNR1  = 0x01,  // sign bit #28, data wherever
    eBNRU  = 0x02,  // unsigned BNR
    eBCD   = 0x04,  // BCD value - assumed in upper bits of word
    eDisc  = 0x08,  // Discrete bits - anywhere
    eOther = 0x0C   // Not in Use
};

enum A429DiscretTypes {
    eDiscStandard,
    eDiscBNR,
    eDiscBCD
};

struct ARINC429_WORD_INFO
{
    // GPC Data
    UINT8               label;
    UINT8               label0;      // the label the parameter was initialized with

    // GPA Data
    UINT8               channel;      // Rx Chan 0,1,2 or 3 from FPGA
    A429WordFormat      format;       // Format (BNR, BCD, Discrete)
    A429DiscretTypes    discType;     // Discrete Type (Standard, BNR, BCD)
    UINT8               wordSize;     // Arinc Word Size 1 - 32 bits
    UINT8               wordPos;      // Arinc Word Start Pos within 32 bit word (Start 0)
    UINT8               lsb;
    UINT8               msb;

    UINT8               validSSM;     // Valid SSM for Word
    //   NOTE: Valid SSM is directly related to the
    //         FailureCondition[] array index.
    //   (0 = Invalid 1 = Valid)
    //   bit 0 = SSM '00'
    //   bit 1 = SSM '01'
    //   bit 2 = SSM '10'
    //   bit 3 = SSM '11'
    UINT8               SSM;

    BOOLEAN             sdiAllCall;   // SDI All Call Mode
    BOOLEAN             ignoreSDI;    // Ignore SDI bits
    UINT8               sdBits;       // SDBit definition

    UINT32              a429Template; // ssm, sdi, label only

    // GPB Data
    UINT32              dataLossTime; // Data Loss Timeout in milliseconds

};

// Parameter Configuration info
struct ParamCfg {
    ParameterName name;
    UINT32 index;
    UINT32 masterId;
    UINT32 rateHz;
    PARAM_SRC_ENUM src;
    PARAM_FMT_ENUM fmt;
    FLOAT32 scale;
    UINT32 gpa;
    UINT32 gpb;
    UINT32 gpc;
    UINT32 gpe;
};

//=============================================================================================
class ParamConverter
{
public:
    ParamConverter();
    void Reset();
    void Init(ParamCfg* paramInfo);
    virtual UINT32 Convert(FLOAT32 value);
    void SetIoiName();

    bool m_isValid;        // is the parameter active and setup was successful
    bool m_isSigned;       // is the parameter signed
    bool m_is2Comp;        // is the parameter in 2's complement or sign/magnitude
    UINT32  m_masterId;
    UINT32  m_gpa;
    UINT32  m_gpb;
    UINT32  m_gpc;
    UINT32  m_gpe;
    UINT32  m_totalBits;
    PARAM_SRC_ENUM m_src;
    PARAM_FMT_ENUM m_type;
    FLOAT32 m_maxValue;    // what is the cfg scale value set to
    FLOAT32 m_scaleLsb;    // the effective value of each bit for the scale(m_maxValue)
    UINT32  m_data;        // the current value for the parameter
    ParameterName m_ioiName;

    // A429 Parameter Attributes
    ARINC429_WORD_INFO m_a429;

    UINT16 a664Offset; // what is the bit offset into the IDL
    UINT16 a664Size;   // how big is the A664 field

    void A429ParseGps();
    UINT32 ExpectedSSM();
    UINT32 A429Converter(float value);
    void SetSdi( INT32 value);
    void SetSsm( INT32 value);
    void SetLabel( INT32 value);
    void SetIoiA429Name();

    //UINT32 A664Converter();
    void SetIoiA664Name();

};

#endif
