#pragma once

#include "alt_stdtypes.h"

#define RAND_MAX 0x7fff

// Linear Congruential Generator class
class RandGen
{
  public:
    RandGen(void);
    virtual void   Seed(UINT32 seed);
    virtual UINT32 Rand();

  protected:

    // Xn+1 = ((A * Xn) + C) % (M)
    unsigned int m_increment;   // Increment "C"
    unsigned int m_multiplier;  // Multiplier "A"
    unsigned int m_seed;        // Previous "Xn" and next "Xn+1" random value.
    unsigned int m_seedMask;
    unsigned int m_shiftCnt;   // shift to normalize seedMask
    unsigned long m_modulus;   // modulus "M"
};
