/// 
/// \file       CAudioFileIO.h
///
///             CAudioFileIO class header file
///
///             NOTE: The CAudioFileIO classes have the following dependencies: 
/// 
///             - "CFileIO.h" ...   RDB-libs/FileIO/CFileIO.h  
/// 
///                 AND
/// 
///             - "AudioWave" ...   https://github.com/adamstark/AudioFile    
/// 
///                 OR
/// 
///             - "dr_wav" ...      https://github.com/mackron/dr_libs/tree/master
/// 


#define _CRT_SECURE_NO_WARNINGS


#ifndef AUDIO_FILE_IO_H
#define AUDIO_FILE_IO_H

#include "../Error/CError.h"

#if __cplusplus < 201703L
COMPILE_ERROR("ERRORL: C++17 not supported")
#endif


#include "CFileIO.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>


enum eAudioFileType_def
{
    eFileType_unknown = 0,
    eFileType_raw,
    eFileType_wav,
    eFileType_aiff,
    eFileType_mp3,
    eFileType_aac,
    eFileType_ac3,
    eFileType_ec3,
    eFileType_info,
    eFileType_text,
};


/// Convert an `eAudioFileType_def` value to a string
/// 
/// @param[in] value enum value
/// @return string representation of enum value

std::string audioFileTypeToString(eAudioFileType_def value);


eAudioFileType_def getAudioFileType(const std::string &filepath);


/// The following determines whether to use 
/// the "AudioFile.h" or "dr_wav.h".
// #define USE_DR_WAV

// #define USE_DR_MP3

#ifdef USE_DR_WAV

/// #define DR_WAV_NO_CONVERSION_API 
#include "../Libs/drwav-lib/dr_wav.h"

#else

#include "../Libs/AudioFile-lib/AudioFile.h"

#endif /// USE_DR_WAV


#ifdef USE_DR_MP3

#include "../Libs/drwav-lib/dr_mp3.h"

#endif

/// This is defined in several different files.
#ifndef UPDATE_FILE_POSITION
#define UPDATE_FILE_POSITION
#endif

#define ConvertInt16ToFloat(sample16) (((float)sample16) / 0x7FFF)

#define ConvertFloatToInt16(fSample)  ((int16_t)(fSample * 0x7FFF))


class CAudioFileIO
{
  protected:

    unsigned int        m_sampleRate;       ///< sample rate
    unsigned int        m_numChls;          ///< number is samples/channels per frame
    unsigned int        m_nFrameSize;

    std::string         m_sFilePath;        ///< stream containug the file patrh/name

    bool                m_bFileOpened;      ///< is file currently open

    eFileIoMode_def     m_eMode;            ///< file mode (input, output, ...)

    int                 m_nIoBlockSize;     ///< number of frames in an I/O block read/write
    long                m_nFramesInFile;    ///< total number of frames (currently) in the file
    int                 m_nCurrentFrameIdx; ///< current frame index within I/O block
    int                 m_nIoCntr;
    long                m_nCurrentFrame;    ///< current frame read/write position within the file
    
    bool                m_bUseLoopingRead;  ///< flag = if EOF reached, restart reading at beginning
    
    eAudioFileType_def  m_eFileType;
    
    int                 m_nBitsPerSample;

  public:

    static std::shared_ptr<CAudioFileIO> openFileTypeByExt
        (
            const std::string &sFilePath, 
            eFileIoMode_def mode, 
            int numChannels = 0,
            int frameRate = 0, 
            int blockSize = 0,
            int bitsPerSasmple = 16
        );

    CAudioFileIO();

    CAudioFileIO(unsigned int numChannels);

    CAudioFileIO(unsigned int numChannels, const std::string &sFilePath);

    virtual ~CAudioFileIO();

    void               setFile(std::string &sFilePath);

    void               setLoopingRead(bool value);

    void               setNumChannels(unsigned int numChls);

    virtual void       setSampleRate(unsigned int rate);

    virtual void       setIoBlockSize(int numFrames);

    virtual void       setSampleSize(int numBits);

    bool               isFileOpened() const;

    std::string        getFilePath();

    eAudioFileType_def getFileType();

    virtual bool       openFile(eFileIoMode_def mode, const std::string &sFilePath) = 0;
    virtual bool       closeFile()                                                  = 0;

    virtual int        getNumChannels();

    virtual long       getNumFramesInFile();

    virtual int        getSampleRate()
    {
        return m_sampleRate;
    }

    virtual bool       isEOF() = 0;

    /// Read a single sample, for the specified channel, at the current frame offset.
    virtual bool readSample(int16_t &data, unsigned int chl)              = 0;
    virtual bool readSample(int32_t &data, unsigned int chl)              = 0;

    /// Read a block of samples (all channels), for the specified number of frames, at the current frame offset.
    virtual bool readBlock(void *pData, unsigned int numFrames)        = 0;

    /// Write a single sample, for the specified channel, at the current frame offset.
    virtual bool writeSample(int16_t data, unsigned int chl)              = 0;
    virtual bool writeSample(int32_t data, unsigned int chl)              = 0;

    /// Write a block of samples (all channels), for the specified number of frames, at the current frame offset.
    virtual bool writeBlock(const void *pData, unsigned int numFrames) = 0;

    unsigned int getIoCount() const;

    /// Move to next input/output frame (read or write frame if needed)
    virtual bool nextFrame() = 0;

    virtual bool setCurrentFrame(unsigned int frameNum) = 0;

    int          getCurrentFrame() const;

    int          getCurrentFrameIndex() const;

    virtual bool resetPlayPosition();
};


class CRawAudioFileIO : 
    public CAudioFileIO
{
  public:

    struct SRawFileInfo
    {
        int         numChannels     = 0;
        int         sampleRate      = 0;
        int         bitsPerSample   = 0;

        void        clear()
        {
            numChannels     = 0;
            sampleRate      = 0;
            bitsPerSample   = 0;
        }

        SRawFileInfo()
        {
            clear();
        }
    };

  private:

    CFileIO         m_fileIO;
    void            *m_pFramebuffer;
    unsigned long   m_lFileSize;
    unsigned long   m_lCurrentFilePos;
    int             m_lastChlRead;
    int             m_lastChlWritten;
    bool            m_bCreateInfoTextFile;

  protected:

    int getNumericStringAt(const std::string &sText, const unsigned int pos);

  public:

    CRawAudioFileIO(unsigned int numChannels);

    CRawAudioFileIO(unsigned int numChannels, const std::string &sFilePath);

    ~CRawAudioFileIO() override;

    void createInfoTextFile(bool value);

    bool parseInfoTextFile(const std::string &sFile, SRawFileInfo &info);

    bool openFile(eFileIoMode_def mode, const std::string &sFilePath) override;

    long getNumFrames();

    bool closeFile() override;

    bool isEOF();

    /// Read a sample from a "raw" data file
    bool readSample(int16_t &data, unsigned int chl) override;
    bool readSample(int32_t &data, unsigned int chl) override;

    bool readBlock(void *pData, unsigned int numFrames) override;

    bool writeSample(int16_t data, unsigned int chl) override;
    bool writeSample(int32_t data, unsigned int chl) override;

    bool writeBlock(const void *pData, unsigned int numFrames) override;

    /// Move to next input/output frame
    bool nextFrame() override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};


class CWavFileIO : 
    public CAudioFileIO
{

#ifndef USE_DR_WAV
    AudioFile<float> m_audioFile;

    unsigned int m_blockSize;
#else
    drwav       m_audioFile{};
#endif

    void        *m_pFramebuffer;

    unsigned int m_currChannel;

    int          m_lastChlRead;
    int          m_lastChlWritten;

    bool         m_bWriteFileForEachFrame;

    bool getSamples(void *pData, unsigned int numFrames);

  public:

    CWavFileIO(unsigned int numChannels);

    CWavFileIO(unsigned int numChannels, const std::string &sFilePath);

    ~CWavFileIO() override;

    bool openFile(eFileIoMode_def mode, const std::string &sFilePath) override;

   ///  @note For "wav" and "aiff" files.
   ///  The data is ONLY written to the disk file when the 
   ///  file is closed unless m_bWriteFileForEachFrame = true.

    bool closeFile() override;

    void setFrameOutputWriteFlag(bool value);

    void setSampleRate(unsigned int rate) override;

#ifndef USE_DR_WAV
    void setBlockSize(const unsigned int blockSize);
#endif

    int  getSampleRate() override;

    int  getNumChannels() override;

    int  getNumFrames() const;

    bool isEOF();

    bool readSample(int16_t &data, unsigned int chl) override;
    bool readSample(int32_t &data, unsigned int chl) override;

    bool readBlock(void *pData, unsigned int numFrames) override;

    bool writeSample(int16_t data, unsigned int chl) override;
    bool writeSample(int32_t data, unsigned int chl) override;

    bool writeBlock(const void *pData, unsigned int numFrames) override;

    /// Move to next input/output frame
    bool nextFrame() override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};


#ifdef USE_DR_MP3

class CMp3FileIO : 
    public CAudioFileIO
{

    drmp3       m_audioFile{};

    void        *m_pFramebuffer;

    unsigned int m_currChannel;

    int          m_lastChlRead;
    int          m_lastChlWritten;

    bool         m_bWriteFileForEachFrame;

  public:

    CMp3FileIO(unsigned int numChannels);

    CMp3FileIO(unsigned int numChannels, const std::string &sFilePath);

    ~CMp3FileIO() override;

    bool openFile(eFileIoMode_def mode, const std::string &sFilePath) override;

   ///  @note For "mp3" files.
   ///  The data is ONLY written to the disk file when the 
   ///  file is closed unless m_bWriteFileForEachFrame = true.

    bool closeFile() override;

    void setFrameOutputWriteFlag(bool value);

    void setSampleRate(unsigned int rate) override;

    int  getSampleRate();

    int  getNumChannels() override;

    int  getNumFrames() const;

    bool isEOF();

    bool readSample(int16_t &data, unsigned int chl) override;
    bool readSample(int32_t &data, unsigned int chl) override;

    bool readBlock(void *pData, unsigned int numFrames) override;

    bool writeSample(int16_t data, unsigned int chl) override;
    bool writeSample(int32_t data, unsigned int chl) override;

    bool writeBlock(const void *pData, unsigned int numFrames) override;

    /// Move to next input/output frame
    bool nextFrame() override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};

#endif  //  USE_DR_MP3

#endif  //  AUDIO_FILE_IO_H
