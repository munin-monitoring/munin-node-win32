
#ifndef _JC_THREAD_H_
#define _JC_THREAD_H_

// wrapper for whatever critical section we have
class JCCritSec {
public:
	JCCritSec() {
		InitializeCriticalSection(&m_CritSec);
	};
	~JCCritSec() {
		DeleteCriticalSection(&m_CritSec);
	};
	void Lock() {
		EnterCriticalSection(&m_CritSec);
	};
	void Unlock() {
		LeaveCriticalSection(&m_CritSec);
	};

protected:
	CRITICAL_SECTION m_CritSec;
private:
	// Make copy constructor and assignment operator inaccessible
	// Copying a critical section is a bad idea :P
	JCCritSec(const JCCritSec &refCritSec) { };
	JCCritSec &operator=(const JCCritSec &refCritSec) { return *this; };
};

// Auto-Lock class
class JCAutoLockCritSec {
public:
	JCAutoLockCritSec(JCCritSec *lockCritSec) {
		m_CritSec = lockCritSec;
		m_CritSec->Lock();
	};
	~JCAutoLockCritSec() {
		m_CritSec->Unlock();
	};

protected:
	JCCritSec *m_CritSec;
private:
	// Make copy constructor and assignment operator inaccessible
	// Copying a critical section lock is a bad idea :P
	JCAutoLockCritSec(const JCAutoLockCritSec &refCritSec) { assert(!"We should never ever be here!"); };
	JCAutoLockCritSec &operator=(const JCAutoLockCritSec &refCritSec) { assert(!"We should never ever be here!"); return *this; };
};

class JCAutoInterlockedIncrement {
public:
  JCAutoInterlockedIncrement(long *pLong) {
    m_pLong = pLong;
  };
  ~JCAutoInterlockedIncrement() {
    InterlockedIncrement(m_pLong);
  };

protected:
  long *m_pLong;
};

class JCEvent {
public:
  JCEvent(bool bManualReset = false, bool bInitialState = false, LPCTSTR szName = NULL) {
    m_Event = CreateEvent(NULL, bManualReset ? TRUE : FALSE, bInitialState ? TRUE : FALSE, szName);
  }
  ~JCEvent() {
    CloseHandle(m_Event);
  }

  bool Set() {
    return (SetEvent(m_Event) == TRUE);
  }
  bool Reset() {
    return (ResetEvent(m_Event) == TRUE);
  }
  int Wait(int iMilliseconds) {
    return (int)WaitForSingleObject(m_Event, (DWORD)iMilliseconds);
  }
protected:
  HANDLE m_Event;
};

class JCRefCount {
public:
    JCRefCount() {
        m_RefCount = 0;
    }
    virtual ~JCRefCount() {
    }

	long AddRef() {
		JCAutoLockCritSec refCountLock(&m_RefCountCrit);		
		return m_RefCount++;
	}
	long RemoveRef() {
    {
		  JCAutoLockCritSec refCountLock(&m_RefCountCrit);	

		  if (--m_RefCount)
			  return m_RefCount;
    }
		delete this;

		return 0L;
	}

protected:	
	JCCritSec m_RefCountCrit;
	long m_RefCount;
};

class JCThread {
private:
	static void __cdecl JCThreadFunction(void *threadData) {		
		JCThread *threadObj = (JCThread *)threadData;
		// Check thread pointer
		assert(threadObj != NULL);

		threadObj->JCThread_AddRef();

		threadObj->m_StartedCrit.Lock();
		threadObj->m_bStarted = true;
		threadObj->m_StartedCrit.Unlock();
		threadObj->Entry();	

    threadObj->Stop();
		threadObj->JCThread_RemoveRef();
		
		_endthread();
	};
public:
	JCThread() { 
		m_hThread = NULL;
		m_State = JCThread_State_Waiting;
		m_bStarted = false;
		m_RefCount = 0;
	};
	virtual ~JCThread() { 
		/*if (GetState() == JCThread_State_Running) {
			// We need to stop the thread before deleting it
			Stop();
		}*/
	};

	void Run() {
		JCThread_AddRef();
		m_StateCrit.Lock();
		m_State = JCThread_State_Running;
		m_StateCrit.Unlock();
		m_hThread = (HANDLE)_beginthread(JCThreadFunction, 0, (void *)this);
		// Wait for the thread to start
		while (!TestDestroy()) {
			m_StartedCrit.Lock();
			if (m_bStarted) {
				m_StartedCrit.Unlock();
				break;
			}
			m_StartedCrit.Unlock();
			Sleep(50);
		}
		JCThread_RemoveRef();
	}
	void Stop() {
		m_StateCrit.Lock();
		m_State = JCThread_State_Stopped;
		m_StateCrit.Unlock();		
	}
	void Pause() {
		JCAutoLockCritSec stateLock(&m_StateCrit);
		if (m_State == JCThread_State_Running) {
			if (SuspendThread(m_hThread) != 0xFFFFFFFF)
				m_State = JCThread_State_Paused;
		}
	}
	void Resume() {
		JCAutoLockCritSec stateLock(&m_StateCrit);
		if (m_State == JCThread_State_Paused) {
			if (ResumeThread(m_hThread) != 0xFFFFFFFF)
				m_State = JCThread_State_Running;
		}
	}
	
	virtual void *Entry() = 0;	

	int GetState() {
		int localState;
		m_StateCrit.Lock();
		localState = (int)m_State;
		m_StateCrit.Unlock();
		return localState;
	};

	long JCThread_AddRef() {
		JCAutoLockCritSec refCountLock(&m_RefCountCrit);		
		return m_RefCount++;
	}
	long JCThread_RemoveRef() {
    {
		  JCAutoLockCritSec refCountLock(&m_RefCountCrit);	

		  if (--m_RefCount)
			  return m_RefCount;
    }

		delete this;

		return 0L;
	}

  enum JCThread_State { 
		JCThread_State_Waiting = 0,
		JCThread_State_Stopped,
		JCThread_State_Running,
		JCThread_State_Paused
	};
protected:	
	bool TestDestroy() {
		bool bExit = false;
		m_StateCrit.Lock();
		if (m_State == JCThread_State_Stopped)
			bExit = true;
		m_StateCrit.Unlock();
		return bExit;
	}
	HANDLE m_hThread;
	JCCritSec m_StateCrit;
	JCThread_State m_State;
	JCCritSec m_StartedCrit;
	bool m_bStarted;
	JCCritSec m_RefCountCrit;
	long m_RefCount;
};

// JCQueue
//
// Implements a simple Queue ADT.  The queue contains a finite number of
// objects, access to which is controlled by a semaphore.  The semaphore
// is created with an initial count (N).  Each time an object is added
// a call to WaitForSingleObject is made on the semaphore's handle.  When
// this function returns a slot has been reserved in the queue for the new
// object.  If no slots are available the function blocks until one becomes
// available.  Each time an object is removed from the queue ReleaseSemaphore
// is called on the semaphore's handle, thus freeing a slot in the queue.
// If no objects are present in the queue the function blocks until an
// object has been added.

#define DEFAULT_QUEUESIZE   2

template <class T> class JCQueue {
private:
    HANDLE          hSemPut;        // Semaphore controlling queue "putting"
    HANDLE          hSemGet;        // Semaphore controlling queue "getting"
    CRITICAL_SECTION CritSect;      // Thread seriallization
    int             nMax;           // Max objects allowed in queue
    int             iNextPut;       // Array index of next "PutMsg"
    int             iNextGet;       // Array index of next "GetMsg"
    T              *QueueObjects;   // Array of objects (ptr's to void)

    void Initialize(int n) {
        iNextPut = iNextGet = 0;
        nMax = n;
        InitializeCriticalSection(&CritSect);
        hSemPut = CreateSemaphore(NULL, n, n, NULL);
        hSemGet = CreateSemaphore(NULL, 0, n, NULL);
        QueueObjects = new T[n];
    }


public:
    JCQueue(int n) {
        Initialize(n);
    }

    JCQueue() {
        Initialize(DEFAULT_QUEUESIZE);
    }

    ~JCQueue() {
        delete [] QueueObjects;
        DeleteCriticalSection(&CritSect);
        CloseHandle(hSemPut);
        CloseHandle(hSemGet);
    }

    T GetQueueObject() {
        int iSlot;
        T Object;
        LONG lPrevious;

        // Wait for someone to put something on our queue, returns straight
        // away is there is already an object on the queue.
        //
        WaitForSingleObject(hSemGet, INFINITE);

        EnterCriticalSection(&CritSect);
        iSlot = iNextGet++ % nMax;
        Object = QueueObjects[iSlot];
        LeaveCriticalSection(&CritSect);

        // Release anyone waiting to put an object onto our queue as there
        // is now space available in the queue.
        //
        ReleaseSemaphore(hSemPut, 1L, &lPrevious);
        return Object;
    }

    void PutQueueObject(T Object) {
        int iSlot;
        LONG lPrevious;

        // Wait for someone to get something from our queue, returns straight
        // away is there is already an empty slot on the queue.
        //
        WaitForSingleObject(hSemPut, INFINITE);

        EnterCriticalSection(&CritSect);
        iSlot = iNextPut++ % nMax;
        QueueObjects[iSlot] = Object;
        LeaveCriticalSection(&CritSect);

        // Release anyone waiting to remove an object from our queue as there
        // is now an object available to be removed.
        //
        ReleaseSemaphore(hSemGet, 1L, &lPrevious);
    }
};

#endif // _JC_THREAD_H_
