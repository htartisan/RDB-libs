//**********************************************************************************
//* FILE:    ThreadBase.h
//*
//* DEESC:   This header file defines a std thread base class.
//*          Derrive new thread classes from this class to 
//*          easaly create thread objects.
//*          NOTE: The derrived class MUST define a threadProce
//*          ("void threadProc(void)")
//*
//* AUTHOR:  Russ Barker
//*


#ifdef WINDOWS
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 
#endif


#ifndef THREAD_BASE_H
#define THREAD_BASE_H

#include <mutex>
#include <condition_variable>
#include <exception>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>

#if defined(WINDOWS)
#include <windows.h>
#include <thread>
#include <processthreadsapi.h>
#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL     0
#endif
#elif defined(__QNX__)
#include <pthread.h>
#include <sched.h>
#include <sys/neutrino.h>
#else
#include <pthread.h>
#endif

#include "../Logging/Logging.h"


namespace ThreadBaseDefs
{

#if defined(WINDOWS)
    void newCThreadStartup(void* pContext);
#else
    void* newPThreadStartup(void* pContext);
#endif


#if defined(WINDOWS)

//typedef void (*pStartupFunc_def)(void*);

// If WINDOWS use std::thread

#define ThreadHandle_def					std::unique_ptr<std::thread>
//#define ThreadHandle_def					std::thread

#else

//typedef void * (*pStartupFunc_def)(void*);

// Else use pThread

#define ThreadHandle_def					pthread_t

#endif


// Thread base class

class CThreadBase
{
  protected:

    std::string                 m_sName;

    int                         m_priority;

    ThreadHandle_def            m_threadHandle;

    std::mutex                  m_signalMutex;

    std::condition_variable     m_signalVar;

	std::atomic<bool> m_running{false};
	std::atomic<bool> m_bThreadExitFlag{false};

    //volatile bool               m_running;
    //volatile bool               m_bThreadExitFlag;

    std::mutex                  m_startupMutex;

    std::mutex                  m_exitMutex;

    std::string                 m_sLastError;

    inline void clearThreadHandle()
    {
#if defined(WINDOWS)
        //m_threadHandle.release();
        m_threadHandle = nullptr;
#else
        m_threadHandle = 0;
#endif
    }

    inline bool validThreadHandle()
    {
#if defined(WINDOWS)
        if (m_threadHandle != nullptr)
#else
        if (m_threadHandle != 0)
#endif
        {
            return true;
        }

        return false;
    }

    void joinThread()
    {
        if (validThreadHandle() == true)
        {
            try
            {
#if defined(WINDOWS)
                m_threadHandle->join();
#else
                auto status = pthread_join(m_threadHandle, NULL);
                if (status != 0)
                {
                    m_sLastError = "'pthread_join' returned error: ";
                    m_sLastError.append(std::to_string(status));
                }
#endif

                clearThreadHandle();
            }
            catch(...)
            {
                m_sLastError = "Unknown exception during joinThread";
            }
        }

        m_running = false;
    }

    bool setThreadName(std::string &sName)
    {
        if (sName == "")
        {
            return false;
        }

        sName.append("_thread");

        if (validThreadHandle() == false)
        {
            return false;
        }

        try
        {
#if defined(WINDOWS)
            // Windows - set thread name
            std::wstring wideName(sName.begin(), sName.end());
            auto result =
                SetThreadDescription(m_threadHandle.get(), wideName.c_str());
                //SetThreadDescription(m_threadHandle, (PCWSTR)sName.c_str());
            if ((FAILED(result)))
            {
                m_sLastError = "'SetThreadDescription' failed ";
                return false;
            }
#else
            // Linux/QNX - set pthread name
            auto status = pthread_setname_np(m_threadHandle, sName.c_str());
            if (status != 0)
            {
                m_sLastError = "'pthread_setname_np' returned error: ";
                m_sLastError.append(std::to_string(status));
                return false;
            }
#endif
        }
        catch (...)
        {
            m_sLastError = "Unknown exception during setThreadName";
            return false;
        }

        return true;
    }

    bool setThreadPriority(int newPri)
    {
        if (validThreadHandle() == false)
        {
            return false;
        }

        try
        {
#if defined(WINDOWS)
            auto result = 
                SetThreadPriority(m_threadHandle.get(), newPri);
            if (result != TRUE)
            {
                m_sLastError = "'SetThreadPriority' failed ";
                return false;
            }
#else
            // set pthread priority
            //int newPri = 20; 
            // Set the desired priority (higher value = higher priority)
            int status = 
                pthread_setschedprio(m_threadHandle, newPri);
            if (status != 0)
            {
                m_sLastError = "'pthread_setschedprio' returned error: ";
                m_sLastError.append(std::to_string(status));
                return false;
            }
#endif
        }
        catch (...)
        {
            m_sLastError = "Unknown exception during setThreadPriority";
            return false;
        }

        return true;
    }

  public:

    CThreadBase()
    {
        m_sName.clear();

#if defined(WINDOWS)
        m_priority =            THREAD_PRIORITY_NORMAL;
#else
        m_priority =            20;
#endif

        clearThreadHandle();

        m_running =             false;;
        m_bThreadExitFlag =     false;;

        m_sLastError =          "";
    }

    CThreadBase(const std::string &sName, const int nPrio = 0)
    {
        m_sName =               sName;

        if (nPrio != 0)
        {
            m_priority =        nPrio;
        }

        clearThreadHandle();

        m_running =             false;;
        m_bThreadExitFlag =     false;;

        m_sLastError =          "";
    }

    ~CThreadBase()
    {
        if (m_running == true)
        {
            joinThread();
        }
    }

    void setLastError(const std::string sErr)
    {
        m_sLastError = sErr;
    }

    std::string getLastError()
    {
        std::string sOut = m_sLastError;

        m_sLastError = "";

        return sOut;
    }

    void setName(const std::string& sName = "")
    {
        if (sName != "")
        {
            m_sName = sName;
        }

        if (m_running == true)
        {
            setThreadName(m_sName);
        }
    }

    std::string getName()
    {
        return m_sName;
    }

    void setPriority(const int nPrio = 0)
    {
        if (nPrio != 0)
        {
            m_priority = nPrio;
        }

        if (m_running == true)
        {
            setThreadPriority(m_priority);
        }
    }

    void setRunning(bool bVal)
    {
        m_running = bVal;
    }

    bool isActive()
    {
        return m_running;
    }

    bool createThread()     // This function = start thread
    {
        std::scoped_lock lock(m_startupMutex);

        if (m_running)
        {
            return false;
        }

        try
        {
            auto pThis = this;

            if (pThis == nullptr)
            {
                m_sLastError = "Invalid 'this' pointer during createThread";
                return false;
            }

            m_bThreadExitFlag = false;
            m_running = true;

#if defined(WINDOWS)
            m_threadHandle = std::make_unique<std::thread>((ThreadBaseDefs::newCThreadStartup), ((void*) pThis));
            //m_threadHandle = new std::thread(pNewThread, pThis);
#else
            auto status = 
                pthread_create(&m_threadHandle, nullptr, (ThreadBaseDefs::newPThreadStartup), ((void *) pThis));
            if (status != 0)
            {
                m_running = false;
                m_sLastError = "'pthread_create' returned error: ";
                m_sLastError.append(std::to_string(status));
                return false;
            }
#endif
        }
        catch(const std::exception& e)
        {
            m_running = false;
            m_sLastError = e.what();
            return false;
        }
        catch(...)
        {
            m_running = false;
            m_sLastError = "Unknown exception during creation of new thread startup proc";
            return false;
        }
        
        if (validThreadHandle() == false)
        {
            m_running = false;
            m_sLastError = "Invalid thread proc pointer during createThread";
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

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (m_running == true)
        {
            if (validThreadHandle() == true)
            { 
                joinThread();
            }

            clearThreadHandle();
			
            m_running = false;
        }
    }

    void killThread()
    {
        if (m_running == true)
        {
            m_bThreadExitFlag = true;

            if (validThreadHandle() == true)
            { 
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

                try
                {
#if defined(WINDOWS)
                    auto handle = m_threadHandle->native_handle();
                    ::TerminateThread(handle, 0);
#else
                    //pthread_kill(m_threadHandle, 0);
                    pthread_cancel(m_threadHandle);
#endif
                }
                catch(...)
                { 
                    m_sLastError = "Unknown exception during killThread";
                }

                clearThreadHandle();
            }

            m_running = false;
        }
    }

    bool waitForSignal(uint16_t nMsTimeout = 0)          // If nMsTimeout = 0, wait forever
    {
        std::unique_lock<std::mutex> lock(m_signalMutex);

        //  If nMsTimeout = 0, wait with no timeout
        if (nMsTimeout < 1)                     
        {
            m_signalVar.wait(lock);

            // Signal triggered
            return true;
        } 

        //  Wait with timeout 'nMsTimeout'
        auto status = m_signalVar.wait_for(lock, std::chrono::milliseconds(nMsTimeout));
        if (status == std::cv_status::timeout) 
        {
            // Timeout expired
            return false;
        } 

        return true;
    }

    void triggerSignal(bool bWaitAll = false)
    {
        std::unique_lock<std::mutex> lock(m_signalMutex);

        if (bWaitAll == false)
        {
            m_signalVar.notify_one();
        }
        else
        {
            m_signalVar.notify_all();
        }
    }

    virtual void threadProc(void) = 0; 

};


#if defined(WINDOWS)

inline void newCThreadStartup(void* pContext)
{
    if (pContext == nullptr)
    {
        return;
    }

    CThreadBase* pThisClass = (CThreadBase*) pContext;

    std::string sName = "";

    try
    {
        pThisClass->setRunning(true);

        pThisClass->setName();

        pThisClass->setPriority();

        pThisClass->threadProc();
    }
    catch (std::exception& e)
    {
        LogError("Unhandled exception: {}", e.what());
        pThisClass->setLastError(e.what());
        pThisClass->setRunning(false);
    }
    catch (...)
    {
        LogError("Unknown exception during newPThreadStartup");
        pThisClass->setLastError("Unknown exception during newCThreadStartup");
        pThisClass->setRunning(false);
    }

    return;
}

#else

inline void* newPThreadStartup(void *pContext)
{
    if (pContext == nullptr)
    {
        return nullptr;
    }

    CThreadBase* pThisClass = (CThreadBase*) pContext;

    std::string sName = "";

    std::string sStatus = "Thread started";

    try
    {
        int status, oldValue;

        status = 
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldValue);
        if (status != 0)
        {
            sStatus = "'pthread_setcancelstate' failed";
            pThisClass->setLastError(sStatus);
        }

        status = 
            pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldValue);
        if (status != 0)
        {
            sStatus = "'pthread_setcanceltype' failed";
            pThisClass->setLastError(sStatus);
        }

        pThisClass->setRunning(true);

        pThisClass->setName();

        pThisClass->setPriority();

        pThisClass->threadProc();
    }
    catch (std::exception& e)
    {
        sStatus = "Unhandled exception: {}";
        sStatus.append(e.what());
        pThisClass->setLastError(sStatus);
        pThisClass->setRunning(false);
    }
    catch (...)
    {
        sStatus = "Unknown exception";
        pThisClass->setLastError(sStatus);
        pThisClass->setRunning(false);
    }

    //return (void *) sStatus.c_str();
	return nullptr;
}

#endif


};  //  ThreadBaseDefs


#endif  //  THREAD_BASE_H
