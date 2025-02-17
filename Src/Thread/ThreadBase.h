//**********************************************************************************
// FILE:    ThreadBase.h
//
// DEESC:   This header file defines a std thread base class.
//          Derrive new thread classes from this class to 
//          easaly create thread objects.
//          NOTE: The derrived class MUST define a threadProce
//          ("void threadProc(void)")
//
// AUTHOR:  Russ Barker
//


#ifndef THREAD_BASE_H
#define THREAD_BASE_H

#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <condition_variable>
#include <exception>

#if defined(WINDOWS)
//#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__QNX__)
#include <pthread.h>
#include <sched.h>
#include <sys/neutrino.h>
#else
#include <pthread.h>
#endif

#include "../Logging/Logging.h"


typedef std::unique_ptr<std::thread>    ThreadHandle_def;


// Thread base class

class CThreadBase
{
  protected:

    std::string                   m_sName;

    int                           m_priority;

    ThreadHandle_def              m_threadHandle;

    std::mutex                    m_signalMutex;

    std::condition_variable       m_signalVar;

    volatile bool                 m_running;

    std::mutex                    m_startupMutex;

    volatile bool                 m_bThreadExitFlag;

    std::mutex                    m_exitMutex;

    void newThread()
    //void newThread(CThreadBase * pThisClass)
    {
        try
        {
            m_running = true;

            // Set thread name
            std::string sThreadName = (m_sName + "_thread");

            setThreadName(sThreadName);

            setThreadPriority(20);

            threadProc();
        }
        catch (std::exception &e)
        {
            LogCritical("Unhandled exception: {}", e.what());
        }
        catch (...)
        {
            LogCritical("Unhandled unknown exception");
        }
    }

    void joinThread()
    {
        if (m_threadHandle != nullptr)
        {
            m_threadHandle->join();

            m_threadHandle = nullptr;
        }

        m_running = false;
    }

    void setThreadName(const std::string &sName)
    {
        m_sName =  sName;

#if defined(WINDOWS)
        // Windows - set thread name
        SetThreadDescription(m_threadHandle.get(), (PCWSTR) m_sName.c_str());
#elif defined(__QNX__)
        // QNX - set thread name
        pthread_setname_np(pthread_self(), m_sName.c_str());
#else
        // Linux - set pthread name
        pthread_setname_np(pthread_self(), m_sName.c_str());
#endif
    }

    void setThreadPriority(int newPri)
    {
#if (defined(__QNX__) || defined(Linux))
        // set pthread priority
        pthread_t threadID = pthread_self();
        // Set the desired priority (higher value = higher priority)
        int result = pthread_setschedprio(threadID, newPri);
#endif
    }

  public:

    CThreadBase()
    {
        m_sName.clear();

        m_priority =        0;

        m_threadHandle =    nullptr;

        m_running =         false;;
        m_bThreadExitFlag = false;;
    }

    CThreadBase(const std::string &sName, int pri = 0)
    {
        m_sName =           sName;

        m_priority =        pri;

        m_threadHandle =    nullptr;

        m_running =         false;;
        m_bThreadExitFlag = false;;
    }

    ~CThreadBase()
    {
        stopThread(true);
    }

    bool createThread()     // This function = start thread
    {
        std::scoped_lock lock(m_startupMutex);

        if (m_running)
        {
            return false;
        }

        // Start the AWE audio I/O pump thread
        m_threadHandle = std::make_unique<std::thread>(&CThreadBase::newThread, this);

        if (m_threadHandle == nullptr)
        {
            return false;
        }

        return true;
    }

    void stopThread(bool bSetExitFlag = true)
    {
        std::scoped_lock lock(m_exitMutex);

        if (bSetExitFlag == true)
        {
            m_bThreadExitFlag = true;
        }

        if (m_running == true)
        {
            joinThread();
        }
    }

    bool isActive()
    {
        return m_running;
    }

    void waitForSignal()
    {
        std::unique_lock<std::mutex> lock(m_signalMutex);

        m_signalVar.wait(lock); 
    }

    void triggerSignal(bool bWaitAll = false)
    {
        std::unique_lock<std::mutex> lock(m_signalMutex);

        if (bWaitAll == false)
            m_signalVar.notify_one();
        else
            m_signalVar.notify_all();
    }

    virtual void threadProc(void) = 0; 

};


#endif  //  THREAD_BASE_H
