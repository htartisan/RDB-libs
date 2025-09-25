//**************************************************************************************************
//* FILE:		threadUtil.cpp
//*
//* DESCRIP:	
//*
//* AUTHOR:     Russ Barker
//*


#include "thread.h"


using namespace std;


#ifdef WIN32

mutex::mutex() throw(runtime_error)
    :m_mutex(NULL)
{
    m_mutex = ::CreateMutex(NULL,FALSE,NULL);
    if (!m_mutex)
        throw runtime_error("Could not create mutex");
}

mutex::~mutex() throw()
{
    forgetHandleNULL(m_mutex);
}


event::event(BOOL bManualReset) throw(runtime_error)
    :m_event(NULL)
{
    m_event = ::CreateEvent(NULL,bManualReset,FALSE,NULL);
    if (!m_event)
        throw runtime_error("Could not create event object");
}

event::~event() throw()
{
    forgetHandleNULL(m_event);
}

#else

#include <errno.h>
#include <vector>

using namespace common_namespace;

mutex::mutex() throw(runtime_error)
{
    if (::pthread_mutex_init(&m_mutex,NULL))
        throw runtime_error("Could not create mutex");
}
mutex::~mutex() throw()
{
    ::pthread_mutex_destroy(&m_mutex);
}

void mutex::lock() throw(runtime_error)
{
    if (::pthread_mutex_lock(&m_mutex))
        throw runtime_error("Could not lock mutex");
}

bool mutex::timedLock(int milliseconds) throw(runtime_error)
{
    if (milliseconds == INFINITE)
    {
        lock();
        return true;
    }
    if (milliseconds == 0)
    {
        int err = ::pthread_mutex_trylock(&m_mutex);
        if (err == EBUSY)
            return false;
        if (err)
            throw runtime_error("Could not trylock mutex");
        return true;
    }

    int tenth_second_sleep_intervals = milliseconds / 100;
    for(int x = 0; x < tenth_second_sleep_intervals; ++x)
    {
        int err = ::pthread_mutex_trylock(&m_mutex);
        if (!err)
            return true;
        ::usleep(100000);
        if (err == EBUSY)
            continue;
        throw runtime_error("Could not trylock mutex");
    }
    return false;
}

void mutex::unlock() throw(runtime_error)
{
    if (::pthread_mutex_unlock(&m_mutex))
        throw runtime_error("Could not unlock mutex");
}

conditionVariable::conditionVariable()  throw(runtime_error)
{
    if (::pthread_cond_init(&m_conditionVariable,NULL))
        throw runtime_error("Could not create conditionVariable");
}

conditionVariable::~conditionVariable() throw()
{
    ::pthread_cond_destroy(&m_conditionVariable);
}

void conditionVariable::wait(mutex &m) throw(runtime_error)
{
    if (::pthread_cond_wait(&m_conditionVariable,&m.m_mutex))
        throw runtime_error("Could not wait on condition variable");
}

bool conditionVariable::timedWait(common_namespace::mutex &m,int milliseconds) throw(runtime_error)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds - ((milliseconds / 1000) * 1000)) * 1000000;
    int err = ::pthread_cond_timedwait(&m_conditionVariable,&m.m_mutex,&ts);
    if (!err)
        return true;
    if (err == ETIMEDOUT)
        return false;
    throw runtime_error("timedWait error");
    return false;
}

void conditionVariable::signal() throw(runtime_error)
{
    if (::pthread_cond_signal(&m_conditionVariable))
        throw runtime_error("Could not signal condition variable");
}

void conditionVariable::broadcast() throw(runtime_error)
{
    if (::pthread_cond_broadcast(&m_conditionVariable))
        throw runtime_error("Could not broadcast on condition variable");
}

event::event(bool manualReset)  throw(runtime_error)
    :m_manualReset(manualReset),
     m_signaled(false)
{
}

void event::wait() throw(runtime_error)
{
    stackLock sl(m_mutex);
    while(!m_signaled)
        m_conditionVariable.wait(m_mutex);
    if (!m_manualReset)
        m_signaled = false;
}

bool event::timedWait(int milliseconds) throw(runtime_error)
{
    if (milliseconds == INFINITE)
    {
        wait();
        return true;
    }
    if (milliseconds == 0)
    {
        stackLock sl(m_mutex);
        bool result = m_signaled;
        if (m_signaled && !m_manualReset)
            m_signaled = false;
        return result;
    }

    stackLock sl(m_mutex);
    while(!m_signaled)
    {
        if (!m_conditionVariable.timedWait(m_mutex,milliseconds))
            return false;
    }

    if (!m_manualReset)
        m_signaled = false;
    return true;
}

void event::setEvent() throw(runtime_error)
{
    stackLock sl(m_mutex);
    m_signaled = true;
    m_conditionVariable.signal();
}

void event::resetEvent() throw(runtime_error)
{
    stackLock sl(m_mutex);
    m_signaled = false;
}

int WaitForSingleObject(Win32SyncObject &o,int milli_timeout) throw()
{
    int result = WAIT_ABANDONDED;

    try {
        result = (o.syncObjectTimedWait(milli_timeout) ? WAIT_OBJECT_0 : WAIT_TIMEOUT);
    }
    catch(...) {}

    return result;
}

int WaitForMultipleObjects(int count,Win32SyncObjectPtr *objs,bool waitall,int milliseconds) throw()
{
    try {
        std::vector<bool> signaled(count,false);
        int sig_count = 0;

        for(int x = 0; x < milliseconds; ++x)
        {
            for(int oo = 0; oo < count; ++oo)
            {
                if (!signaled[oo])
                {
                    if (objs[oo]->syncObjectTimedWait(0))
                    {
                        signaled[oo] = true;
                        sig_count += 1;
                        if (!waitall)
                            return WAIT_OBJECT_0 + oo;
                        if (sig_count == count)
                            return WAIT_OBJECT_0;
                    }
                }
            } //for
            ::usleep(1000);
        } // for

        return WAIT_TIMEOUT;

    } //try
    catch(...)
    {}
    return WAIT_ABANDONDED;
}


#endif

