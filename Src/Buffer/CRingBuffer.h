//****************************************************************************
// FILE:    CRingBuffeer.h
//
// DEESC:   Buffer class with configurable data type 
//
// AUTHOR:  Russ Barker
//


#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_


#include "../Logging/Logging.h"

#include <mutex>
#include <string>

#include <vector>

#include <cstring>


//#define DIAPLAY_CONSOLE_LOG_MESSAGES

template <typename T> class CRingBuffer
{
    T                       *m_pDataBuffer;

    size_t                  m_bufferSize;

    size_t                  m_entriesPerBlock;

    volatile size_t         m_currentDataSize;

    volatile size_t         m_writeIdx;
    volatile size_t         m_readIdx;
    
    std::mutex              m_ioMutex;

  protected:

    void free()
    {
        if (m_pDataBuffer != nullptr)
        {
            free(m_pDataBuffer);

            m_pDataBuffer = nullptr;
        }
    }

    bool allocateBuffer(const size_t numEntries)
    {
        free();

        if (numEntries > 0)
        {
            unsigned int bytesPerEntry = (sizeof(T));
            
            m_pDataBuffer = (T*) calloc(numEntries, bytesPerEntry);

            if (m_pDataBuffer == nullptr)
            {
                LogDebug("[CRingBuffer:{}] Invalid data buffer pointer ", __func__);
                return false;
            }

            m_bufferSize = numEntries;
			
            return true;
        }

        return false;
    }

    // Delete (remove) numEntries starting at current pos.
    // Wrap read index if needed (ie: if >= max size).
    volatile bool erase(const size_t numEntries = 1)
    {
        if (m_pDataBuffer == nullptr || m_bufferSize < 1 || m_currentDataSize < 1)
        {
            LogDebug("[CRingBuffer:{}] Invalid param ", __func__);
            return false;
        }

        if (numEntries == 1)
        {
            if (m_readIdx == m_writeIdx)
            {
                LogDebug("[CRingBuffer:{}] Nothing to delete ", __func__);
                m_readIdx = 0;
                m_writeIdx = 0;
                m_currentDataSize = 0;
                return true;
            }

            m_readIdx++;

            if (m_readIdx >= m_bufferSize)
            {
                m_readIdx = 0;
            }

            if (m_currentDataSize > 0)
            {
                m_currentDataSize--;
            }
        }
        else
        {
            // If numEntries to delete is too large
            // delete everything we have.
            if (numEntries > (int) m_bufferSize)
            {
                LogError
                (
                    "[CRingBuffer:{}] numEnties ({}) > maxBufSize ({})  - flushing buffer ", 
                    __func__,
                    numEntries,
                    m_bufferSize
                );
                m_readIdx = 0;
                m_writeIdx = 0;
                m_currentDataSize = 0;
                return false;
            }
            
            if (numEntries > m_currentDataSize)
            {
                LogDebug
                (
                    "[CRingBuffer:{}] numEnties ({}) > currDataSiZe ({}) ", 
                    __func__,
                    numEntries,
                    m_currentDataSize
                );
                m_readIdx = 0;
                m_writeIdx = 0;
                m_currentDataSize = 0;
            }
            else
            {
                if ((m_readIdx + numEntries) >= m_bufferSize) 
                {
                    // the new read index would be > bufSize...
                    size_t wrapAmount = ((m_readIdx + numEntries) - m_bufferSize);

                    // so set read index to 0 + wrapAmount
                    m_readIdx = wrapAmount;
                }
                else
                {
                    // update the read index
                    m_readIdx += numEntries;
                }

                // Update the number of current entries (depth)
                if (m_currentDataSize > numEntries)
                {
                    m_currentDataSize -= numEntries;
                }
                else
                {
                    m_currentDataSize = 0;
                }
            }
        }

        return true;
    }

  public:

    CRingBuffer(const size_t numEntries = 0)
    {
        m_pDataBuffer = nullptr;

        m_entriesPerBlock = 1;
        m_bufferSize = 0;

		m_currentDataSize = 0;
        m_writeIdx = 0;
        m_readIdx = 0;

        if (numEntries > 0)
        {
            if (allocateBuffer(numEntries) == true)
                m_bufferSize = numEntries;
        }
    }

    CRingBuffer(const size_t entriesPerBlock, const size_t numBlocks)
    {
        m_pDataBuffer = nullptr;

        m_entriesPerBlock = entriesPerBlock;
        m_bufferSize = 0;

		m_currentDataSize = 0;
        m_writeIdx = 0;
        m_readIdx = 0;

        if (entriesPerBlock > 0 && numBlocks > 0)
        {
            auto numEntries = (entriesPerBlock * numBlocks);
            if (allocateBuffer(numEntries) == true)
                m_bufferSize = numEntries;
        }
    }

    ~CRingBuffer()
    {
        free();

        m_entriesPerBlock = 0;
        m_bufferSize = 0;

		m_currentDataSize = 0;
        m_writeIdx = 0;
        m_readIdx = 0;
    }

    // Set the maximum number of entries that can be stored in the entry array.
	// NOTE: Doing this will flush (zero out) the data buffer.
    bool setMaxBufferSize(const size_t numEntries)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return allocateBuffer(numEntries);
    }
	
    // Set the maximum number of entries that can be stored in the entry array.
	// NOTE: Doing this will flush (zero out) the data buffer.
    bool setMaxBufferSize(const size_t entriesPerBlock, const size_t numBlocks)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (entriesPerBlock > 0 && numBlocks > 0)
        {
            auto numEntries = (entriesPerBlock * numBlocks);
			
            auto status = allocateBuffer(numEntries);

            if (status == true)
            {
                m_entriesPerBlock = entriesPerBlock;
                m_bufferSize = numEntries;
            }

            return status;
        }

        LogDebug("[CRingBuffer:{}] Invalid param ", __func__);

        return false;
    }
	
    // Get the maximum number of entries that can be 
    // stored in the buffer (storage array).
    int getMaxBufferSize()
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return (int) m_bufferSize;
    }

    // Get the current (active) number of entries in 
    // the buffer (storage array).
    volatile unsigned int getCurrentDataSize()
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return (unsigned int) m_currentDataSize;
    }

	// Flush (zero out) the data buffer.
    volatile bool flush(const bool bZeroOutBuffer = false)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

		m_currentDataSize = 0;
        m_writeIdx = 0;
        m_readIdx = 0;

        if (bZeroOutBuffer == false)
        {
            return true;
        }

        if (m_bufferSize > 0)
        {
			memset(m_pDataBuffer, 0, (m_bufferSize * sizeof(T)));
			
            return true;
        }

        LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);

        return false;
    }

    // Read "blockSize" entries from the data block (entry array).
    volatile int readBlock
        (
            T *pTargetBuff, 
            const size_t blockSize, 
            const bool deleteEntries = false
        )
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (m_pDataBuffer == nullptr || pTargetBuff == nullptr)
        {
            LogDebug("[CRingBuffer:{}] Invalid param ", __func__);
            return -1;
        }

        if (m_bufferSize < 1 || blockSize > m_bufferSize)
        {
            LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);
            return -1;
        }

        if (m_currentDataSize < 1)
            return 0;

        size_t numEntriesToRead = blockSize;

		unsigned int currentReadIdx = (unsigned int) m_readIdx;
		unsigned int entriesRead = 0;

        // copy the data from the data buffer to the target buffer
        while (entriesRead < numEntriesToRead)
		{
			if (currentReadIdx >= m_bufferSize)
            {
				currentReadIdx = 0;
            }

			if (currentReadIdx == m_writeIdx)
            {
                // No data in buffer to read
				break;
            }

            try
            {
                T valueRead = *(m_pDataBuffer + currentReadIdx);

                *(pTargetBuff + entriesRead) = valueRead;
            }
            catch(...)
            {
                LogDebug("[CRingBuffer:{}] Unknown exception ", __func__);
                break;
            }

			// If true, delete the current entry
			if (deleteEntries == true)
			{
                erase(1);
            }
			
            currentReadIdx++;

			entriesRead++;
		}

        return (int) entriesRead;
    }

    // Read "blockSize" entries from the data block (entry array).
    volatile int readBlock
        (
            T *pTargetBuff, 
            const size_t entriesPerBlock,       // this is (more or less) numChannels
            const size_t blockSize,             // this is (more or less) numFrames
            const bool deleteEntries = false
        )
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (m_pDataBuffer == nullptr || pTargetBuff == nullptr)
        {
            LogDebug("[CRingBuffer:{}] Invalid param ", __func__);
            return -1;
        }

        if (m_bufferSize < 1 || blockSize > m_bufferSize)
        {
            LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);
            return -1;
        }

        if (m_currentDataSize < 1)
        {
            LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);
            return 0;
        }

        size_t numEntriesToRead = (entriesPerBlock * blockSize);

		unsigned int currentReadIdx = (unsigned int) m_readIdx;
		unsigned int entriesRead = 0;

        // copy the data from the data buffer to the target buffer
        while (entriesRead < numEntriesToRead)
		{
			if (currentReadIdx >= m_bufferSize)
            {
				currentReadIdx = 0;
            }

			if (currentReadIdx == m_writeIdx)
            {
                // No data in buffer to read
				break;
            }
			
            try
            {
                T valueRead = *(m_pDataBuffer + currentReadIdx);

                *(pTargetBuff + entriesRead) = valueRead;
            }
            catch(...)
            {
                LogDebug("[CRingBuffer:{}] Unknown exception ", __func__);
                break;
            }
			
			// If true, delete the current entry
			if (deleteEntries == true)
			{
                erase(1);
            }
			
            currentReadIdx++;

			entriesRead++;
		}

        return (int) entriesRead;
    }

    // Write "blockSize" entries to the data block (entry array).
    volatile int writeBlock
        (
            const T *pBuff, 
            const size_t blockSize, 
            bool bOverwrite = false
        )
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (m_pDataBuffer == nullptr || pBuff == nullptr)
        {
            LogDebug("[CRingBuffer:{}] Invalid param ", __func__);
            return -1;
        }

        if (m_bufferSize < 1 || blockSize >= m_bufferSize)
        {
            LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);
            return -2;
        }

		unsigned int entriesWritten = 0;

        // copy the data from the input buffer to the data buffer
        while (entriesWritten < blockSize)
		{
			if (m_writeIdx == m_readIdx && m_currentDataSize > 0)
			{
				if (bOverwrite == false)
				{
					// overwrite flag = false, so break
                    LogDebug("[CRingBuffer:{}] Buffer over-flow ", __func__);
    				return -5;
				}
				else
				{
					// overwrite flag = true, so update read index
					m_readIdx++;
					if (m_readIdx >= m_bufferSize)
                    {
						m_readIdx = 0;
                    }
				}
			}

            try
            {
                T valueToWrite = *(pBuff + entriesWritten);

                *(m_pDataBuffer + m_writeIdx) = valueToWrite;
            }
            catch(...)
            {
                LogDebug("[CRingBuffer:{}] Unkown exception ", __func__);
                return -6;
            }
			
            if (m_currentDataSize < m_bufferSize)
            {
                m_currentDataSize++;
            }
            else
            {
				if (bOverwrite == false)
				{
                    LogDebug
                    (
                        "[CRingBuffer:{}] currSize: {} = maxBufSize: {}, overWrite = false ", 
                        __func__,
                        m_currentDataSize,
                        m_bufferSize
                    );
                }
                else
                {
                    LogTrace
                    (
                        "[CRingBuffer:{}] currSize: {} = maxBufSize: {} - buffer full ", 
                        __func__,
                        m_currentDataSize,
                        m_bufferSize
                    );
                }
            }

            m_writeIdx++;
			if (m_writeIdx >= m_bufferSize)
            {
				m_writeIdx = 0;
            }

			entriesWritten++;
		}
		

        return entriesWritten;
    }

    // Write "blockSize" entries to the data block (entry array).
    volatile int writeBlock
        (
            const T *pBuff, 
            const size_t entriesPerBlock,           // this is (more or less) numChannels
            const size_t blockSize,                 // this is (more or less) numFrames
            bool bOverwrite = false
        )
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        if (m_pDataBuffer == nullptr || pBuff == nullptr)
        {
            LogDebug("[CRingBuffer:{}] Invalid param ", __func__);
            return -1;
        }

        if (m_bufferSize < 1 || blockSize >= m_bufferSize)
        {
            LogDebug("[CRingBuffer:{}] Invalid buffer size ", __func__);
            return -2;
        }

        size_t writeSize = (entriesPerBlock * blockSize);

        if (writeSize < 1 || writeSize >= m_bufferSize)
        {
            LogDebug("[CRingBuffer:{}] Invalid write size ", __func__);
            return -3;
        }

		unsigned int entriesWritten = 0;

        // copy the data from the input buffer to the data buffer
        while (entriesWritten < writeSize)
		{
			if (m_writeIdx == m_readIdx && m_currentDataSize > 0)
			{
				if (bOverwrite == false)
				{
					// overwrite flag = false, so break
                    LogDebug("[CRingBuffer:{}] Buffer over-flow ", __func__);
    				-5;
				}
				else
				{
					// overwrite flag = true, so update read index
					m_readIdx++;
					if (m_readIdx >= m_bufferSize)
                    {
						m_readIdx = 0;
                    }
				}
			}
			
            try
            {
                T valueToWrite = *(pBuff + entriesWritten);

                *(m_pDataBuffer + m_writeIdx) = valueToWrite;
            }
            catch(...)
            {
                LogDebug("[CRingBuffer:{}] Unkown exception ", __func__);
                return -6;
            }
			
            if (m_currentDataSize < m_bufferSize)
            {
                m_currentDataSize++;
            }
            else
            {
				if (bOverwrite == false)
				{
                    LogDebug
                    (
                        "[CRingBuffer:{}] entriesPerBlock: {}, currSize: {} = maxBufSize: {}, overWrite = false ", 
                        __func__,
                        entriesPerBlock,
                        m_currentDataSize,
                        m_bufferSize
                    );
                }
                else
                {
                    LogTrace
                    (
                        "[CRingBuffer:{}] entriesPerBlock: {}, currSize: {},  maxBufSize: {} ", 
                        __func__,
                        entriesPerBlock,
                        m_currentDataSize,
                        m_bufferSize
                    );
                }
            }

            m_writeIdx++;
			if (m_writeIdx >= m_bufferSize)
            {
				m_writeIdx = 0;
            }

			entriesWritten++;
		}
		
        return entriesWritten;
    }

    // Delete (remove) 1 entry.
    volatile bool deleteEntry()
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return erase(1);
    }

    // Delete (remove) entries, from  start to 1 less than end. 
    // Range is NOT inclusive of end.
    volatile bool deleteBlock(unsigned int numEntries)
    {
        std::lock_guard<std::mutex> lock{m_ioMutex};

        return erase(numEntries);
    }
};

#endif // _RING_BUFFER_H_
