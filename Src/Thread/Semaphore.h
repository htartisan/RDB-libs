/// 
/// \file       CIpc.hpp
/// 
///             This header file contains writer and reader 
///             C++ classes for the IPC.
///


#ifndef CSEMAPHONE_HPP
#define CSEMAPHONE_HPP


#include <mutex>
#include <condition_variable>


class CBinarySemaphore 
{
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_count;

public:
    CBinarySemaphore() :
         m_count(0) 
    {

    }

    bool init(int initial_count = 0)
    {
        if (initial_count > 1 || initial_count < 0) 
        {
            return false;
        }

        m_count = initial_count;

        return true;
    }

    // Wait (P) operation: acquires the semaphore, blocks if count is zero
    bool acquire(uint32_t nTimeoutMs = 0) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Wait until the count is greater than zero, then decrement
        if (nTimeoutMs == 0)
        {
            m_cv.wait(lock, [this] { return m_count > 0; });
        }
        else
        {
            if (m_cv.wait_for(lock, std::chrono::milliseconds(nTimeoutMs), [this] { return m_count > 0; }) == false)
            {
                return false;
            }
        }

        m_count--;
        return true;
    }

    // Signal (V) operation: releases the semaphore, unblocks a waiting thread
    void release() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        if (m_count < 1) 
        {
            m_count++;
            lock.unlock();
            m_cv.notify_one(); // Notify one waiting thread
        }
    }

};


#endif  //  CSEMAPHONE_HPP
