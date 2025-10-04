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
///             - "OpenCV" ...   https://opencv.org/
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


#ifndef E_VIDEO_DATA_FOTMAT_DEF
#define E_VIDEO_DATA_FOTMAT_DEF
typedef enum
{
    eVideoDataIoFormat_unknown = 0,
    eVideoDataIoFormat_yuv,
    eVideoDataIoFormat_rgb,
    eVideoDataIoFormat_mjpeg,
    eVideoDataIoFormat_mpeg1,
    eVideoDataIoFormat_mpeg2,
    eVideoDataIoFormat_mpeg4,
    eVideoDataIoFormat_h264,
    eVideoDataIoFormat_h265,
    eVideoDataIoFormat_divx,
    eVideoDataIoFormat_webm,
    eVideoDataIoFormat_vp7,
    eVideoDataIoFormat_vp8,
    eVideoDataIoFormat_vp9,
    eVideoDataIoFormat_vc1,

} eVideoDataIoFormat_def;
#endif


enum eVideoFileType_def
{
    eFileType_unknown = 0,
    eFileType_mpg4,
    eFileType_mjpeg,
    eFileType_raw,
    eFileType_avi,
    eFileType_mov,
    eFileType_wmv,
    eFileType_mpg1,
    eFileType_mpg2,
    eFileType_mkv,
    eFileType_webm,
    eFileType_info,

};


/// Convert an `eVideoFileType_def` value to a string
/// 
/// @param[in] value enum value
/// @return string representation of enum value

std::string videpFileTypeToString(eVideoFileType_def value);

eVideoFileType_def getVideoFileType(const std::string &filepath);

eVideoFileType_def fourCcToVideoFileType(const std::string &sFourCC);

std::string videoFormatToFourCC(eVideoDataIoFormat_def fmt);

eVideoDataIoFormat_def fourCcToVideoFormat(const std::string &sFourCC);


/// This is defined in several different files.
#ifndef UPDATE_FILE_POSITION
#define UPDATE_FILE_POSITION
#endif


// class CVideoFileIO

class CVideoFileIO
{
  protected:

    unsigned int            m_frameRate;        ///< frame rate
    unsigned int            m_width;            ///< number of pixels wide for video frame
    unsigned int            m_height;           ///< number of pixels high for video frame
    unsigned int            m_bitsPerPixel;
	
    unsigned int            m_nFrameSize;

    std::string             m_sFilePath;        ///< stream containug the file patrh/name

    bool                    m_bFileOpened;      ///< is file currently open

    eFileIoMode_def         m_eMode;            ///< file mode (input, output, ...)

    long                    m_nFramesInFile;    ///< total number of frames (currently) in the file
    int                     m_nCurrentFrameIdx; ///< current frame index within I/O block
    int                     m_nIoCntr;
    long                    m_nCurrentFrame;    ///< current frame read/write position within the file
    
    bool                    m_bUseLoopingRead;  ///< flag = if EOF reached, restart reading at beginning
    
    eVideoFileType_def      m_eFileType;

    eVideoDataIoFormat_def  m_eVideoFormat;
    
  public:

    static std::shared_ptr<CVideoFileIO> openFileTypeByExt
        (
            const std::string &sFilePath, 
            const eFileIoMode_def mode,
            const int width = 0,
            const int height = 0,
            const int frameRate = 0,
            const int bitsPerpixel = 24,
            const std::string &sFourCC = ""
        );

    CVideoFileIO();

    CVideoFileIO(unsigned int width, unsigned int height, unsigned int bitsPerpixel = 24);

    CVideoFileIO(const std::string &sFilePath);

    virtual ~CVideoFileIO();

    void                setFile(std::string &sFilePath);

    void                setLoopingRead(bool value);

    void                setFrameSize(unsigned int width, unsigned int height, unsigned int bitsPerPixel = 0);

    virtual void        setFrameRate(unsigned int rate);

    virtual void        setBitsPerPixel(int numBits);

    void                setVideoFormat(int fmt)
                        {
                            m_eVideoFormat = (eVideoDataIoFormat_def) fmt;
                        }

    bool                isFileOpened() const;

    std::string         getFilePath();

    eVideoFileType_def  getFileType();

    virtual int         getFrameRate()
                        {
                            return m_frameRate;
                        }

    virtual bool        openFile(const eFileIoMode_def mode, const std::string &sFilePath) = 0;
    virtual bool        closeFile() = 0;

    virtual bool        isEOF() = 0;

    virtual long        getNumFrames();

    unsigned int        getVideoFormat()
                        {
                            return (unsigned int) m_eVideoFormat;
                        }

    unsigned int        getVideoWidth()
                        {
                            return (unsigned int) m_width;
                        }

    unsigned int        getVideoHeigth()
                        {
                            return (unsigned int) m_height;
                        }

    unsigned int        getFrameSize()    // Get the frame size (in bytes)
                        {
                            return m_nFrameSize;
                        }

    /// Read a single frame, at the current frame offset.
    virtual bool        readFrame(void *pData) = 0;

    /// Read a block of frames, for the specified number of frames, at the current frame offset.
    virtual bool        readBlock(void *pData, unsigned int numFrames) = 0;

    /// Write a single frame, at the current frame offset.
    virtual bool        writeFrame(const void *pData) = 0;

    /// Write a block of frames, for the specified number of frames, at the current frame offset.
    virtual bool        writeBlock(const void *pData, unsigned int numFrames) = 0;

    unsigned int        getIoCount() const;

    virtual bool        setCurrentFrame(unsigned int frameNum) = 0;

    int                 getCurrentFrame() const;

    int                 getCurrentFrameIndex() const;

    virtual bool        resetPlayPosition();
};


// class CRawVideoFileIO

class CRawVideoFileIO : 
    public CVideoFileIO
{
  public:

    struct SRawFileInfo
    {
        int         width;
        int         height;
        int         bitsPerPixel;
        int         frameRate;
        std::string fourCC;

        void        clear()
        {
            width     		= 0;
            height   		= 0;
            bitsPerPixel   	= 0;
            frameRate      	= 0;
            fourCC          = "";
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

    SRawFileInfo    m_fileInfo;

  protected:

    bool parseInfoTextFile(const std::string& sFile, SRawFileInfo& info);

    bool writeInfoTextFile(const std::string& sFile, SRawFileInfo& info);

public:

    CRawVideoFileIO();

    CRawVideoFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    CRawVideoFileIO(const std::string &sFilePath);

    ~CRawVideoFileIO() override;

    virtual void setFrameRate(const unsigned int rate)
    {
        m_frameRate = rate;

        m_fileInfo.frameRate = rate;
    }

    void createInfoTextFile(bool value)
    {
        m_bCreateInfoTextFile = value;
    }

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
        int         width;
        int         height;
        int         bitsPerPixel;
        int         frameRate;

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

    SAviFileInfo    m_fileInfo;

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

  #if 0
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
#endif

  protected:

    inline std::string cvFormatToStr(const int fmt)
    {
        // Convert the integer FOURCC to a string
        std::string sOut = "";

        sOut += static_cast<char>(fmt & 0xFF);         // Extract the first character
        sOut += static_cast<char>((fmt >> 8) & 0xFF);  // Extract the second character
        sOut += static_cast<char>((fmt >> 16) & 0xFF); // Extract the third character
        sOut += static_cast<char>((fmt >> 24) & 0xFF); // Extract the fourth character

        return sOut;
    }

    inline eVideoDataIoFormat_def cvFourCcToVideoFmt(const int fourCC)
    {
        auto sFourCC = cvFormatToStr(fourCC);

        if (sFourCC == "YUV")
            return eVideoDataIoFormat_yuv;

        if (sFourCC == "RGB")
            return eVideoDataIoFormat_rgb;

        if (sFourCC == "MJPG")
            return eVideoDataIoFormat_mjpeg;

        if (sFourCC == "H264")
            return eVideoDataIoFormat_h264;

        if (sFourCC == "H265")
            return eVideoDataIoFormat_h265;

        if (sFourCC == "MPG4")
            return eVideoDataIoFormat_mpeg4;

        if (sFourCC == "MPG2")
            return eVideoDataIoFormat_mpeg2;

        if (sFourCC == "MPEG" || sFourCC == "MPG1")
            return eVideoDataIoFormat_mpeg1;

        if (sFourCC == "DIVX")
            return eVideoDataIoFormat_divx;

        if (sFourCC == "WEBM")
            return eVideoDataIoFormat_webm;

        if (sFourCC == "VP7")
            return eVideoDataIoFormat_vp7;

        if (sFourCC == "VP8")
            return eVideoDataIoFormat_vp8;

        if (sFourCC == "VP9")
            return eVideoDataIoFormat_vp9;

        if (sFourCC == "VC1")
            return eVideoDataIoFormat_vc1;

        return eVideoDataIoFormat_unknown;
    }

  private:

    cv::VideoCapture    *m_pFileInput;

    cv::VideoWriter     *m_pFileOutput;

    unsigned long       m_lFileSize;

    std::string         m_sFourCC;

    cv::Mat             *m_pCvFrame;
    
    int                 m_lastFrameRead;
    int                 m_lastFrameWritten;

    inline bool writeToCvFrame(cv::Mat& frame, const void* pSrc, const unsigned int copySize)
    {
        size_t frameDataSize = (frame.total() * frame.elemSize());

        if (copySize > frameDataSize)
        {
            memcpy((void *) frame.data, pSrc, frameDataSize);

            return false;
        }

        memcpy((void*) frame.data, pSrc, copySize);

        return true;
    }

    inline bool readFromCvFrame(void* pTrgt, cv::Mat& frame, const unsigned int maxCopySize)
    {
        size_t frameDataSize = (frame.total() * frame.elemSize());

        if (frameDataSize > maxCopySize)
        {
            memcpy(pTrgt, (void*) frame.data, maxCopySize);

            return false;
        }

        memcpy(pTrgt, (void*) frame.data, frameDataSize);

        return true;
    }

public:

    COcvFileIO();

    COcvFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    COcvFileIO(const std::string& sFilePath);

    ~COcvFileIO() override;

    virtual bool openFile(eFileIoMode_def mode, const std::string& sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    cv::Mat* getCvFramePtr()
    {
        return m_pCvFrame;
    }

    /// Read a frame from an "AVI" data file
    bool readFrame(void* pData) override;

    bool readBlock(void* pData, unsigned int numFrames) override;

    bool writeFrame(const void* pData) override;

    bool writeBlock(const void* pData, unsigned int numFrames) override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};


#endif  //  VIDEO_FILE_IO_H
