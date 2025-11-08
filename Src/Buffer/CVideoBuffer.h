//****************************************************************************
// FILE:    VideoBufferCls.h
//
// DESC:    An video buffer handler class.
//
// AUTHOR:  Russ Barker
//

#ifndef _VIDEO_BUFFER_CLASS_H
#define _VIDEO_BUFFER_CLASS_H

#include <malloc.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

#include "../Error/CError.h"


typedef std::vector<uint8_t>    VideoPixelData_def;


class CSimpleVideoBuffer :
    public CErrorHandler
{
    // type 'T' = data size of 1 pixel

    unsigned int                m_frameWidth;   // video frame width (in pixels)
    unsigned int                m_frameHeight;  // video frame height 

    unsigned int                m_blockSize;    // number of video frames per data block (default 1)
    unsigned int                m_frameSize;    // frameSize = total numner of pixels in a block

    unsigned int                m_nBitsPerPixel;

    unsigned long               m_nVideoFormat; // 0 = uncopressed/raw video

    unsigned long               m_readIdx;      // current frame/pixel read offset
    unsigned long               m_writeIdx;     // current frame/pixel write offset

    bool                        m_bAllocated;   // whether the pixel data buffer is allocated

    void                        *m_pBuffer;

    unsigned long               m_nBufferLen;

    std::vector<unsigned long>  m_frameDataLen;   // Length of actual frame data (in byts)

    std::mutex                  m_ioLock;

protected:

    void resetFrameLen()
    {
        for (auto i = m_frameDataLen.begin(); i != m_frameDataLen.end(); i++)
        {
            (*i) = 0;
        }
    }

public:

    CSimpleVideoBuffer() :
        m_bAllocated(false)
    {
        m_blockSize = 1;
        m_frameSize = 0;

        m_readIdx = 0;
        m_writeIdx = 0;

        m_nBitsPerPixel = 0;

        m_nVideoFormat = 0;

        m_pBuffer = nullptr;

        m_nBufferLen = 0;

        m_frameDataLen.clear();
        m_frameDataLen.resize(m_blockSize);
        resetFrameLen();
    }

    ~CSimpleVideoBuffer()
    {
        m_blockSize = 1;
        m_frameSize = 0;

        m_readIdx = 0;
        m_writeIdx = 0;

        m_frameDataLen.clear();

        free();
    }

    void setPixelSize(const unsigned int nBitsPerPixel)
    {
        m_nBitsPerPixel = nBitsPerPixel;
    }

    void setVideoFormat(const unsigned int nFmt)
    {
        m_nVideoFormat = nFmt;
    }

    void setResolution(const unsigned int frameWidth, const unsigned int frameHeight)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_frameWidth = frameWidth;
        m_frameHeight = frameHeight;
    }

    void setFrameWidth(const unsigned int frameWidth)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_frameWidth = frameWidth;
    }

    void setFrameHeight(const unsigned int frameHeight)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_frameHeight = frameHeight;
    }

    unsigned int getFrameWidth()
    {
        return m_frameWidth;
    }

    unsigned int getFrameHeight()
    {
        return m_frameHeight;
    }

    void setFramesPerBlock(const unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_blockSize = blockSize;

        m_frameDataLen.resize(blockSize);

        resetFrameLen();
    }

    unsigned int getFramesPerBlock()
    {
        return m_blockSize;
    }

    bool alloc(const unsigned int blockSize = 0)
    {
        if (blockSize > 0)
            m_blockSize = blockSize;

        if (m_blockSize < 1)
        {
            return false;
        }

        if (m_frameWidth > 0 && m_frameHeight > 0)
        {
            // frameSize = total numner of pixels in the buffer
            if (m_frameSize < 1)
            {
                m_frameSize = (m_frameWidth * m_frameHeight);
            }
        }

        if (m_nBitsPerPixel < 1 || m_frameSize < 1)
        {
            m_frameSize = 0;

            return false;
        }

        auto numPixelsPerBlock = (m_frameSize * m_blockSize);

        auto bufferSizeInBytes = ((numPixelsPerBlock * m_nBitsPerPixel) / 8);

        m_pBuffer = (void*) calloc(bufferSizeInBytes, sizeof(uint8_t));

        if (m_pBuffer == nullptr)
        {
            m_nBufferLen = 0;

            return false;
        }

        m_nBufferLen = bufferSizeInBytes;

        resetFrameLen();

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

        if (m_bAllocated == false)
        {
            return;
        }

        if (m_pBuffer != nullptr)
        {
            try
            {
                ::free(m_pBuffer);
            }
            catch (...)
            {
            }
        }

        m_pBuffer = nullptr;

        m_nBufferLen = 0;

        resetFrameLen();

        m_bAllocated = false;
    }

    unsigned int getFrameLength()
    {
        return m_frameSize;
    }

    void setFrameDataLen(const unsigned long nLen, const unsigned int nFrame = 0)
    {
        if (m_frameDataLen.size() > nFrame)
        {
            m_frameDataLen[nFrame] = nLen;
        }
    }

    unsigned long getFrameDataLen(const unsigned int nFrame = 0)
    {
        if (m_frameDataLen.size() > nFrame)
        {
            auto len = m_frameDataLen[nFrame];

            return  len;
        }

        return 0;
    }

    void clear()
    {
        if (m_frameSize < 1 || m_blockSize < 1 || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        memset(m_pBuffer, 0, m_nBufferLen);

        m_readIdx = 0;
        m_writeIdx = 0;

        resetFrameLen();
    }

    void* getBuffPtr()
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        return (void*) m_pBuffer;
    }

    void* getFramePtr(const unsigned int frame)
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        if (m_frameSize < 1)
        {
            return nullptr;
        }

        if (frame >= m_blockSize)
        {
            return nullptr;
        }

        unsigned int bytesPerFrame = ((m_nBitsPerPixel * m_frameSize) / 8);

        unsigned int offset = (bytesPerFrame * frame);

        return ((void*) (((uint8_t *) m_pBuffer) + offset));
    }

    void lockBuffer()
    {
        m_ioLock.lock();
    }

    void unlockBuffer()
    {
        m_ioLock.unlock();
    }

    bool getFrame(void* pTarget, const unsigned int frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pTarget == nullptr)
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        unsigned int bytesPerFrame = ((m_nBitsPerPixel * m_frameSize) / 8);

        unsigned int offset = (bytesPerFrame * frame);

        try
        {
            memcpy(pTarget, (((uint8_t*) m_pBuffer) + offset), bytesPerFrame);
        }
        catch (...)
        {
            return false;
        }

        return true;;
    }

    bool setFrame(const void* pSource, const unsigned int frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pSource == nullptr)
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        unsigned int bytesPerFrame = ((m_nBitsPerPixel * m_frameSize) / 8);

        unsigned int offset = (bytesPerFrame * frame);

        try
        {
            memcpy((((uint8_t*) m_pBuffer) + offset), pSource, bytesPerFrame);

            m_frameDataLen[frame] = bytesPerFrame;
        }
        catch (...)
        {
            return false;
        }

        return true;;
    }

    void resetReadIndex()
    {
        m_readIdx = 0;
    }

    void resetWriteIndex()
    {
        m_writeIdx = 0;
    }

    // read a single pixel at the current read offset 
    // and then increment the offset.
    int readPixel(void *pTrgtPixel, const unsigned int frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 1)
        {
            return -1;
        }

        if (pTrgtPixel == nullptr)
        {
            return -1;
        }

        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_readIdx >= m_frameSize)
        {
            m_readIdx = 0;
            return -1;
        }

        unsigned int bytesPerFrame = ((m_nBitsPerPixel * m_frameSize) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + ((m_readIdx * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy(pTrgtPixel, (((uint8_t*) m_pBuffer) + offset), pixelSize);
        }
        catch (...)
        {
            return -1;
        }

        m_readIdx++;

        return m_readIdx;
    }

    // read a single pixel at 'index' offset.
    bool readPixelAt(void* pTrgtPixel, const unsigned long index, const unsigned int frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || index > m_frameSize || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pTrgtPixel == nullptr)
        {
            return false;
        }

        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (index >= (m_frameSize * m_blockSize))
        {
            return false;
        }

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + ((index * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy(pTrgtPixel, (((uint8_t*) m_pBuffer) + offset), pixelSize);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    // read a single pixel at 'index' offset.
    // return: the pixel value at that offset
    bool readPixel(void* pTrgtPixel, const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || xPos > m_frameWidth || yPos > m_frameHeight || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pTrgtPixel == nullptr)
        {
            return false;
        }

        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int bytesPerScanLine = ((m_frameWidth * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + (yPos * bytesPerScanLine) + ((xPos * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy(pTrgtPixel, (((uint8_t*)m_pBuffer) + offset), pixelSize);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    // read a block of size 'count' (pixels) starting at the current read offset,
    // and then update the current read offset. return the updated read offset
    int readPixels(void* pTrgtPixel, const unsigned int count, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return -1;
        }

        if (pTrgtPixel == nullptr)
        {
            return -1;
        }

        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if ((m_readIdx + count) >= (m_frameSize * m_blockSize))
        {
            return -1;
        }

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + ((m_readIdx * m_nBitsPerPixel) / 8));

        unsigned int copySize = ((((m_nBitsPerPixel * count) + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy(pTrgtPixel, (((uint8_t*) m_pBuffer) + offset), copySize);
        }
        catch (...)
        {
            return false;
        }

        m_readIdx += count;

        return m_readIdx;
    }

    // write a single pixel at the current write offset, and
    // then increment the offset, return the updated write offset.
    int writePixel(const void* pSrctPixel, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return -1;
        }

        if (pSrctPixel == nullptr)
        {
            return -1;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_writeIdx >= (unsigned long) m_frameSize)
        {
            m_writeIdx = 0;
            return -1;
        }

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + ((m_writeIdx * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy((((uint8_t*) m_pBuffer) + offset), pSrctPixel, pixelSize);
        }
        catch (...)
        {
            return false;
        }

        m_writeIdx++;

        return m_writeIdx;
    }

    // write a single pixel at 'index' write offset.
    // return: the write offset    
    bool writePixelAt(const void* pSrctPixel, const unsigned long index, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pSrctPixel == nullptr)
        {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        if (index >= (unsigned long) m_frameSize)
        {
            return false;
        }

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + ((index * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy((((uint8_t*)m_pBuffer) + offset), pSrctPixel, pixelSize);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    // write a single pixel at 'index' offset.
    // return: the write offset    
    bool writePixel(const void* pSrctPixel, const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || xPos > m_frameWidth || yPos > m_frameHeight || m_nBitsPerPixel < 8)
        {
            return false;
        }

        if (pSrctPixel == nullptr)
        {
            return false;
        }

        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned int bytesPerScanLine = ((m_frameWidth * m_nBitsPerPixel) / 8);

        unsigned int offset = ((frame * bytesPerFrame) + (yPos * bytesPerScanLine) + ((xPos * m_nBitsPerPixel) / 8));

        unsigned int pixelSize = (((m_nBitsPerPixel + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy((((uint8_t*) m_pBuffer) + offset), pSrctPixel, pixelSize);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

    // write a block of size 'count' (pixels)
    // starting at the current write offset,
    // and then update the current write offset.
    // return:  the updated write offset.
    int writePixels(const void* pSrctPixel, const unsigned int count, const unsigned long frame = 0)
    {
        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated || count > m_frameSize || m_nBitsPerPixel < 8)
        {
            return -1;
        }

        if (pSrctPixel == nullptr)
        {
            return -1;
        }

        std::lock_guard<std::mutex> lock(m_ioLock);

        if ((m_writeIdx + count) > (unsigned long) m_frameSize)
        {
            return -1;
        }

        unsigned int bytesPerFrame = ((m_frameSize * m_nBitsPerPixel) / 8);

        unsigned offset = ((frame * bytesPerFrame) + ((m_writeIdx * m_nBitsPerPixel) / 8));

        unsigned int copySize = ((((m_nBitsPerPixel * count) + 8) - 1) / 8);       // divide by 8 and round up

        try
        {
            memcpy((((uint8_t*)m_pBuffer) + offset), pSrctPixel, copySize);
        }
        catch (...)
        {
            return false;
        }

        m_writeIdx += count;

        return m_writeIdx;
    }

};


#endif // _VIDEO_BUFFER_CLASS_H
