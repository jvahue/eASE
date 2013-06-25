#ifndef ASETHREAD_H
#define ASETHREAD_H
//
// File: AseThread.h

// Description: Implements the socket connection for the low level driver code
// This is the client side of the socket
//
// The AseSocket creates two threads that receive and transmit data.  These threads
// fill in data within this object.  The main functions are:
// Create - Spawn threads
// Connect - attempt to get a connection
// Read - read data from the thread - standard command msgs from the PySTE are supported
// Send - send data
// Close - close the connection
//
// Includes OS
//

// Includes PWC
#include "alt_stdtypes.h"
#include "procapi.h"

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
    virtual void Run();

    
    // Accessors
    AseThreadState GetRunState();
    
protected:
    // Obj refs
    // Thread Attribs.
    thread_handle_t m_hThread;
    AseThreadState  m_state;
    
    virtual void Create(const CHAR* name, const CHAR* tName);
    virtual void Create() {};

    // Methods
    virtual void Process() {};
    
    static void ThreadFunc( DWORD obj)
    {
      ((AseThread*)obj)->Process();
    }
};
#endif


