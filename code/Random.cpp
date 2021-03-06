#include "Random.h"

RandGen::RandGen(void)
: m_seed(0)
{
    m_multiplier = 1103515245;
    m_modulus    = 0x80000000; // pow(2,31)
    m_increment  = 12345;
    m_seedMask   = 0x7FFF0000; // bits 30..16 of seed are used
    m_shiftCnt   = 16;
}

void RandGen::Seed(UINT32 seed)
{
    m_seed = seed;
}

UINT32 RandGen::Rand()
{
    // Generate next seed
    m_seed = ((m_multiplier * m_seed) + m_increment); // % m_modulus;

    // Return random # as the truncated/shifted value in bits 30..16
    return (m_seed & m_seedMask) >> m_shiftCnt;
}
