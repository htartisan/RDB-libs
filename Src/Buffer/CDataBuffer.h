//****************************************************************************
// FILE:    CDataBuffer.h
//
// DESC:    A data buffer handler class.
//
// AUTHOR:  Russ Barker
//

#ifndef _DATA_BUFFER_CLASS_H
#define _DATA_BUFFER_CLASS_H

#include <malloc.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

#include "../Error/CError.h"


template <class T> class CSimpleDataBuffer : 
    public CErrorHandler
{
    unsigned int  m_frameSize;      // number of elements in a frame
    unsigned int  m_blockSize;      // number of frames per buffer
    unsigned int  m_arraySize;      // arraySize = total numner of elements in a block

    unsigned long m_readIdx;        // current frame/data item read offset
    unsigned long m_writeIdx;       // current frame/data item write offset

    bool          m_bAllocated;     // whether the data buffer is allocated

    T*            m_pBuffer;

    std::mutex    m_ioLock;

  public:

    CSimpleDataBuffer(bool bInterleaved = false) :
        m_frameSize(1),
        m_bAllocated(false)
    {
        m_blockSize     = 0;
        m_arraySize     = 0;

        m_readIdx       = 0;
        m_writeIdx      = 0;

        m_pBuffer       = nullptr;
    }

    ~CSimpleDataBuffer()
    {
        m_blockSize     = 0;
        m_arraySize     = 0;

        m_readIdx       = 0;
        m_writeIdx      = 0;

        free();
    }

    void setFrameSize(unsigned int frameSize)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_frameSize = frameSize;
    }

    unsigned int getFrameSize()
    {
        return m_frameSize;
    }

    void setFramePerBlock(unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_blockSize = blockSize;
    }

    unsigned int getFramesPerBlock()
    {
        return m_blockSize;
    }

    void setSize(unsigned int frameSize, unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_frameSize = frameSize;
        m_blockSize = blockSize;
    }

    bool alloc(unsigned int blockSize = 0)
    {
        if (blockSize > 0)
            m_blockSize = blockSize;

        if (m_blockSize < 1)
        {
             return false;
        }

        if (m_frameSize < 1)
        {
            return false;
        }

        m_arraySize = (m_blockSize * m_frameSize);  // arraySize = total numner of data items in the buffer

        m_pBuffer = (T*) calloc(m_arraySize, sizeof(T));

        if (m_pBuffer == nullptr)
        {
            m_arraySize = 0;

            return false;
        }

        m_bAllocated = true;

        return true;
    }

    bool allocate()
    {
        return alloc(0);
    }

    bool isAllocated()
    {
        return m_bAllocated;
    }

    void free()
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_pBuffer != nullptr)
        {
            ::free(m_pBuffer);
        }

        m_bAllocated = false;
    }

    unsigned int getLength()
    {
        return m_arraySize;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {            
            return;
        }

        if (m_arraySize < 1)
        {
            return;
        }

        memset(m_pBuffer, 0, (sizeof(T) * m_arraySize));

        m_readIdx  = 0;
        m_writeIdx = 0;
    }

    T *getBuffPtr()
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        return m_pBuffer;
    }


    // This function gets a pointer to the start 
    // of a frame or the start of a channel, 
    // depending on whether the data
    // is interleaved or not.
    T *getDataPtr(const unsigned index)
    {
        unsigned int offset;

        offset = index * m_frameSize;   // offset = frame_number = (index * size_of_a_frame)   

        if (offset >= m_arraySize)
        {
            return nullptr;
        }

        return (m_pBuffer + offset);
    }

    inline T *getFramePtr(const unsigned int frame)
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        if (frame >= m_blockSize)
        {
            return nullptr;
        }

        auto offset = frame * m_frameSize;  // frame offset = (frame_number * size_of_a_frame)

        if (offset >= m_arraySize)
        {
            return nullptr;
        }

        return (m_pBuffer + offset);
    }

    void lockBuffer()
    {
        m_ioLock.lock();
    }

    void unlockBuffer()
    {
        m_ioLock.unlock();
    }

    T getData(unsigned int idx, unsigned int frame)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_frameSize < 1 || frame > m_frameSize || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {            
            return 0;
        }

        unsigned int offset;

        offset = (frame * m_frameSize) + idx;   // offset = (frame_number * size_of_a_frame) + item_idx

        if (offset >= m_arraySize)
        {
            return 0;
        }

        return *(m_pBuffer + offset);
    }

    void setSample(unsigned int idx, unsigned int frame, T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_frameSize < 1 || idx > m_frameSize || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {
            return;
        }

        unsigned int offset;

        offset = (frame * m_frameSize) + idx;   // offset = (frame_number * size_of_a_frame) + item_index

        if (offset >= m_arraySize)
        {
            return;
        }

        *(m_pBuffer + offset) = value;
    }

    void resetReadIndex()
    {
        m_readIdx = 0;
    }

    void resetWriteIndex()
    {
        m_writeIdx = 0;
    }

    // read a single data item at the current  
    // read offset and then increment the offset.
    // return: the item value at that offset
    T readItem()
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_readIdx >= m_arraySize)
        {
            return 0;
        }

        return *(m_pBuffer + (m_readIdx++));
    }

    // read a single data item at 'index' offset.
    // return: the item value at that offset
    T readData(const unsigned long index)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (index >= m_arraySize)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        return *(m_pBuffer + index);
    }

    // read a block of size 'count' (items) starting at the current read offset,
    // and then update the current read offset. return the updated read offset
    unsigned long readDataBlock(T buf[], const unsigned int numItems)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_readIdx + numItems) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numItems; x++)
            buf[x] = *(m_pBuffer + x);

        m_readIdx += numItems;

        return m_readIdx;
    }

    // read a block of size 'numItems' (data items)
    // starting at the 'startPos' read offset.
    // return: start offset + number of items read
    unsigned long readDataBlock(T buf[], const unsigned int startPos, const unsigned int numItems)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((startPos + numItems) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numItems; x++)
            buf[x] = *(m_pBuffer + (startPos + x));

        return startPos + numItems;
    }

    // write a single data item at the current write offset, and
    // then increment the offset, return the updated write offset.
    unsigned long writeData(const T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_writeIdx >= (unsigned long) m_arraySize)
        {
            return 0;
        }

        *(m_pBuffer + (m_writeIdx++)) = value;

        return m_writeIdx;
    }

    // write a single data item at 'index' write offset.
    // return: the write offset    
    unsigned long writeDataBlock(unsigned long index, const T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (index >= (unsigned long)m_arraySize)
        {
            return 0;
        }

        *(m_pBuffer + (index++)) = value;

        return index;
    }

    // write a block of size 'count' (items)
    // starting at the current write offset,
    // and then update the current write offset.
    // return:  the updated write offset.
    unsigned long writeDataBlock(const T buf[], const unsigned int numItems)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_writeIdx + numItems) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numItems; x++)
            *(m_pBuffer +  (m_writeIdx + x)) = buf[x];

        m_writeIdx += numItems;

        return m_writeIdx;
    }

    // write a block of size 'count' (items)
    // starting at the current write offset,
    // and then update the current write offset.
    // return:  the updated write offset.
    unsigned long writeDataBlock(const void *pBuf, const unsigned int numItems)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_writeIdx + numItems) >= m_arraySize)
        {
            return 0;
        }

        auto bytesPerItem = sizeof(T);

        auto byteCnt = (numItems * bytesPerItem);

        memcpy((m_pBuffer + m_writeIdx), pBuf, byteCnt);

        m_writeIdx += numItems;

        return m_writeIdx;
    }


    // write a block of size 'numItems' (data items)
    // starting at the 'startPos' write offset
    // return: start offset + number of items written
    unsigned long writeDataBlock(T buf[], const unsigned int startPos, const unsigned int numItems)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((startPos + numItems) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numItems; x++)
            *(m_pBuffer +  (startPos + x)) = buf[x];

        return startPos + numItems;
    }

};



#endif // _DATA_BUFFER_CLASS_H
