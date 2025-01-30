//****************************************************************************
// FILE:    CVectorBuffeer.h
//
// DEESC:   Buffer class with configurable data type, 
//          derrived from std::vector.
//
// AUTHOR:  Russ Barker
//


#ifndef _VECTOR_BUFFER_h
#define _VECTOR_BUFFER_h

#include <mutex>
#include <string>
#include <vector>

#include "../Error/CError.h"


template <typename T> class CVectorBuffer: public CErrorHandler
{
    std::vector<T> m_dataBuffer;
    size_t         m_maxSize;
    std::mutex     m_ioMutex;

  protected:

    // Delete (remove) 1 entry at pos
    // If no pos is given, delete entry at pos zero (0).
    bool erase(int pos = -1)
    {
        auto bufSize = m_dataBuffer.size();

        if (pos >= (int)bufSize)
            return false;

        if (pos == -1)
            pos = 0;

        auto entry = (m_dataBuffer.begin() + pos);
        m_dataBuffer.erase(entry);

        auto newSize = m_dataBuffer.size();
        if (newSize > (bufSize - 1))
            return false;

        return true;
    }

    // Delete (remove) array entries, from  start to 1 less than end. Range is NOT inclusive of end
    bool erase(int start = -1, int end = -1)
    {
        auto bufSize = m_dataBuffer.size();

        if (start >= (int)bufSize)
            return false;

        if (end > (int)bufSize)
            return false;

        if (start == -1)
            start = 0;

        if (end == -1)
            end = ((int)m_dataBuffer.size());

        if (start >= end)
            return false;

        auto first = (m_dataBuffer.begin() + start);
        auto last  = (m_dataBuffer.begin() + end);
        m_dataBuffer.erase(first, last);

        auto delSize = (end - start);
        auto newSize = m_dataBuffer.size();
        if (newSize > (bufSize - delSize))
            return false;

        return true;
    }

  public:

    CVectorBuffer(const size_t buffSize = 0)
    {
        m_maxSize = 0;
        if (buffSize > 0)
            m_maxSize = buffSize;
    }

    ~CVectorBuffer()
    {
        clear();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        m_dataBuffer.clear();
    }

    // Set the maximum number of entries that can be stored in the entry array.
    bool setMaxBufferSize(const size_t buffSize)
    {
        if (buffSize > 0)
        {
            if (buffSize > m_dataBuffer.max_size())
                return false;

            m_maxSize = buffSize;
            return true;
        }

        return false;
    }

    // Get the maximum number of entries that can be stored in the entry array.
    int getMaxBufferSize()
    {
        return m_maxSize;
    }

    // Get the current number of entries in the data block (entry array).
    int getCurrentDataSize()
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return (int)m_dataBuffer.size();
    }

    T get(const unsigned int pos)
    {
        std::lock_guard<std::mutex> lock{ m_ioMutex };

        if (pos >= m_maxSize)
            return (T) 0;;

        return m_dataBuffer.at(x);
    }

    bool set(const unsigned int pos, T &val)
    {
        std::lock_guard<std::mutex> lock{ m_ioMutex };

        if (pos >= m_maxSize)
            return false;;

        m_dataBuffer.at(pos) = val;

        return true;
    }

    // Read "blockSize" entries from the data block (entry array).
    int readBlock(T *pTargetBuff, const size_t blockSize, const bool deleteEntries = false)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (pTargetBuff == nullptr)
            return -1;

        if (blockSize > m_maxSize)
            return -1;

        size_t numSamples = m_dataBuffer.size();

        if (numSamples < 1)
            return 0;

        size_t readSize = blockSize;

        if (readSize > numSamples)
            readSize = numSamples;

        // copy the sample data from the output buffer to the target buffer
        for (unsigned int x = 0; x < readSize; x++)
            *(pTargetBuff + x) = m_dataBuffer.at(x);

        // If true, delete those sample entries from the output buffer
        if (deleteEntries == true)
        {
            erase(0, (int)readSize);
            size_t newSize = m_dataBuffer.size();
            if (newSize != numSamples - readSize)
                return -1;
        }

        return readSize;
    }

    // Write "blockSize" entries to the data block (entry array).
    bool writeBlock(const T *pBuff, const size_t blockSize)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        auto                        bufSize = m_dataBuffer.size();
        if ((bufSize + blockSize) > m_maxSize)
        {
            return false;
        }

        for (unsigned int x = 0; x < blockSize; x++)
        {
            T sampleValue = *(pBuff + x);
            m_dataBuffer.push_back(sampleValue);
        }

        return true;
    }

    // Delete (remove) 1 entry at "pos" from the data block (entry array).
    // If no pos is given, delete entry at pos zero (0).
    bool deleteEntry(int pos = -1)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return erase(pos);
    }

    // Delete (remove) entries, from  start to 1 less than end. 
    // Range is NOT inclusive of end.
    bool deleteBlock(int start = -1, int end = -1)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return erase(start, end);
    }

    // Delete (remove) entries, from  beginning of buffer. 
    // Range is NOT inclusive of end.
    bool deleteBlockFromStart(int end = -1)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return erase(0, end);
    }

    // Get a pointer to thr data block (entry array), starting with the 1st entry.
    // The data block will be "locked" until "freeDataPtr" is called.
    T *getBufferPtr()
    {
        m_ioMutex.lock();

        return (T *)(m_dataBuffer.data());
    }

    // "unlock" the data block "locked" by the "freeDataPtr" call.
    void freeBufferPtr()
    {
        m_ioMutex.unlock();
    }
};

#endif // _VECTOR_BUFFER_h
