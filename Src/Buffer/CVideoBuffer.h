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


template <class T> 
class CSimpleVideoBuffer : public CErrorHandler
{
    // type 'T' = data size of 1 pixel

    unsigned int    m_frameWidth;   // video frame width (in pixels)
    unsigned int    m_frameHeight;  // video frame height 

    unsigned int    m_blockSize;    // number of video frames per data block (default 1)
    unsigned int    m_frameSize;    // frameSize = total numner of pixels in a block

    unsigned long   m_nVideoFormat; // 0 = uncopressed/raw video

    unsigned long   m_readIdx;      // current frame/pixel read offset
    unsigned long   m_writeIdx;     // current frame/pixel write offset

    bool            m_bAllocated;   // whether the pixel data buffer is allocated

    T*              m_pBuffer;

    std::mutex      m_ioLock;

  public:

    CSimpleVideoBuffer() :
        m_bAllocated(false)
    {
        m_blockSize     = 1;
        m_frameSize     = 0;

        m_readIdx       = 0;
        m_writeIdx      = 0;

        m_pBuffer       = nullptr;
    }

    ~CSimpleVideoBuffer()
    {
        m_blockSize     = 1;
        m_frameSize     = 0;

        m_readIdx       = 0;
        m_writeIdx      = 0;

        free();
    }

    void setResolution(unsigned int frameWidth, unsigned int frameHeight)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_frameWidth = frameWidth;
        m_frameHeight = frameHeight;
    }

    void setFrameWidth(unsigned int frameWidth)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_frameWidth = frameWidth;
    }

    void setFrameHeight(unsigned int frameHeight)
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

    void setFramesPerBlock(unsigned int blockSize)
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

    bool alloc(unsigned int blockSize = 0)
    {
        if (blockSize > 0)
            m_blockSize = blockSize;

        if (m_blockSize < 1)
        {
             return false;
        }

        if (m_frameWidth > 0 && m_frameHeight > 0 && m_nVideoFormat == 0)
        {
            m_frameSize = (m_frameWidth * m_frameHeight);   // frameSize = total numner of pixels in the buffer
        }

        m_pBuffer = (T*) calloc(m_frameSize, sizeof(T));

        if (m_pBuffer == nullptr)
        {
            m_frameSize = 0;

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

        m_bAllocated = false;
    }

    unsigned int getLength()
    {
        return m_frameSize;
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {            
            return;
        }

        if (m_frameSize < 1)
        {
            return;
        }

        memset(m_pBuffer, 0, (sizeof(T) * m_frameSize));

        m_readIdx  = 0;
        m_writeIdx = 0;
    }

    T *getBuffPtr()
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        return (T *) m_pBuffer;
    }

    T * getFramePtr(const unsigned int frame)
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

        auto offset = frame * m_frameSize; 

        return (((T *) m_pBuffer) + offset);
    }

    void lockBuffer()
    {
        m_ioLock.lock();
    }

    void unlockBuffer()
    {
        m_ioLock.unlock();
    }

    bool getFrame(T* target, unsigned int frame)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {            
            return false;
        }

        unsigned int offset;

        offset = m_frameSize * frame;   

        memcpy(target, (((T *) m_pBuffer) + offset), (sizeof(t) * m_frameSize));

        return true;;
    }

    bool setFrame(unsigned int frame, T* value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_frameSize < 1 || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {            
            return false;
        }

        unsigned int offset;

        offset = m_frameSize * frame;   

        memcpy((((T *) m_pBuffer) + offset), value, (sizeof(t) * m_frameSize));

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
    // return: the pixel value at that offset
    T readPixel()
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_readIdx >= (m_frameSize * m_blockSize))
        {
            return 0;
        }

        return *(((T *) m_pBuffer) + (m_readIdx++));
    }

    // read a single pixel at 'index' offset.
    // return: the pixel value at that offset
    T readPixel(const unsigned long index)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            // return zero pixel
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (index >= (m_frameSize * m_blockSize))
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        return *(((T *) m_pBuffer) + index);
    }

    // read a single pixel at 'index' offset.
    // return: the pixel value at that offset
    T readPixel(const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated || m_frameWidth < 1 || m_frameHeight < 1 || m_frameSize < 1 || m_nVideoFormat != 0)
        {
            // return zero pixel
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (m_frameWidth <= xPos || m_frameHeight <= yPos)
        {
            // return zero pixel
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        unsigned long index = ((frame * m_frameSize) + (yPos * m_frameWidth) + yPos);

        if (index >= (m_frameSize * m_blockSize))
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        return *(((T *) m_pBuffer) + index);
    }

    // read a block of size 'count' (pixels) starting at the current read offset,
    // and then update the current read offset. return the updated read offset
    unsigned long readPixels(T buf[], const unsigned int count)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_readIdx + count) >= (m_frameSize * m_blockSize))
        {
            return 0;
        }

        for (unsigned int x = 0; x < count; x++)
        {
            buf[x] = *(((T *) m_pBuffer) + x);
        }
        
        m_readIdx += count;

        return m_readIdx;
    }

    // read a block of size 'm_frameSize' (pixels)
    // starting at the 'frame' number read offset.
    // return: m_frameSize
    unsigned long readFrame(T buf[], const unsigned int frame = 0)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (frame >= m_blockSize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < m_frameSize; x++)
            buf[x] = *(((T *) m_pBuffer) + ((m_frameSize * frame) + x));

        return m_frameSize;
    }

    // write a single pixel at the current write offset, and
    // then increment the offset, return the updated write offset.
    unsigned long writePixel(const T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_writeIdx >= (unsigned long) (m_frameSize * m_blockSize))
        {
            return 0;
        }

        *(((T *) m_pBuffer) + (m_writeIdx++)) = value;

        return m_writeIdx;
    }

    // write a single pixel at 'index' write offset.
    // return: the write offset    
    unsigned long writePixel(const T value, unsigned long index)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (index >= (unsigned long) (m_frameSize * m_blockSize))
        {
            return 0;
        }

        *(((T *) m_pBuffer) + (index++)) = value;

        return index;
    }

    // write a single pixel at 'index' offset.
    // return: the write offset    
    unsigned long writePixel(const T value, const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated || m_frameWidth < 1 || m_frameHeight < 1 || m_frameSize < 1 || m_nVideoFormat != 0)
        {
            return 0;
        }

        if (m_frameWidth <= xPos || m_frameHeight <= yPos)
        {
            return 0;
        }

        unsigned long index = ((frame * m_frameSize) + (yPos * m_frameWidth) + yPos);

        if (index >= (m_frameSize * m_blockSize))
        {
            return 0;
        }

        *(((T *) m_pBuffer) + index) = value;

        return index;
    }

    // write a block of size 'count' (pixels)
    // starting at the current write offset,
    // and then update the current write offset.
    // return:  the updated write offset.
    unsigned long writePixels(const T buf[], const unsigned int count)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_writeIdx + count) >= (m_frameSize * m_blockSize))
        {
            return 0;
        }

        for (unsigned int x = 0; x < count; x++)
            *(((T *) m_pBuffer) +  (m_writeIdx + x)) = buf[x];

        m_writeIdx += count;

        return m_writeIdx;
    }

    // write a block of size 'm_frameSize' (pixels)
    // starting at the 'frame' * 'm_frameSize' write offset
    // return: m_frameSize
    unsigned long writeFrame(T buf[], const unsigned int frame)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (frame >= m_blockSize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < m_frameSize; x++)
            *(((T *) m_pBuffer) + ((m_frameSize * frame) + x)) = buf[x];

        return m_frameSize    
    }

};

template <class T> class CVideoBufferBase
{
  protected:

    // type 'T' = data size of 1 pixel

    T               *m_pBuffer;

    unsigned int    m_frameWidth;   // video frame width (in pixels)
    unsigned int    m_frameHeight;  // video frame height 

    unsigned long   m_nVideoFormat; // 0 = uncopressed/raw video

    unsigned int    m_pixelsPerFrame;
    
    unsigned int    m_framesPerBlock;
    
    unsigned int    m_totalNumPixels;

    bool            m_bAllocated;

#ifdef SUPPORT_FILE_IO
    std::string        m_sInputFile;
    std::string        m_sOutputFile;

    eVideoFileType_def m_eInputType;
    eVideoFileType_def m_eOutputType;
    
    CVideoFileIO      *m_pFileReader;
    CVideoFileIO      *m_pFileWriter;
#endif

  public:

    // Allocate an empty pixel buffer
    void allocBuffer()
    {
        if (m_pixelsPerFrame == 0 || m_framesPerBlock == 0)
        {
            return;
        }

        m_totalNumPixels = (m_pixelsPerFrame * m_framesPerBlock)

        if (m_pBuffer != nullptr)
            free(m_pBuffer);

        m_pBuffer = (T *) calloc(m_totalNumPixels, sizeof(T));

        if (m_pBuffer == nullptr)
        {
            m_totalNumPixels = 0;

            return;
        }

        m_bAllocated = true;
    }

    void alloc(const unsigned int size = 0)
    {
        if (size > 0)
            m_pixelsPerFrame = size;

        allocBuffer();
    }

    void freeBuffer()
    {
        if (m_pBuffer == nullptr)
            ::free(m_pBuffer);

        m_pBuffer           = nullptr;
        m_bAllocated      = false;
        m_totalNumPixels = 0;
    }

    // Buffer base class constructor
    CVideoBufferBase()
    {
        m_pBuffer         = nullptr;
        m_frameWidth      = 0;
        m_frameHeight     = 0;
        m_pixelsPerFrame  = 0;
        m_framesPerBlock  = 1;
        m_totalNumPixels  = 0;
        m_nVideoFormat    = 0;
        m_bAllocated      = false;

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eVideoFileType_def::eFileType_unknown;
        m_eOutputType = eVideoFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif
    }

    CVideoBufferBase(const unsigned int frameSize) :
        m_pixelsPerFrame(frameSize)
    {
        m_pBuffer         = nullptr;
        m_frameWidth      = 0;
        m_frameHeight     = 0;
        m_framesPerBlock  = 1;
        m_totalNumPixels  = 0;
        m_nVideoFormat    = 0;
        m_bAllocated      = false;

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eVideoFileType_def::eFileType_unknown;
        m_eOutputType = eVideoFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif

        if (m_pixelsPerFrame < 1 || m_nVideoFormat != 0)
            return;

        allocBuffer();
    }

    CVideoBufferBase(const unsigned int width, const unsigned int height, unsigned long format = 0) :
        m_frameWidth(width),
        m_frameHeight(height),
        m_nVideoFormat(format)
    {
        m_pBuffer         = nullptr;
        m_pixelsPerFrame  = 0;
        m_framesPerBlock  = 1;
        m_totalNumPixels  = 0;
        m_bAllocated      = false;

        if (width > 0 && height > 0)
        {
            m_pixelsPerFrame = (width * height);
        }

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eVideoFileType_def::eFileType_unknown;
        m_eOutputType = eVideoFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif

        if (m_pixelsPerFrame < 1 || m_nVideoFormat != 0)
            return;

        allocBuffer();
    }

    // Buffer base class destructor
    ~CVideoBufferBase()
    {
#ifdef SUPPORT_FILE_IO
        if (m_pFileReader != nullptr)
            m_pFileReader->closeFile();

        if (m_pFileWriter != nullptr)
            m_pFileWriter->closeFile();
#endif

        if (m_pBuffer != nullptr)
            free(m_pBuffer);

        m_pBuffer = nullptr;
    }

    void setResolution(unsigned int frameWidth, unsigned int frameHeight)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_frameWidth = frameWidth;
        m_frameHeight = frameHeight;
    }

    void setFrameWidth(unsigned int frameWidth)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_frameWidth = frameWidth;
    }

    void setFrameHeight(unsigned int frameHeight)
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

    void setFramesPerBlock(unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_blockSize = blockSize;
    }

    unsigned int getFramesPerBock()
    {
        return m_framesPerBlock;
    }

    unsigned int getLength()
    {
        return (m_totalNumPixels);
    }

    bool initialized()
    {
        if (CVideoBufferBase<T>::m_totalNumPixels < 1 || CVideoBufferBase<T>::m_framesPerBlock < 1)
            return false;

        if (CVideoBufferBase<T>::m_pBuffer == nullptr)
            return false;

        if (!m_bAllocated)
            return false;

        return true;
    }

    // Get a pointer to the pixel buffer
    T *getBuffPtr()
    {
        return ((T *) m_pBuffer);
    }

    // Clear the pixel buffer (set it to zeros)
    void clear()
    {
        if (m_bAllocated == false || m_totalNumPixels < 1)
        {
            return;
        }

        if (m_pBuffer == nullptr)
        {
            return;
        }

        memset(m_pBuffer, 0, (m_totalNumPixels * sizeof(T)));
    }

#ifdef SUPPORT_FILE_IO
    // This method enables/disables reading from an input file
    // NOTE: File input & file output can't be used at the same time on the same buffer.
    bool readFromInputFile(const bool val)
    {
        if (m_pFileWriter != nullptr)
            return false;

        if (val)
        {
            if (m_pFileReader != nullptr)
                return false;

            m_eInputType = ::getVideoFileType(m_sInputFile);

            switch (m_eInputType)
            {
                case eFileType_raw:
                    {
                        m_pFileReader = (CVideoFileIO *) new CRawFileIO(m_numChls);
                        if (m_pFileReader == nullptr)
                            return false;
                    }
                    break;

                default:
                    return false;
            }

            bool status = m_pFileReader->openFile(eFileIoMode_def::eFileIoMode_input, m_sInputFile);
            if (!status)
                return false;
        }
        else if (m_pFileReader != nullptr)
        {
            m_pFileReader->closeFile();
            delete m_pFileReader;
            m_pFileReader = nullptr;
        }

        return true;
    }

    // This method enables/disables writing to an output file
    // NOTE: File input & file output can't be used at the same time on the same buffer.
    bool writeToOutputFile(const bool val)
    {
        if (m_pFileReader != nullptr)
            return false;

        if (val)
        {
            if (m_pFileWriter != nullptr)
                return false;

            m_eOutputType = ::getVideoFileType(m_sOutputFile);

            switch (m_eOutputType)
            {
                case eFileType_raw:
                    {
                        m_pFileWriter = (CVideoFileIO *)new CRawFileIO(m_numChls);
                        if (m_pFileWriter == nullptr)
                            return false;

                        ((CRawFileIO *)m_pFileWriter)->createInfoTextFile(true);
                    }
                    break;

                default:
                    return false;
            }

            bool status = m_pFileWriter->openFile(eFileIoMode_def::eFileIoMode_output, m_sOutputFile);
            if (!status)
                return false;
        }
        else if (m_pFileWriter != nullptr)
        {
            m_pFileWriter->closeFile();
            delete m_pFileWriter;
            m_pFileWriter = nullptr;
        }

        return true;
    }

    eVideoFileType_def getInputFileType()
    {
        return m_eInputType;
    }

    eVideoFileType_def getOutputFileType()
    {
        return m_eOutputType;
    }
#endif

    // Set the value of a pixel within the buffer
    bool setPixel(const T value, const size_t pixel, const size_t frame)
    {
        if (!initialized())
        {
            return false;
        }

        if (pixel >= m_pixelsPerFrame)
        {
            return false;
        }

        if (frame >= m_framesPerBlock)
        {
            return false;
        }

#ifdef SUPPORT_FILE_IO
        if (m_pFileWriter != nullptr)
            m_pFileWriter->writeFrame(value, pixel, frame);
#endif

        *(m_pBuffer + (frame * m_pixelsPerFrame) + pixel) = value;

        return true;
    }

    // read a single pixel at 'index' offset.
    // return: bool (success = true, error = false)
    bool setPixel(const T value, const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated || m_frameWidth < 1 || m_frameHeight < 1 || m_pixelsPerFrame < 1 || m_nVideoFormat != 0)
        {
            return false;
        }

        if (m_frameWidth <= xPos || m_frameHeight <= yPos || m_framesPerBlock < 1)
        {
            return false;
        }

        unsigned long index = ((frame * m_pixelsPerFrame) + (yPos * m_frameWidth) + yPos);

        if (index >= m_totalNumPixels)
        {
            return false;
        }

        *(((T *) m_pBuffer) + index) = value;

        return true;
    }

    // write a block of size 'm_pixelsPerFrame' (pixels)
    // starting at the 'frame' * 'm_pixelsPerFrame' write offset
    // return: m_pixelsPerFrame or 0 (error)
    unsigned long setFrame(T source[], const unsigned int frame = 0)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (frame >= m_framesPerBlock)
        {
            return 0;
        }

        for (unsigned int x = 0; x < m_pixelsPerFrame; x++)
        {
            *(((T *) m_pBuffer) + ((m_pixelsPerFrame * frame) + x)) = source[x];
        }

        return m_pixelsPerFrame    
    }

    // Get the value of a pixel within the buffer
    T getPixel(const size_t pixel, const size_t frame)
    {
        if (!initialized())
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (pixel >= m_pixelsPerFrame)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (frame >= m_framesPerBlock)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

#ifdef SUPPORT_FILE_IO
        if (m_pFileReader != nullptr)
        {
            int16_t value = 0;
            m_pFileReader->readFrame(value, pixel, frame);
            return value;
        }
#endif
        return m_pBuffer + (frame * m_pixelsPerFrame) + pixel);
    }

    // read a single pixel at 'index' offset.
    // return: the pixel value at that offset
    T getPixel(const unsigned long xPos, const unsigned long yPos, const unsigned long frame = 0)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated || m_frameWidth < 1 || m_frameHeight < 1 || m_pixelsPerFrame < 1 || m_nVideoFormat != 0)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        if (m_frameWidth <= xPos || m_frameHeight <= yPos || m_framesPerBlock < 1)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        unsigned long index = ((frame * m_pixelsPerFrame) + (yPos * m_frameWidth) + yPos);

        if (index >= m_totalNumPixels)
        {
            // return zeros
            T out;

            memset(&out, 0, sizeof(T));

            return out;
        }

        return *(((T *) m_pBuffer) + index);
    }

    // read a block of size 'm_pixelsPerFrame' (pixels)
    // starting at the 'frame' * 'm_pixelsPerFrame' write offset
    // return: m_pixelsPerFrame or 0 (error)
    unsigned long getFrame(T target[], const unsigned int frame = 0)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (frame >= m_framesPerBlock)
        {
            return 0;
        }

        for (unsigned int x = 0; x < m_pixelsPerFrame; x++)
        {
            target[x] = *(((T *) m_pBuffer) + ((m_pixelsPerFrame * frame) + x));
        }

        return m_pixelsPerFrame    
    }


};

#endif // _VIDEO_BUFFER_CLASS_H
