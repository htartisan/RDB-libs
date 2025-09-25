/// 
/// \file       CVideoFileIO.h
///
///             CVideoFileIO class header file
///
///             NOTE: The CVideoFileIO classes have the following dependencies: 
/// 
///             - "CFileIO.h" ...   RDB-libs/FileIO/CFileIO.h  
/// 
///                 AND
/// 
///             - "VideoWave" ...   https://github.com/adamstark/VideoFile    
/// 


#ifndef VIDEO_FILE_IO_H
#define VIDEO_FILE_IO_H

#include "../Error/CError.h"

#if __cplusplus < 201703L
COMPILE_ERROR("ERRORL: C++17 not supported")
#endif


#include "CFileIO.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

#if 0
#include "../../../libgwavi/inc/gwavi.h"
#include "../../../libgwavi/src/gwavi_private.h"
#endif

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"


enum eVideoFileType_def
{
    eFileType_unknown = 0,
    eFileType_raw,
    eFileType_mp4,
    eFileType_mov,
    eFileType_avi,
    eFileType_mpg,
    eFileType_wmv,
    eFileType_mkv,
    eFileType_webm,
    eFileType_info,
    eFileType_text,
};


/// Convert an `eVideoFileType_def` value to a string
/// 
/// @param[in] value enum value
/// @return string representation of enum value

std::string videpFileTypeToString(eVideoFileType_def value);

eVideoFileType_def getVideoFileType(const std::string &filepath);


/// This is defined in several different files.
#ifndef UPDATE_FILE_POSITION
#define UPDATE_FILE_POSITION
#endif


// class CVideoFileIO

class CVideoFileIO
{
  protected:

    unsigned int        m_frameRate;        ///< frame rate
    unsigned int        m_width;            ///< number of pixels wide for video frame
    unsigned int        m_height;           ///< number of pixels high for video frame
    unsigned int        m_bitsPerPixel;
	
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
    
    eVideoFileType_def  m_eFileType;
    
  public:

    static std::shared_ptr<CVideoFileIO> openFileTypeByExt
        (
            const std::string &sFilePath, 
            eFileIoMode_def mode, 
            int width = 0,
            int height = 0,
            int frameRate = 0, 
            int blockSize = 0,
            int bitsPerpixel = 24
        );

    CVideoFileIO();

    CVideoFileIO(unsigned int width, unsigned int height, unsigned int bitsPerpixel = 24);

    CVideoFileIO(const std::string &sFilePath);

    virtual ~CVideoFileIO();

    void               setFile(std::string &sFilePath);

    void               setLoopingRead(bool value);

    void               setFrameSize(unsigned int width, unsigned int height, unsigned int bitsPerPixel = 0);

    virtual void       setFrameRate(unsigned int rate);

    virtual void       setIoBlockSize(int numFrames);

    virtual void       setBitsPerPixel(int numBits);

    bool               isFileOpened() const;

    std::string        getFilePath();

    eVideoFileType_def getFileType();

    virtual int        getFrameRate()
    {
        return m_frameRate;
    }

    virtual bool       openFile(const eFileIoMode_def mode, const std::string &sFilePath) = 0;
    virtual bool       closeFile() = 0;

    virtual bool       isEOF() = 0;

    virtual long       getNumFrames();

    /// Read a single frame, at the current frame offset.
    virtual bool readFrame(void *pData) = 0;

    /// Read a block of frames, for the specified number of frames, at the current frame offset.
    virtual bool readBlock(void *pData, unsigned int numFrames) = 0;

    /// Write a single frame, at the current frame offset.
    virtual bool writeFrame(const void *pData) = 0;

    /// Write a block of frames, for the specified number of frames, at the current frame offset.
    virtual bool writeBlock(const void *pData, unsigned int numFrames) = 0;

    unsigned int getIoCount() const;

    virtual bool setCurrentFrame(unsigned int frameNum) = 0;

    int          getCurrentFrame() const;

    int          getCurrentFrameIndex() const;

    virtual bool resetPlayPosition();
};


// class CRawVideoFileIO

class CRawVideoFileIO : 
    public CVideoFileIO
{
  public:

    struct SRawFileInfo
    {
        int         width     		= 0;
        int         height     		= 0;
        int         bitsPerPixel   	= 0;
        int         frameRate      	= 0;

        void        clear()
        {
            width     		= 0;
            height   		= 0;
            bitsPerPixel   	= 0;
            frameRate      	= 0;
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
    
    int             m_lastFrameRead;
    int             m_lastFrameWritten;
    
    bool            m_bCreateInfoTextFile;

  public:

    CRawVideoFileIO();

    CRawVideoFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    CRawVideoFileIO(const std::string &sFilePath);

    ~CRawVideoFileIO() override;

    void createInfoTextFile(bool value);

    bool parseInfoTextFile(const std::string &sFile, SRawFileInfo &info);

    virtual bool openFile(eFileIoMode_def mode, const std::string &sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    /// Read a frame from a "raw" data file
    bool readFrame(void* pData) override;

    bool readBlock(void *pData, unsigned int numFrames) override;

    bool writeFrame(const void* pData) override;

    bool writeBlock(const void *pData, unsigned int numFrames) override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};


// class CAviFileIO

#if 0

class CAviFileIO :
    public CVideoFileIO
{
public:

    struct SAviFileInfo
    {
        int         width = 0;
        int         height = 0;
        int         bitsPerPixel = 0;
        int         frameRate = 0;

        void        clear()
        {
            width = 0;
            height = 0;
            bitsPerPixel = 0;
            frameRate = 0;
        }

        SAviFileInfo()
        {
            clear();
        }
    };

private:

    gwavi_t         *m_pFileCtrl;

    gwavi_audio_t   m_audioInfo;

    std::string     m_sFourCC;

    unsigned long   m_lFileSize;
    
    int             m_lastFrameRead;
    int             m_lastFrameWritten;

public:

    CAviFileIO();

    CAviFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    CAviFileIO(const std::string& sFilePath);

    ~CAviFileIO() override;

    virtual bool openFile(eFileIoMode_def mode, const std::string& sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    /// Read a frame from an "AVI" data file
    bool readFrame(void* pData) override;

    bool readBlock(void* pData, unsigned int numFrames) override;

    bool writeFrame(const void* pData) override;

    bool writeBlock(const void* pData, unsigned int numFrames) override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};

#endif

// class COcvFileIO

class COcvFileIO :
    public CVideoFileIO
{
public:

    struct SOcvFileInfo
    {
        int         width = 0;
        int         height = 0;
        int         bitsPerPixel = 0;
        int         frameRate = 0;

        void        clear()
        {
            width = 0;
            height = 0;
            bitsPerPixel = 0;
            frameRate = 0;
        }

        SOcvFileInfo()
        {
            clear();
        }
    };

private:

    cv::VideoCapture    *m_pFileInput;

    cv::VideoWriter     *m_pFileOutput;

    unsigned long       m_lFileSize;

    std::string         m_sFourCC;
    
    int                 m_lastFrameRead;
    int                 m_lastFrameWritten;

public:

    COcvFileIO();

    COcvFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    COcvFileIO(const std::string& sFilePath);

    ~COcvFileIO() override;

    virtual bool openFile(eFileIoMode_def mode, const std::string& sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    /// Read a frame from an "AVI" data file
    bool readFrame(void* pData) override;

    bool readBlock(void* pData, unsigned int numFrames) override;

    bool writeFrame(const void* pData) override;

    bool writeBlock(const void* pData, unsigned int numFrames) override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};


#endif  //  VIDEO_FILE_IO_H
