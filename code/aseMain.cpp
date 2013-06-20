/******************************************************************************
            Copyright (C) 2013 Pratt & Whitney Engine Services, Inc.
               All Rights Reserved. Proprietary and Confidential.

    File:        aseMain.cpp

    Description: The main ASE process

    VERSION
    $Revision: $  $Date: $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <videobuf.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
#include "video.h"

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
int main(void)
{ // The function main() implements this process' main thread (sometimes referred to as its
  // primary thread).  That is, when Deos automatically creates the main thread, main() will
  // execute.

  // Declare some VideoStream objects that will allow this thread to output text to the target's
  // video memory (essentially, a printf-style debugging aid).  For each VideoStream object,
  // we specify four parameters (y, x, numY, numX), where:
  // y=starting row, x=starting column, numY=number of rows, and numX=number of columns
  //VideoStream videoOutTitle(14, 0, 1, 50);   // here is where we'll output the title string
  //VideoStream videoOut1(20, 40, 1, 40);     // here is where we'll output the system tick value
  debug_str_init();

  // These variables are used to hold values we want to output to video memory.
  const UNSIGNED32 systemTickTimeInHz = 1000000 / systemTickInMicroseconds();
  UNSIGNED32 *systemTickPtr;

  // Print the title message.
  debug_str(AseMain, 0, 0, "ASE ...System Tick Rate (Hz) = %d", systemTickTimeInHz);

  //videoOutTitle << "ASE ... System Tick Rate (Hz) = " << dec << systemTickTimeInHz << endl;

  // Grab the system tick pointer
  systemTickPtr = systemTickPointer();

  // The main thread goes into an infinite loop.
  while (1)
  {
    // Write the system tick value to video memory.
      debug_str(AseMain, 0, 40, "System Tick = %d", *systemTickPtr);
      //videoOut1 << "System Tick = " << dec << *systemTickPtr << endl;

    // Yield the CPU and wait until the next period to run again.
    waitUntilNextPeriod();
  }
}

