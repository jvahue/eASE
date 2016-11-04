#ifndef VIDEO_H_
#define VIDEO_H_

/*
* video.h
*
*  Created on: Mar 18, 2013
*      Author: pw46601
*
*  Video.h/cpp adds multiple debug screens that allow printf type debug output to
*
*
*  Do the following
*  to add this feature:
*
*  1. Include video.h in the files you wish to have debug output for
*      NOTE: Consider wrapping the include and calls to debug_str and debug_str_init in pre-compiler defines
*            so they can be easily removed for production.  DEoS video library is Level-E
*      Ensure the pd.xml defines enough memory objects in the quota, each video screen
*      requires one memory object.
*
*	2. Call debug_str_init with a video memory object name
*
*	3. In the system video window hit <ESC>, type "am <video memory object name>", type "g"
*
*/

#undef VID_DEF
#define VID_DEF(Id,Name,Scroll)
// ----- If the list changes update eFAST.py in the ePySte -----
//                      Text      Scroll
//     Id               Name      Mode
#define VID_DEF_LIST \
    VID_DEF(VID_SYS,       "",        FALSE),\
    VID_DEF(AseMain,       "AseMain", FALSE),\
    VID_DEF(CmProc,        "CmProc",  FALSE),\
    VID_DEF(Ioi,           "Ioi",     FALSE),\
    VID_DEF(Params,        "Params",  FALSE),\
    VID_DEF(Static,        "Static",  FALSE),\
    // ----- If the list changes update eFAST.py in the ePySte -----

#undef  VID_DEF
#define VID_DEF(Id,Name,Scroll) Id
typedef enum {
    VID_DEF_LIST
    VID_MAX
}VID_DEFS;

#if defined(__cplusplus)

extern "C" {

    void debug_str (VID_DEFS screen, int row, int col, const CHAR* str, ... );
    void debug_str1 (VID_DEFS screen, int row, int col, CHAR* str);
    void debug_str_init(void);
    void clearRow(VID_DEFS screen,  int row);

    extern VID_DEFS videoRedirect;

}

//void debug_str (VID_DEFS screen, int row, int col, const CHAR* str, ... );



#endif
#endif /* VIDEO_H_ */
