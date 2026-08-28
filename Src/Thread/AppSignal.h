/// 
/// \file   CSignal.h
/// 
///         CSignal class header file
///


#ifndef CSignal_H
#define CSignal_H

#include <mutex>
#include <thread>
#include <thread>
#include <condition_variable>
#include <exception>



class CSignal
{
    std::mutex                    m_signalMutex;

    std::condition_variable       m_signalVar;

public:

    CSignal()
    {

    }

    ~CSignal()
    {

    }

    bool init()
    {
        return true;
    }

    bool wait(uint16_t nMsTimeout = 0)          // If nMsTimeout = 0, wait forever
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
        if (m_signalVar.wait_for(lock, std::chrono::microseconds(nMsTimeout), []{ return true; })) 
        {
            // Signal triggered
            return true;
        } 

        // Timeout expired
        return false;
    }

    void triggerSignal(bool bNotifyAll = false)
    {
        std::unique_lock<std::mutex> lock(m_signalMutex);

        if (bNotifyAll == false)
            m_signalVar.notify_one();
        else
            m_signalVar.notify_all();
    }

};


#endif  //  CSignal_H
