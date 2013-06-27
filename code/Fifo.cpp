/******************************************************************************
            Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
               All Rights Reserved. Proprietary and Confidential.

    File:        Fifo.cpp

    Description: This file implements the FIFO

    VERSION
    $Revision: $  $Date: $

******************************************************************************/


/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <string.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "Fifo.h"

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




////////////////////////////////////////////////////////////////////////////////////////////////////
// FIFO Code
////////////////////////////////////////////////////////////////////////////////////////////////////
FIFO::FIFO()
{
    Reset();
}

//-----------------------------------------------------
void FIFO::Reset() {
    memset(buffer, 0, eFifoSize);
    writeCount = readCount = 0;
}

//-----------------------------------------------------
int FIFO::Push( const char* data, int size)
{
    char* src = (char*)data;
    int writeAt = writeCount % eFifoSize;
    int left = eFifoSize - writeAt;
    int buffered = writeCount - readCount;

    if ( (buffered + size) < eFifoSize)
    {
        // check for wrap around
        if ( left < size)
        {
            memcpy( &buffer[writeAt], src, left);
            writeAt = 0;
            writeCount += left;
            size -= left;
            src = &src[left];
        }
        else
        {
            left = 0;
        }

        // copy data or remaining
        memcpy( &buffer[writeAt], src, size);
        writeCount += size;

        return (size + left);
    }
    else
    {
        return 0;
    }
}

//-----------------------------------------------------
int FIFO::Pop( char* data, int size)
{
    char* dest = data;
    int readAt = readCount % eFifoSize;
    int left = eFifoSize - readAt;
    int buffered = writeCount - readCount;

    // can only return what we have
    if ( buffered < size)
    {
        size = buffered;
    }

    // check for wrap around
    if ( left < size)
    {
        memcpy( dest, &buffer[readAt], left);
        readAt = 0;
        readCount += left;
        size -= left;
        dest = &dest[left];
    }
    else
    {
        left = 0;
    }

    // copy data or remaining
    memcpy( dest, &buffer[readAt], size);
    readCount += size;

    return (size + left);
}

