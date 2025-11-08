/// 
/// \file       CVideoFileIO.h
///
///             CVideoFileIO class header file
///
///             NOTE: The CVideoFileIO classes have the following dependencies: 
/// 
///             - "CFileIO.h" ...   RDB-libs/FileIO/CFileIO.h  
///                 AND
///             - "gwAVI" ...       https://github.com/rolinh/libgwavi      (if enabled)
///                 AND
///             - "Matroska" ...    https://www.matroska.org/               (if enabled)
///                 AND
///             - "OpenCV" ...      https://opencv.org/                     (if enabled)
/// 


#ifndef VIDEO_FILE_IO_H
#define VIDEO_FILE_IO_H

//#define SUPPORT_AVI_IO_LOGIC          // Support (internal) AVI file I/O
//#define SUPPORT_MKV_IO_LOGIC          // Support (internal) MKV file I/O
//#define SUPPORT_OCV_IO_LOGIC          // Support OpenCV file I/O


#include "../Error/CError.h"

#if __cplusplus < 201703L
COMPILE_ERROR("ERRORL: C++17 not supported")
#endif


#include "CFileIO.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef SUPPORT_AVI_IO_LOGIC
extern "C"
{
#include "../../../libGwAVI/inc/gwavi.h"
#include "../../../libGwAVI/src/gwavi_private.h"
};
#endif

#ifdef SUPPORT_MKV_IO_LOGIC
#include "../../../libEBML/ebml/EbmlStream.h"
#include "../../../libMatroska/matroska/c/libmatroska.h"
#endif

#include "opencv2/opencv.hpp"
#include "opencv2/videoio.hpp"


#ifndef E_VIDEO_DATA_FOTMAT_DEF
#define E_VIDEO_DATA_FOTMAT_DEF
typedef enum
{
    eVideoDataIoFormat_unknown = 0,
    // raw pixel formats
    eVideoDataIoFormat_yuv,         // YUV 4:4:4
    eVideoDataIoFormat_yuy2,        // YUV 4:2:2
    eVideoDataIoFormat_yv12,        // YUV 4:2:0
    eVideoDataIoFormat_nv12,        // YUV 4:2:0
    eVideoDataIoFormat_rgb,
    eVideoDataIoFormat_bgr,
    // encoded video formats
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
    eVideoDataIoFormat_i420,       // Intel I420 

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

    /// Read a single video frame, at the current frame offset.
    virtual bool        readVideoFrame(void *pData) = 0;

    /// Read a block of video frames, for the specified number of frames, at the current frame offset.
    virtual bool        readVideoBlock(void *pData, unsigned int numFrames) = 0;

    /// Write a single video frame, at the current frame offset.
    virtual bool        writeVideoFrame(const void *pData) = 0;

    /// Write a single video frame, at the current frame offset.
    virtual bool        writeVideoFrame(const void* pData, unsigned int frameLen) = 0;

    /// Write a block of video frames, for the specified number of frames, at the current frame offset.
    virtual bool        writeVideoBlock(const void *pData, unsigned int numFrames) = 0;

    /// Read a single audio frame, at the current frame offset.
    virtual bool        readAudioFrame(void* pData) = 0;

    /// Read a block of audio frames, for the specified number of frames, at the current frame offset.
    virtual bool        readAudioBlock(void* pData, unsigned int numFrames) = 0;

    /// Write a single audio frame, at the current frame offset.
    virtual bool        writeAudioFrame(const void* pData) = 0;

    /// Write a block of audio frames, for the specified number of frames, at the current frame offset.
    virtual bool        writeAudioBlock(const void* pData, unsigned int numFrames) = 0;

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

      struct SVideoFormatInfo
      {
          int         width;
          int         height;
          int         bitsPerPixel;
          int         frameRate;

          std::string sFourCC;

          void        clear()
          {
              width = 0;
              height = 0;
              bitsPerPixel = 0;
              frameRate = 0;

              sFourCC = "";
          }

          SVideoFormatInfo()
          {
              clear();
          }
      };

  private:

    CFileIO             m_fileIO;

    void                *m_pFramebuffer;
    
    unsigned long       m_lFileSize;
    unsigned long       m_lCurrentFilePos;
    
    int                 m_lastFrameRead;
    int                 m_lastFrameWritten;
    
    bool                m_bCreateInfoTextFile;

    SVideoFormatInfo    m_fileInfo;

  protected:

    bool parseInfoTextFile(const std::string& sFile, SVideoFormatInfo& info);

    bool writeInfoTextFile(const std::string& sFile, SVideoFormatInfo& info);

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

    /// Read a video frame from a "raw" data file
    virtual bool readVideoFrame(void* pData) override;

    virtual bool readVideoBlock(void *pData, unsigned int numFrames) override;

    virtual bool writeVideoFrame(const void* pData) override;

    virtual bool writeVideoFrame(const void* pData, unsigned int frameLen) override;

    virtual bool writeVideoBlock(const void *pData, unsigned int numFrames) override;

    virtual bool readAudioFrame(void* pData) override
    {
        return false;
    }

    virtual bool readAudioBlock(void* pData, unsigned int numFrames) override
    {
        return false;
    }

    virtual bool writeAudioFrame(const void* pData) override
    {
        return false;
    }

    virtual bool writeAudioBlock(const void* pData, unsigned int numFrames) override
    {
        return false;
    }

    virtual bool setCurrentFrame(unsigned int frameNum) override;

    virtual bool resetPlayPosition() override;
};


// class CAviFileIO

#ifdef SUPPORT_AVI_IO_LOGIC

class CAviFileIO :
    public CVideoFileIO
{
public:

    struct SVideoFormatInfo
    {
        int         width;
        int         height;
        int         bitsPerPixel;
        int         frameRate;

        std::string sVideo4CC;

        void        clear()
        {
            width = 0;
            height = 0;
            bitsPerPixel = 0;
            frameRate = 0;

            sVideo4CC = "";
        }

        SVideoFormatInfo()
        {
            clear();
        }
    };

    struct SAudioFormatInfo
    {
        int         numTracks;
        int         bitsPerSample;
        int         sampleRate;

        std::string sAudio4CC;

        void        clear()
        {
            numTracks = 0;
            bitsPerSample = 0;
            sampleRate = 0;

            sAudio4CC = "";
        }

        SAudioFormatInfo()
        {
            clear();
        }
    };

private:

    gwavi_t             *m_pFileCtrl;

    //std::string         m_sFourCC;

    gwavi_audio_t       m_audioInfo;

    unsigned long       m_lFileSize;
    
    int                 m_lastFrameRead;
    int                 m_lastFrameWritten;

    SVideoFormatInfo    m_fileInfo;

public:

    CAviFileIO();

    CAviFileIO
        (
            unsigned int width, 
            unsigned int height, 
            unsigned int bitsPerPixel, 
            const unsigned int nFrameRate = 0, 
            const std::string &sFourCC = ""
        );

    CAviFileIO(const std::string& sFilePath);

    ~CAviFileIO() override;

    void setFileFourCC(const std::string& sFourCC)
    {
        //m_sFourCC = sFourCC;
        m_fileInfo.sVideo4CC;
    }

    void setVideoConfig
        (
            unsigned int width,
            unsigned int height,
            unsigned int bitsPerPixel,
            const unsigned int nFrameRate = 0,
            const std::string& sFourCC = ""
        );

    void setAudioConfig
        (
            int         numTracks,
            int         bitsPerSample,
            int         sampleRate,
            std::string sFourCC
        );

    virtual bool openFile(eFileIoMode_def mode, const std::string& sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    /// Read a video frame from an "AVI" data file
    virtual bool readVideoFrame(void* pData) override;

    virtual bool readVideoBlock(void* pData, unsigned int numFrames) override;

    virtual bool writeVideoFrame(const void* pData) override;

    virtual bool writeVideoFrame(const void* pData, unsigned int frameSize) override;

    virtual bool writeVideoBlock(const void* pData, unsigned int numFrames) override;

    /// Read a video frame from an "AVI" data file
    virtual bool readAudioFrame(void* pData) override;

    virtual bool readAudioBlock(void* pData, unsigned int numFrames) override;

    virtual bool writeAudioFrame(const void* pData) override;

    virtual bool writeAudioBlock(const void* pData, unsigned int numFrames) override;

    virtual bool setCurrentFrame(unsigned int frameNum) override;

    virtual bool resetPlayPosition() override;
};

#endif


#ifdef SUPPORT_MKV_IO_LOGIC

class CMkvFileIO :
    public CVideoFileIO
{
public:

    struct SVideoFormatInfo
    {
        int         width;
        int         height;
        int         bitsPerPixel;
        int         frameRate;

        std::string m_sVideo4CC;

        void        clear()
        {
            width = 0;
            height = 0;
            bitsPerPixel = 0;
            frameRate = 0;

            m_sVideo4CC = "";
        }

        SVideoFormatInfo()
        {
            clear();
        }
    };

    struct SAudioFormatInfo
    {
        int         numTracks;
        int         bitsPerSample;
        int         sampleRate;

        std::string m_sAudio4CC;

        void        clear()
        {
            numTracks = 0;
            bitsPerSample = 0;
            sampleRate = 0;

            m_sAudio4CC = "";
        }

        SVideoFormatInfo()
        {
            clear();
        }
    };

private:

    unsigned long       m_lFileSize;

    int                 m_lastFrameRead;
    int                 m_lastFrameWritten;

    SVideoFormatInfo    m_videoFormatInfo;

public:

    CMkvFileIO();

    CMkvFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel);

    CMkvFileIO(const std::string& sFilePath);

    ~CMkvFileIO() override;

    virtual bool openFile(eFileIoMode_def mode, const std::string& sFilePath) override;

    virtual bool closeFile() override;

    virtual long getNumFrames();

    virtual bool isEOF() override;

    /// Read a video frame from an "MKV" data file
    virtual bool readVideoFrame(void* pData) override;

    virtual bool readVideoBlock(void* pData, unsigned int numFrames) override;

    virtual bool writeVideoFrame(const void* pData) override;

    virtual bool writeVideoFrame(const void* pData, unsigned int frameSize) override;

    virtual bool writeVideoBlock(const void* pData, unsigned int numFrames) override;

    /// Read a video frame from an "MKV" data file
    virtual bool readAudioFrame(void* pData) override;

    virtual bool readAudioBlock(void* pData, unsigned int numFrames) override;

    virtual bool writeAudioFrame(const void* pData) override;

    virtual bool writeAudioBlock(const void* pData, unsigned int numFrames) override;

    virtual bool setCurrentFrame(unsigned int frameNum) override;

    virtual bool resetPlayPosition() override;
};

#endif


#ifdef SUPPORT_OCV_IO_LOGIC

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

        if (sFourCC == "YUY2")
            return eVideoDataIoFormat_yuy2;

        if (sFourCC == "Yv12")
            return eVideoDataIoFormat_yv12;

        if (sFourCC == "Nv12")
            return eVideoDataIoFormat_nv12;

        if (sFourCC == "RGB")
            return eVideoDataIoFormat_rgb;

        if (sFourCC == "BGR")
            return eVideoDataIoFormat_bgr;

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

        if (sFourCC == "I420")
            return eVideoDataIoFormat_i420;

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

    /// Read a video frame from an "OpenCV" data file
    bool readVideoFrame(void* pData) override;

    bool readVideoBlock(void* pData, unsigned int numFrames) override;

    bool writeVideoFrame(const void* pData) override;

    bool writeVideoBlock(const void* pData, unsigned int numFrames) override;

    /// Read a video frame from an "OpenCV" data file
    bool readAudioFrame(void* pData) override;

    bool readAudioBlock(void* pData, unsigned int numFrames) override;

    bool writeAudioFrame(const void* pData) override;

    bool writeAudioBlock(const void* pData, unsigned int numFrames) override;

    bool setCurrentFrame(unsigned int frameNum) override;

    bool resetPlayPosition() override;
};

#endif

#endif  //  VIDEO_FILE_IO_H
