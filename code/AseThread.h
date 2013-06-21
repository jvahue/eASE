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
	enum AseThreadState	{ eNotCreated, eStop, eRun, eSusp};
	AseThread();
	//virtual ~AseThread();

	// Thread control
	void Run();
	void Suspend();
	void Stop();

	// Accessors
	AseThreadState GetRunState();
	virtual void Create () {};

  protected:
    // Obj refs
	int* m_pCmd;  // make this a ptr to CmdObj

	// Thread Attribs.
	thread_handle_t m_hThread;
	AseThreadState  m_state;

	// Methods
	virtual void Process() {};

	static void ThreadFunc( DWORD obj)
    {
      ((AseThread*)obj)->Process();
    }

  private:
};

/**********************************************************************/
class FxProc : public AseThread
{
	protected:
	  // Methods
	  virtual void Process(); // override the AseThread::Process

public:
	  virtual void Create();  // override the AseThread::Create
	  UNSIGNED32 m_ticks;
};
