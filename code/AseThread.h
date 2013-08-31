#ifndef ASETHREAD_H
#define ASETHREAD_H
//
// File: AseThread.h

// Description: Implements the socket connection for the low level driver code
// This is the client side of the socket
//
// The AseSocket creates two threads that receive and transmit data.  These threads
// fill in data within this object.
//
// The virtual functions are:
// Run - Prepare the thread object for execution, init data, etc, calls Launch to start the thread running
// Process - Performs the specific processing needs of the thread.
//
// Includes OS
//

// Includes PWC
#include "alt_stdtypes.h"
#include "procapi.h"
#include "AseCommon.h"

// Fwd decls
class CmdObj;

class AseThread
{
public:
    enum AseThreadState {
        eNotCreated,
        eStop,
        eRun,
        eSusp,
        eComplete,
        eError};

    AseThread();

    // Thread control
    virtual void Run() {}
    virtual void Run(AseCommon* pCommon)
    {
        m_pCommon = pCommon;
        Run();
    };

    // Accessors
    AseThreadState GetRunState();

protected:
    // Obj refs
    // Thread Attribs.
    thread_handle_t m_hThread;
    AseThreadState  m_state;
    AseCommon*      m_pCommon;

    void Launch(const CHAR* name, const CHAR* tName);

    // Methods
    virtual void Process() {};

    static void ThreadFunc( DWORD obj)
    {
      ((AseThread*)obj)->Process();
    }
};
#endif
