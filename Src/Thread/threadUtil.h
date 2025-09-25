//**************************************************************************************************
//* FILE:		threadUtil.h
//*
//* DESCRIP:	
//*
//* AUTHOR:     Russ Barker
//*


#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <assert.h>
#endif
#include <stdexcept>
#include "macros.h"


//********************************************************************
//* Various thread classes for applications that need to do threading
//* and aren't going to be including various Microsoft class libraries
//*


//********************************************************************
//* Tthread and thread
//*
//* allows you to run code on a thread via a template. Create a class
//* which has a method unsigned operator()() and use it to instantiate the
//* template. If you want to use a bare function, encapsulate it in the
//* class pointer_to_thread_function. The Vthread class uses the traditional
//* virtual function approach
//*
//	Example:
//
//	class foo
//		{
//		unsigned operator()()
//			{
//			for (int x = 0; x < 4: ++x)
//				cout << x << endl;
//
//			return 1;
//			}
//		};
//
//	unsigned bar()
//		{
//		for(int x = 0; x < 4; ++x)
//			cout << x << endl;
//		return 1;
//		}
//
//	class narf: public Vthread
//	{
//		unsigned operator()()
//			{
//			for (int x = 0; x < 4; ++x)
//				cout << x << endl;
//			return 1;
//			}
//
//	};
//
//	main()
//		{
//		Tthread<foo> f;
//		Tthread<pointer_to_thread_function> b(bar);
//		narf n;
//		n.start();
//		f.start();
//		b.start();
//		::WaitForSingleObject(f,INFINITE); // or f.join();
//		::WaitForSingleObject(b,INFINITE); // or b.join();
//		::WaitForSingleObject(n,INFINITE); // or b.join();
//		}
//
//
//**********************************************************************

#ifdef WIN32
#ifndef ASSERT
#define ASSERT(x) { if (!(x)) ::MessageBox(0,"Assert failure","ASSERT",MB_OK); }
#endif
#else
#ifndef ASSERT
#define ASSERT(x) assert(x)
#endif
#endif

#ifdef WIN32
class thread_CORE
{
    nocopy(thread_CORE)

protected:
    unsigned m_threadIdentifier;
    HANDLE	 m_threadHandle;
public:
    thread_CORE():m_threadHandle(0) {}

    ~thread_CORE() throw()
    {
        if (m_threadHandle)
        {
            ::WaitForSingleObject(m_threadHandle,INFINITE);
            ::CloseHandle(m_threadHandle);
        }
    
		m_threadHandle = 0;
    }

    inline void join() 
	{
        ::WaitForSingleObject(m_threadHandle,INFINITE);
    }
    
	inline operator HANDLE() const throw() 
	{
        return m_threadHandle;
    }
};

template<class Handler>
class Tthread: public thread_CORE, public Handler
{
    static inline unsigned __stdcall _start_func(void *arg)
    {
        return reinterpret_cast<Tthread<Handler> *>(arg)->operator()();
    }
public:
    Tthread() {}
    template <class P1> Tthread(const P1 &p1):Handler(p1) {}
    template <class P1,class P2> Tthread(const P1 &p1,const P2 &p2):Handler(p1,p2) {}
    template <class P1,class P2,class P3> Tthread(const P1 &p1,const P2 &p2,const P3 &p3):Handler(p1,p2,p3) {}
    template <class P1,class P2,class P3,class P4> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4):Handler(p1,p2,p3,p4) {}
    template <class P1,class P2,class P3,class P4,class P5> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5):Handler(p1,p2,p3,p4,p5) {}
    template <class P1,class P2,class P3,class P4,class P5,class P6> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6):Handler(p1,p2,p3,p4,p5,p6) {}
    template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6,const P7 &p7):Handler(p1,p2,p3,p4,p5,p6,p7) {}
    template <class P1> Tthread(P1 &p1):Handler(p1) {}

    void start() throw(std::runtime_error)
    {
        m_threadHandle = (HANDLE)::_beginthreadex(NULL,0,_start_func,this,0,&m_threadIdentifier);
        if (!m_threadHandle)
            throw std::runtime_error("thread::start() - Could not start thread");
    }
};

#else

class thread_CORE
{
    nocopy(thread_CORE)

protected:
    bool		m_threadValid;
    pthread_t	m_threadHandle;
public:
    thread_CORE():m_threadValid(false) {}

    ~thread_CORE() throw()
    {
        if (m_threadValid)
        {
            ::pthread_join(m_threadHandle,NULL);
        }
        m_threadValid = false;
    }

    inline void join() {
        ::pthread_join(m_threadHandle,NULL);
    }
    inline operator pthread_t() const throw() {
        return m_threadHandle;
    }
};

template<class Handler>
class Tthread: public thread_CORE, public Handler
{
    static inline void* _start_func(void *arg)
    {
        return (void*)reinterpret_cast<Tthread<Handler> *>(arg)->operator()();
    }
public:
    Tthread() {}
    template <class P1> Tthread(const P1 &p1):Handler(p1) {}
    template <class P1,class P2> Tthread(const P1 &p1,const P2 &p2):Handler(p1,p2) {}
    template <class P1,class P2,class P3> Tthread(const P1 &p1,const P2 &p2,const P3 &p3):Handler(p1,p2,p3) {}
    template <class P1,class P2,class P3,class P4> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4):Handler(p1,p2,p3,p4) {}
    template <class P1,class P2,class P3,class P4,class P5> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5):Handler(p1,p2,p3,p4,p5) {}
    template <class P1,class P2,class P3,class P4,class P5,class P6> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6):Handler(p1,p2,p3,p4,p5,p6) {}
    template <class P1,class P2,class P3,class P4,class P5,class P6,class P7> Tthread(const P1 &p1,const P2 &p2,const P3 &p3,const P4 &p4,const P5 &p5,const P6 &p6,const P7 &p7):Handler(p1,p2,p3,p4,p5,p6,p7) {}
    template <class P1> Tthread(P1 &p1):Handler(p1) {}

    void start() throw(std::runtime_error)
    {
        if (m_threadValid)
            throw std::runtime_error("thread::start() - Already exists");
        if (pthread_create(&m_threadHandle,NULL,_start_func,this))
            throw std::runtime_error("thread::start() - Could not start thread");
        m_threadValid = true;
    }
};
#endif


class pointer_to_thread_function
{
public:
    typedef unsigned (*func_t)();

    inline pointer_to_thread_function(func_t f):m_function(f) {}
private:
    func_t	m_function;
protected:
    inline unsigned operator()()	{
        return (*m_function)();
    }
};

class vthread_stub
{
protected:
    virtual unsigned operator()() = 0;
};

typedef Tthread<vthread_stub> Vthread;

#ifdef WIN32
/// these provide basic resource mgmt for typical uses
class mutex
{
    nocopy(mutex)

private:
    HANDLE	m_mutex;

public:
    mutex() throw(std::runtime_error);
    ~mutex() throw();
    void lock() throw() {
        ::WaitForSingleObject(m_mutex,INFINITE);
    }
    void unlock() throw() {
        ::ReleaseMutex(m_mutex);
    }
    inline operator HANDLE() const throw() {
        return m_mutex;
    }
};

class event
{
    nocopy(event)

private:
    HANDLE	m_event;

public:
    event(BOOL bManualReset) throw(std::runtime_error);
    ~event() throw();

    inline operator HANDLE() const throw() {
        return m_event;
    }
};

/* class stackLock

  Stack based mutex locker/unlocker for unwrapped Win32 mutexes.
  Equivalent to CAutoLock for CCritSec objects.
*/

class stackLock
{
    HANDLE m_h;
public:
    stackLock(HANDLE h): m_h(h)
    {
        ::WaitForSingleObject(m_h,INFINITE);
    }
    ~stackLock()
    {
        ::ReleaseMutex(m_h);
    }
};
#define common_namespace
#else

class Win32SyncObject
{
public:
    virtual void syncObjectWait() throw(std::runtime_error) = 0;
    virtual bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) = 0;
    virtual ~Win32SyncObject() {}
};
typedef Win32SyncObject* Win32SyncObjectPtr;

/// these provide basic resource mgmt for typical uses

class conditionVariable;

namespace common {
// conflicts with headers in solaris
class mutex: public Win32SyncObject
{
    nocopy(mutex)

private:
    pthread_mutex_t	m_mutex;

public:
    mutex()  throw(std::runtime_error);
    ~mutex() throw();
    void lock() throw(std::runtime_error);
    bool timedLock(int milliseconds) throw(std::runtime_error);
    void unlock() throw(std::runtime_error);
    void syncObjectWait() throw(std::runtime_error) {
        lock();
    }
    bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) {
        return timedLock(milliseconds);
    }

    inline operator Win32SyncObject*() throw() {
        return this;
    }

    friend class conditionVariable;

};
}


class conditionVariable
{
    nocopy(conditionVariable)

private:
    pthread_cond_t	m_conditionVariable;

public:
    conditionVariable() throw(std::runtime_error);
    ~conditionVariable() throw();
    void wait(common_namespace::mutex &m) throw(std::runtime_error);
    bool timedWait(common_namespace::mutex &m,int milliseconds) throw(std::runtime_error);
    void signal() throw(std::runtime_error);
    void broadcast() throw(std::runtime_error);
};


class event: public Win32SyncObject
{
    nocopy(event)

private:
    bool m_manualReset;
    bool m_signaled;

    common_namespace::mutex	m_mutex;
    conditionVariable		m_conditionVariable;

public:
    event(bool bManualReset) throw(std::runtime_error);
    void wait() throw(std::runtime_error);
    bool timedWait(int milliseconds) throw(std::runtime_error);
    void setEvent() throw(std::runtime_error);
    void resetEvent() throw(std::runtime_error);
    void syncObjectWait() throw(std::runtime_error) {
        wait();
    }
    bool syncObjectTimedWait(int milliseconds) throw(std::runtime_error) {
        return timedWait(milliseconds);
    }

    inline operator Win32SyncObject*() throw() {
        return this;
    }
};


/* class stackLock

  Stack based mutex locker/unlocker for unwrapped Win32 mutexes.
  Equivalent to CAutoLock for CCritSec objects.
*/

class stackLock
{
    common_namespace::mutex &m_m;
public:
    stackLock(common_namespace::mutex &m): m_m(m)
    {
        m_m.lock();
    }
    ~stackLock()
    {
        m_m.unlock();
    }
};

#define WAIT_ABANDONDED (-1)
#define WAIT_TIMEOUT (0)
#define WAIT_OBJECT_0 (1)
#define INFINITE (-1)

int WaitForSingleObject(Win32SyncObject &o,int milli_timeout) throw();
inline int WaitForSingleObject(Win32SyncObject *o,int milli_timeout) throw() {
    return WaitForSingleObject(*o,milli_timeout);
}
int WaitForMultipleObjects(int count,Win32SyncObjectPtr *objs,bool waitall,int milliseconds) throw();
inline bool SetEvent(event &e) {
    bool result=false;
    try {
        e.setEvent();
        result=true;
    }
    catch(...) {} return result;
}
inline bool SetEvent(event *e) {
    return SetEvent(*e);
}
inline bool ResetEvent(event &e) {
    bool result=false;
    try {
        e.resetEvent();
        result=true;
    }
    catch(...) {} return result;
}
inline bool ResetEvent(event *e) {
    return ResetEvent(*e);
}
inline bool ReleaseMutex(common_namespace::mutex &m) {
    bool result=false;
    try {
        m.unlock();
        result=true;
    }
    catch(...) {} return result;
}
inline bool ReleaseMutex(common_namespace::mutex *m) {
    return ReleaseMutex(*m);
}
inline void CloseHandle(Win32SyncObject *o) {
    delete o;
}
#endif

#endif


