/******************************************************************************
              Copyright (C) 2012-2013 Pratt & Whitney Engine Services, Inc.
                 All Rights Reserved. Proprietary and Confidential.

    File:        ParamSrcHMU.c

    Description: Read and decode parameters from the 664 bus through DEoS IoI

    VERSION
      $Revision: 3 $  $Date: 6/19/13 11:56a $

******************************************************************************/

/*****************************************************************************/
/* Compiler Specific Includes                                                */
/*****************************************************************************/
#include <deos.h>
#include <videobuf.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*****************************************************************************/
/* Software Specific Includes                                                */
/*****************************************************************************/
extern "C" {
#include "alt_basic.h"
#include "alt_stdtypes.h"
#include "alt_Time.h"
}
#include "video.h"

/*****************************************************************************/
/* Local Defines                                                             */
/*****************************************************************************/
/*****************************************************************************/
/* Local Typedefs                                                            */
/*****************************************************************************/
typedef struct
{
	CHAR smo_name[32];
	void* smo;
	ipcMemoryObjectHandle mo;
	ipcAttachedMemoryObjectHandle amo;
	VideoStream vid_stream;
	BOOLEAN scroll;
}DEBUG_SCREEN_DATA;

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/
#undef VID_DEF
#define VID_DEF(Id,Name,Scroll) {Name,0,0,0,VideoStream(),Scroll}
DEBUG_SCREEN_DATA m_screens[] = {VID_DEF_LIST};

VID_DEFS videoRedirect;
char blankLine[CGA_NUM_COLS+1]; // blanks the video line

/*****************************************************************************/
/* Local Function Prototypes                                                 */
/*****************************************************************************/
/*****************************************************************************/
/* Constant Data                                                             */
/*****************************************************************************/
/*****************************************************************************/
/* Public Functions                                                          */
/*****************************************************************************/

/******************************************************************************
 * Function:    debug_str_init
 *
 * Description: Setup a new debug screen. Create a new globally accessable
 *              shared memory object and initialize it as video memory.
 *
 * Parameters:  none
 *
 * Returns:     none
 *
 * Notes:
 *
 *****************************************************************************/
void debug_str_init(void)
{
    ipcStatus is;
    INT32 i;

    videoRedirect = VID_SYS;
    memset(blankLine, 0x20, sizeof(blankLine));
    blankLine[80] = '\0';

    for(i = 0; i < VID_MAX; i++)
    {
      //if not default screen, create a new screen

      if(strcmp(m_screens[i].smo_name,"") != 0)
      {
        if((is= createMemoryObject(m_screens[i].smo_name,4096,FALSE,
                               &m_screens[i].mo))!=ipcValid){}
        else if((is = grantMemoryObjectAccess(m_screens[i].mo, currentProcessHandle(), TRUE,
                                           readWriteDeleteAccess)) !=ipcValid){}

        else if((is = attachMemoryObject(m_screens[i].smo_name, m_screens[i].mo, readWriteAccess,
                                           &m_screens[i].smo, 0x1000, 0,
                                           &m_screens[i].amo)) !=ipcValid){}
        else
        {
          initializeVideoMemory(m_screens[i].smo);
          m_screens[i].vid_stream.setViewPortAddress((unsigned)m_screens[i].smo);
          m_screens[i].vid_stream.getViewPort().setScroll(m_screens[i].scroll);
          m_screens[i].vid_stream <<  "Main Vid Success: " << dec << is << "                   " <<  endl;
        }
      }
    }
}



/******************************************************************************
 * Function:    debug_str
 *
 * Description:
 *
 * Parameters:  [in] screen: Screen enum
 *              [in] row,col: Initial cursor position, only for screens with no
 *                            scroll option set.
 *              [in] str:    Formatted string to write
 *              [in] ...:    varargs to use with formatted string.
 *
 * Returns:     none
 *
 * Notes:
 *
 *****************************************************************************/
void debug_str(VID_DEFS screen, int row, int col, const CHAR* str, ... )
{
static VID_DEFS redirectZ1 = VID_SYS;

    CHAR buf[1024];

	va_list args;

	va_start(args,str);

	vsprintf( buf, str, args);

	va_end(args);

	buf[80] = '\0';

	if (screen == videoRedirect)
	{
	    // redirect to VID_SYS
	    screen = VID_SYS;

	    if (redirectZ1 != videoRedirect)
	    {
	        redirectZ1 = videoRedirect;

	        // clear VID_SYS if we just changed
	        for (UINT32 i=0; i < CGA_NUM_ROWS; ++i)
	        {
	            clearRow(screen, i);
	        }
	    }
	}

	if(!m_screens[screen].scroll)
	{
		m_screens[screen].vid_stream.getViewPort().cursor().put(row,col);
    }

	m_screens[screen].vid_stream << buf;

}

/******************************************************************************
 * Function:    clearRow
 *
 * Description: clear a row of a display
 *
 * Parameters:  [in] screen: Screen enum
 *              [in] row,col: Initial cursor position, only for screens with no
 *                            scroll option set.
 *              [in] str:    Formatted string to write
 *              [in] ...:    varargs to use with formatted string.
 *
 * Returns:     none
 *
 * Notes:
 *
 *****************************************************************************/
void clearRow(VID_DEFS screen,  int row)
{
    m_screens[screen].vid_stream.getViewPort().cursor().put(row,0);
    m_screens[screen].vid_stream << blankLine;
}
