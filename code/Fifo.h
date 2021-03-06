#ifndef Fifo_h
#define Fifo_h
/******************************************************************************
Copyright (C) 2013-2016 Knowlogic Software Corp.
All Rights Reserved. Proprietary and Confidential.

File:        aseMain.cpp

Description: The main ASE process

VERSION
$Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/

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
struct FIFO
{
    enum FifoConstants {eFifoSize = 4096}; // 1024->2048
    char buffer[eFifoSize];
    int writeCount;
    int readCount;

    //-----------------------------------------------------
    FIFO();
    void Reset();
    int Push( const char* data, int size);
    int Pop( char* data, int size);
    inline bool IsEmpty() const {return (readCount == writeCount);}
    inline int Used() const {return (writeCount - readCount);}
};

#endif
