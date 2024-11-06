//****************************************************************************
// FILE:    AudioBufferCls.h
//
// DESC:    An audio buffer handler class, that supports getting audio data 
//          from, or writing daata to an audio file.
//
// AUTHOR:  Russ Barker
//

#ifndef _AUDIO_BUFFER_CLASS_H
#define _AUDIO_BUFFER_CLASS_H

#include <malloc.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>


template <class T> class CSimpleAudioBuffer : public std::vector<T>
{
    unsigned int  m_numChannels;  // number of channels to buffer
    unsigned int  m_blockSize;    // if interleaved, number of frames per block, if not, number of samples per channel
    unsigned int  m_arraySize;    // arraySize = total numner of samples in a block

    unsigned long m_readIdx;      // current frame/sample read offset
    unsigned long m_writeIdx;     // current frame/sample write offset

    bool          m_bInterleaved; // whether sample data is interleaved in the buffer
    bool          m_bAllocated;   // whether the sample data buffer is allocated

    std::mutex    m_ioLock;

  public:

    CSimpleAudioBuffer(bool bInterleaved = false) :
        m_numChannels(1),
        m_bAllocated(false)
    {
        m_bInterleaved = bInterleaved;
        m_blockSize    = 0;
        m_arraySize    = 0;

        m_readIdx      = 0;
        m_writeIdx     = 0;
    }

    void setNumChannels(unsigned int numChls)
    {
        if (m_bAllocated)
        {
            return;
        }
        
        m_numChannels = numChls;
    }

    unsigned int getNumChannels()
    {
        return m_numChannels;
    }

    void setSamplesPerBlock(unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_blockSize = blockSize;
    }

    unsigned int getSamplesPerBlock()
    {
        return m_blockSize;
    }

    void setSize(unsigned int numChls, unsigned int blockSize)
    {
        if (m_bAllocated)
        {
            return;
        }

        m_numChannels = numChls;
        m_blockSize   = blockSize;
    }

    bool setInterleaved(bool bInterleaved)
    {
        if (m_bAllocated)
        {
            return false;
        }

        m_bInterleaved = bInterleaved;

        return true;
    }

    bool alloc(unsigned int blockSize = 0)
    {
        if (blockSize > 0)
            m_blockSize = blockSize;

        if (m_blockSize < 1)
        {
             return false;
        }

        if (m_numChannels < 1)
        {
            return false;
        }

        m_arraySize = (m_blockSize * m_numChannels); // arraySize = total numner of samples in a block
        this->resize(m_arraySize);

        if (this->size() < m_arraySize)
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

        this->clear();
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

        if (m_arraySize > this->size())
        {
            return;
        }

        memset((this->data()), 0, (sizeof(T) * m_arraySize));

        m_readIdx  = 0;
        m_writeIdx = 0;
    }

    T *getBuffPtr()
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        return ((T *)this->data());
    }


    // This function gets a pointer to the start 
    // of a frame or the start of a channel, 
    // depending on whether the data
    // is interleaved or not.
    T *getDataPtr(const unsigned index)
    {
        unsigned int offset;

        if (m_bInterleaved)
            offset = index * m_numChannels; // sample offset = index (frame number) * size of a frame
        else
            offset = index * m_blockSize;   // sample offset = index (channel number) * size of a channel block

        if (offset >= m_arraySize)
        {
            return nullptr;
        }

        return ((T *)(this->data() + (offset * sizeof(T))));
    }

    inline T *getChannelPtr(const unsigned int chan)
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        if (m_bInterleaved)
        {
            return nullptr;
        }

        if (chan >= m_numChannels)
        {
            return nullptr;
        }

        auto offset = chan * m_blockSize; // sample offset = channel number * size of a channel block

        if (offset >= this->size())
        {
            return nullptr;
        }

        return (((T *)(this->data())) + offset);
    }

    inline T *getFramePtr(const unsigned int frame)
    {
        if (!m_bAllocated)
        {
            return nullptr;
        }

        if (!m_bInterleaved)
        {
            return nullptr;
        }

        if (frame >= m_blockSize)
        {
            return nullptr;
        }

        auto offset = frame * m_numChannels; // sample offset = frame number * size of a frame

        if (offset >= this->size())
        {
            return nullptr;
        }

        return (((T *)(this->data())) + offset);
    }

    void lockBuffer()
    {
        m_ioLock.lock();
    }

    void unlockBuffer()
    {
        m_ioLock.unlock();
    }

    T getSample(unsigned int chan, unsigned int frame)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_numChannels < 1 || chan > m_numChannels || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {            
            return 0;
        }

        unsigned int offset;

        if (m_bInterleaved)
            offset = (frame * m_numChannels) + chan; // sample offset = (frame * size of a frame) + channel
        else
            offset = (chan * m_blockSize) + frame;   // sample offset = (channel * size of channel block) + frame

        if (offset >= this->size())
        {
            return 0;
        }

        return (this->at(offset));
    }

    void setSample(unsigned int chan, unsigned int frame, T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (m_numChannels < 1 || chan > m_numChannels || m_blockSize < 1 || frame > m_blockSize || !m_bAllocated)
        {
            return;
        }

        unsigned int offset;
        if (m_bInterleaved)
            offset = (frame * m_numChannels) + chan; // sample offset = (frame * size of frame) + channel
        else
            offset = (chan * m_blockSize) + frame;   // sample offset = (channel * size of channel block) + frame

        if (offset >= this->size())
        {
            return;
        }
        this->at(offset) = value;
    }

    void resetReadIndex()
    {
        m_readIdx = 0;
    }

    void resetWriteIndex()
    {
        m_writeIdx = 0;
    }

    // read a single sample at the current read offset 
    // and then increment the offset.
    // return: the sample value at that offset
    T readSample()
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_readIdx >= this->size())
        {
            return 0;
        }

        return (this->at(m_readIdx++));
    }

    // read a single sample at 'index' offset.
    // return: the sample value at that offset
    T readSample(const unsigned long index)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            // return zero sample
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

        return (this->at(index));
    }

    // read a block of size 'count' (samples) starting at the current read offset,
    // and then update the current read offset. return the updated read offset
    unsigned long readSamples(T buf[], const unsigned int count)
    {
        // make sure multiple functions are not modifying the buffer at the same time
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_readIdx + count) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < count; x++)
            buf[x] = this->at(m_readIdx + x);

        m_readIdx += count;

        return m_readIdx;
    }

    // read a block of size 'numSamples' (samples)
    // starting at the 'startPos' read offset.
    // return: start offset + number of samples read
    unsigned long readSampleBlock(T buf[], const unsigned int startPos, const unsigned int numSamples)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((startPos + numSamples) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numSamples; x++)
            buf[x] = this->at(startPos + x);

        return startPos + numSamples;
    }

    // write a single sample at the current write offset, and
    // then increment the offset, return the updated write offset.
    unsigned long writeSample(const T value)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if (m_writeIdx >= (unsigned long)m_arraySize)
        {
            return 0;
        }

        this->at(m_writeIdx++) = value;

        return m_writeIdx;
    }

    // write a single sample at 'index' write offset.
    // return: the write offset    
    unsigned long writeSample(unsigned long index, const T value)
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

        this->at(index++) = value;

        return index;
    }

    // write a block of size 'count' (samples)
    // starting at the current write offset,
    // and then update the current write offset.
    // return:  the updated write offset.
    unsigned long writeSamples(const T buf[], const unsigned int count)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((m_writeIdx + count) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < count; x++)
            this->at(m_writeIdx + x) = buf[x];

        m_writeIdx += count;

        return m_writeIdx;
    }

    // write a block of size 'numSamples' (samples)
    // starting at the 'startPos' write offset
    // return: start offset + number of samples written
    unsigned long writeSampleBlock(T buf[], const unsigned int startPos, const unsigned int numSamples)
    {
        std::lock_guard<std::mutex> lock(m_ioLock);

        if (!m_bAllocated)
        {
            return 0;
        }

        if ((startPos + numSamples) >= m_arraySize)
        {
            return 0;
        }

        for (unsigned int x = 0; x < numSamples; x++)
            this->at(startPos + x) = buf[x];

        return startPos + numSamples;
    }
};

template <class T> class CAudioBufferBase
{
  protected:

    T           *m_pBuff;
    unsigned int m_numChls;
    unsigned int m_samplesPerBlock;
    unsigned int m_totalNumSamples;
    bool         m_bAllocated;

#ifdef SUPPORT_FILE_IO
    std::string        m_sInputFile;
    std::string        m_sOutputFile;
    eAudioFileType_def m_eInputType;
    eAudioFileType_def m_eOutputType;
    CAudioFileIO      *m_pFileReader;
    CAudioFileIO      *m_pFileWriter;
#endif

  public:

    // Allocate an empty sample buffer
    void allocBuffer()
    {
        if (m_numChls == 0 || m_samplesPerBlock == 0)
        {
            return;
        }

        m_totalNumSamples = (m_numChls * m_samplesPerBlock);

        if (m_pBuff != nullptr)
            free(m_pBuff);

        m_pBuff = (T *)calloc(m_totalNumSamples, sizeof(T));

        if (m_pBuff == nullptr)
        {
            m_totalNumSamples = 0;

            return;
        }

        m_bAllocated = true;
    }

    void alloc(const unsigned int size = 0)
    {
        if (size > 0)
            m_samplesPerBlock = size;

        if (m_numChls == 0)
            m_numChls = 1;

        allocBuffer();
    }

    void freeBuffer()
    {
        if (m_pBuff == nullptr)
            ::free(m_pBuff);

        m_pBuff           = nullptr;
        m_bAllocated      = false;
        m_totalNumSamples = 0;
    }

    // Buffer base class constructor
    CAudioBufferBase()
    {
        m_pBuff           = nullptr;
        m_numChls         = 0;
        m_samplesPerBlock = 0;
        m_totalNumSamples = 0;
        m_bAllocated      = false;

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eAudioFileType_def::eFileType_unknown;
        m_eOutputType = eAudioFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif

        if (m_numChls == 0 || m_samplesPerBlock == 0)
            return;

        allocBuffer();
    }

    CAudioBufferBase(const unsigned int numChls) :
        m_numChls(numChls)
    {
        m_pBuff           = nullptr;
        m_samplesPerBlock = 0;
        m_totalNumSamples = 0;
        m_bAllocated      = false;

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eAudioFileType_def::eFileType_unknown;
        m_eOutputType = eAudioFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif

        if (m_numChls == 0 || m_samplesPerBlock == 0)
            return;

        allocBuffer();
    }

    CAudioBufferBase(const unsigned int numChls, const unsigned int samplesPerBlk) :
        m_numChls(numChls),
        m_samplesPerBlock(samplesPerBlk)
    {
        m_pBuff           = nullptr;
        m_totalNumSamples = 0;

#ifdef SUPPORT_FILE_IO
        m_sInputFile  = "";
        m_sOutputFile = "";
        m_eInputType  = eAudioFileType_def::eFileType_unknown;
        m_eOutputType = eAudioFileType_def::eFileType_unknown;
        m_pFileReader = nullptr;
        m_pFileWriter = nullptr;
#endif

        if (m_numChls == 0 || m_samplesPerBlock == 0)
            return;

        allocBuffer();
    }

    // Buffer base class destructor
    ~CAudioBufferBase()
    {
#ifdef SUPPORT_FILE_IO
        if (m_pFileReader != nullptr)
            m_pFileReader->closeFile();

        if (m_pFileWriter != nullptr)
            m_pFileWriter->closeFile();
#endif

        if (m_pBuff != nullptr)
            free(m_pBuff);

        m_pBuff = nullptr;
    }

    // Set number of channels
    void setNumChannels(const unsigned int numChls)
    {
        m_numChls = numChls;
    }

    unsigned int getNumChannels()
    {
        return m_numChls;
    }

    // Set the number of sampler "frames"
    void setSamplesPerBlock(const unsigned int samplesPerBlk)
    {
        m_samplesPerBlock = samplesPerBlk;
    }

    unsigned int getSamplesPerBock()
    {
        return m_samplesPerBlock;
    }

    unsigned int getSize()
    {
        return (m_numChls * m_samplesPerBlock * m_totalNumSamples);
    }

    bool initialized()
    {
        if (CAudioBufferBase<T>::m_numChls < 1 || CAudioBufferBase<T>::m_samplesPerBlock < 1)
            return false;

        if (CAudioBufferBase<T>::m_pBuff == nullptr)
            return false;

        if (!m_bAllocated)
            return false;

        if (m_totalNumSamples < 1)
            return false;

        return true;
    }

    // Get a pointer to the sample buffer
    T *getBuffPtr()
    {
        return m_pBuff;
    }

    // Clear the sample buffer (set it to zeros)
    void clear()
    {
        if (m_numChls < 1 || m_samplesPerBlock < 1 || m_totalNumSamples < 1)
        {
            return;
        }

        if (m_pBuff == nullptr)
        {
            return;
        }

        if (!m_bAllocated)
            return;

        unsigned int size = (m_numChls * m_samplesPerBlock * m_totalNumSamples);
        memset(m_pBuff, 0, size);
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

            m_eInputType = ::getAudioFileType(m_sInputFile);

            switch (m_eInputType)
            {
                case eFileType_raw:
                    {
                        m_pFileReader = (CAudioFileIO *) new CRawFileIO(m_numChls);
                        if (m_pFileReader == nullptr)
                            return false;
                    }
                    break;

                case eFileType_wav:
                    {
                        m_pFileReader = (CAudioFileIO *) new CWavFileIO(m_numChls);
                        if (m_pFileReader == nullptr)
                            return false;

                        ((CWavFileIO *)m_pFileReader)->setSampleRate(A2B_SAMPLE_RATE);
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

            m_eOutputType = ::getAudioFileType(m_sOutputFile);

            switch (m_eOutputType)
            {
                case eFileType_raw:
                    {
                        m_pFileWriter = (CAudioFileIO *)new CRawFileIO(m_numChls);
                        if (m_pFileWriter == nullptr)
                            return false;

                        ((CRawFileIO *)m_pFileWriter)->createInfoTextFile(true);
                    }
                    break;

                case eFileType_wav:
                    {
                        m_pFileWriter = (CAudioFileIO *)new CWavFileIO(m_numChls);
                        if (m_pFileWriter == nullptr)
                            return false;

                        ((CWavFileIO *)m_pFileReader)->setSampleRate(A2B_SAMPLE_RATE);
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

    eAudioFileType_def getInputFileType()
    {
        return m_eInputType;
    }

    eAudioFileType_def getOutputFileType()
    {
        return m_eOutputType;
    }
#endif
};

template <class T> class CInterleavedBuffer : public CAudioBufferBase<T>
{
  public:

    // Interleaved buffer class constructor
    CInterleavedBuffer()
    {
    }

    CInterleavedBuffer(const unsigned int numChls) :
        CAudioBufferBase<T>(numChls)
    {
    }

    CInterleavedBuffer(const unsigned int numChls, const unsigned int samplesPerBlk) :
        CAudioBufferBase<T>(numChls, samplesPerBlk)
    {
    }

    // Set the value of a sample within the buffer
    void setSample(const size_t chan, const size_t index, const T value)
    {
        if (!CAudioBufferBase<T>::initialized())
        {
            return;
        }

#ifdef SUPPORT_FILE_IO
        if (CAudioBufferBase<T>::m_pFileWriter != nullptr)
            CAudioBufferBase<T>::m_pFileWriter->writeSample(value, chan);
#endif
        *(CAudioBufferBase<T>::m_pBuff + (index * CAudioBufferBase<T>::m_numChls) + chan) = value;
    }

    // Get the value of a sample within the buffer
    T getSample(const size_t chan, const size_t index)
    {
        if (!CAudioBufferBase<T>::initialized())
        {
            return 0;
        }

#ifdef SUPPORT_FILE_IO
        if (CAudioBufferBase<T>::m_pFileReader != nullptr)
        {
            int16_t value = 0;
            CAudioBufferBase<T>::m_pFileReader->readSample(value, chan);
            return value;
        }
#endif
        return *(CAudioBufferBase<T>::m_pBuff + (index * CAudioBufferBase<T>::m_numChls) + chan);
    }
};

template <class T> class CNonInterleavedBuffer : public CAudioBufferBase<T>
{
  public:

    // Non-interleaved buffer class constructor
    CNonInterleavedBuffer()
    {
    }

    CNonInterleavedBuffer(const unsigned int numChls) :
        CAudioBufferBase<T>(numChls)
    {
    }

    CNonInterleavedBuffer(const unsigned int numChls, const unsigned int samplesPerBlk) :
        CAudioBufferBase<T>(numChls, samplesPerBlk)
    {
    }

    // Set the value of a sample within the buffer
    void setSample(const size_t chan, const size_t index, const T value)
    {
        if (!CAudioBufferBase<T>::initialized())
        {
            return;
        }

#ifdef SUPPORT_FILE_IO
        if (CAudioBufferBase<T>::m_eInputType == eAudioFileType_def::eFileType_raw)
        {
            return;
        }

        if (CAudioBufferBase<T>::m_pFileWriter != nullptr)
            CAudioBufferBase<T>::m_pFileWriter->writeSample(value, chan);
#endif
        /// @todo error checking for buffer overrun
        *(CAudioBufferBase<T>::m_pBuff + (chan * CAudioBufferBase<T>::m_numChls) + index) = value;
    }

    // Get the value of a sample within the buffer
    T getSample(const size_t chan, const size_t index)
    {
        if (!CAudioBufferBase<T>::initialized())
        {
             return 0;
        }

#ifdef SUPPORT_FILE_IO
        if (CAudioBufferBase<T>::m_eInputType == eAudioFileType_def::eFileType_raw)
        {
            return 0;
        }

        if (CAudioBufferBase<T>::m_pFileReader != nullptr)
        {
            int16_t value = 0;
            CAudioBufferBase<T>::m_pFileReader->readSample(value, chan);

            return value;
        }
#endif
        return *(CAudioBufferBase<T>::m_pBuff + (chan * CAudioBufferBase<T>::m_numChls) + index);
    }
};

#endif // _AUDIO_BUFFER_CLASS_H
