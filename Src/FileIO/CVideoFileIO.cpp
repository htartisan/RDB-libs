/// 
/// \file       CVideoFileIO.cpp
/// 
///             CVideoFileIO function definitions
/// 
///             NOTE: The CVideoFileIO classes have the following dependencies: 
/// 
///             - "CFileIO.h" ...   RDB-libs/FileIO/CFileIO.h  
/// 
///                 AND
/// 
///             - "OpenCV" ...   https://opencv.org/
///


#include "../Logging/Logging.h"

#include "CVideoFileIO.h"

#include <string>
#include <map>

#include <filesystem>
#include <optional>

#include "../String/StrUtils.h"

#include "FileUtils.h"


// Utility functions defs


std::string FourCC2String(uint32_t fourcc);

uint32_t String2FourCC(const std::string& fourcc);


// Convert eVideoFileType_def to file ext string
std::string videpFileTypeToString(eVideoFileType_def value)
{
    static std::map<eVideoFileType_def, std::string> map = 
    {
        {eFileType_raw, "raw"},   
        {eFileType_mpg4, "mp4"},   
        {eFileType_mjpeg, "mjpg"},
        {eFileType_avi, "avi"},
        {eFileType_mov, "mov"},
        {eFileType_wmv, "wmv"},
        {eFileType_mpg1, "mpg"},
        {eFileType_mpg2, "mp2"},
        {eFileType_mkv, "mkv"},
        {eFileType_webm, "webm"},
        {eFileType_info, "info"}, 
    };

    std::string name = "unknown value " + std::to_string(static_cast<int>(value));
    
    auto        it   = map.find(value);
    if (it != map.end())
        name = it->second;
    
    return name;
}


eVideoFileType_def getVideoFileType(const std::string& filepath)
{
    if (filepath.empty())
        return eFileType_unknown;

    std::string ext = std::filesystem::path(filepath).extension().string();

    ext = StrUtils::toLower(ext);

    if (ext.empty() || ext == ".raw")
        return eFileType_raw;

    if (ext == ".mp4")
        return eFileType_mpg4;

    if (ext == ".mjpg")
        return eFileType_mjpeg;

    if (ext == ".avi")
        return eFileType_avi;

    if (ext == ".mov")
        return eFileType_mov;

    if (ext == ".wmv")
        return eFileType_wmv;

    if (ext == ".mpg")
        return eFileType_mpg1;

    if (ext == ".mpeg")
        return eFileType_mpg1;

    if (ext == ".mp2")
        return eFileType_mpg2;

    if (ext == ".mpg2")
        return eFileType_mpg2;

    if (ext == ".mkv")
        return eFileType_mkv;

    if (ext == ".webm")
        return eFileType_webm;

    if (ext == ".info")
        return eFileType_info;

    return eFileType_unknown;
}


// Convert FourCC string to eVideoFileType_def
eVideoFileType_def fourCcToVideoFileType(const std::string &SFourCC)
{
    static std::map<std::string, eVideoFileType_def> map =
    {
        {"RAW", eFileType_raw},
        {"MPG4", eFileType_mpg4},
        {"MJPG", eFileType_mjpeg},
        {"AVI", eFileType_avi},
        {"MOV", eFileType_mov},
        {"WMV", eFileType_wmv},
        {"MPG1", eFileType_mpg1},
        {"MPG2", eFileType_mpg2},
        {"MKV", eFileType_mkv},
        {"WEBM", eFileType_webm},
        {"INFO", eFileType_info}
    };

    eVideoFileType_def type = eFileType_unknown;

    std::string sLookUp = StrUtils::toUpper(SFourCC);

    auto        it = map.find(sLookUp);
    if (it != map.end())
        type = it->second;

    return type;
}


std::string videoFormatToFourCC(eVideoDataIoFormat_def fmt)
{
    switch (fmt)
    {
        case eVideoDataIoFormat_def::eVideoDataIoFormat_yuv:
            return "YUV";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_yuy2:
            return "YUY2";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_yv12:
            return "YV12";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_nv12:
            return "NV12";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_rgb:
            return "RGB";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_bgr:
            return "BGR";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_mjpeg:
            return "MJPG";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_mpeg4:
            return "MPG4";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_mpeg2:
            return "MPG2";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_mpeg1:
            return "MPG1";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_h264:
            return "H264";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_h265:
            return "H265";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_vp7:
            return "VP7V";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_vp8:
            return "VP8";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_vp9:
            return "VP9";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_vc1:
            return "VC1";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_divx:
            return "DIVx";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_webm:
            return "WEBM";

        case eVideoDataIoFormat_def::eVideoDataIoFormat_i420:
            return "I420";
    }

    return "";
}


// Convert FourCC string to eVideoFileType_def
eVideoDataIoFormat_def fourCcToVideoFormat(const std::string& sFourCC)
{
    static std::map<std::string, eVideoDataIoFormat_def> map =
    {
        {"YUV", eVideoDataIoFormat_yuv},
        {"YUY2", eVideoDataIoFormat_yuy2},
        {"YUY2", eVideoDataIoFormat_yv12},
        {"YUY2", eVideoDataIoFormat_nv12},
        {"RGB", eVideoDataIoFormat_rgb},
        {"BGR", eVideoDataIoFormat_bgr},
        {"MJPG", eVideoDataIoFormat_mjpeg},
        {"H264", eVideoDataIoFormat_h264},
        {"H265", eVideoDataIoFormat_h265},
        {"MPG4", eVideoDataIoFormat_mpeg4},
        {"MPG2", eVideoDataIoFormat_mpeg2},
        {"MPG1", eVideoDataIoFormat_mpeg1},
        {"MPEG", eVideoDataIoFormat_mpeg1},
        {"DIVX", eVideoDataIoFormat_divx},
        {"WEBM", eVideoDataIoFormat_webm},
        {"VP7", eVideoDataIoFormat_vp7},
        {"VP8", eVideoDataIoFormat_vp8},
        {"VP9", eVideoDataIoFormat_vp9},
        {"VC1", eVideoDataIoFormat_vc1},
        {"I420", eVideoDataIoFormat_i420},
    };

    eVideoDataIoFormat_def type = eVideoDataIoFormat_unknown;

    std::string sLookUp = StrUtils::toUpper(sFourCC);

    auto        it = map.find(sLookUp);
    if (it != map.end())
        type = it->second;

    return type;
}


std::string FourCC2String(uint32_t fourcc)
{
    std::string out;

    for (uint32_t i = 0; i < 4; i++)
    {
        out += static_cast<char>(fourcc & 0xFF);

        fourcc >>= 8;
    }

    return out;
}


uint32_t String2FourCC(const std::string& fourcc)
{
    uint32_t out = 0;

    uint8_t* pChar = (uint8_t*)&out;

    for (uint32_t i = 0; i < 4; i++)
    {
        if (i >= fourcc.length())
            break;

        *(pChar + i) = fourcc.at(i);
    }

    return out;
}




// class CVideoFileIO static functions

std::shared_ptr<CVideoFileIO> CVideoFileIO::openFileTypeByExt
    (
        const std::string &sFilePath, 
        const eFileIoMode_def mode, 
        const int width,
        const int height,
        const int frameRate,
        const int bitsPerPixel,
        const std::string &sFourCC
    )
{
    if (sFilePath.empty())
    {
        LogDebug("file path not set");
        return nullptr;
    }

    std::shared_ptr<CVideoFileIO> pVFIO = nullptr;

    auto eFileType = getVideoFileType(sFilePath);

    if (eFileType == eFileType_raw)
    {
        // Open video file using 'raw' IO functions

        std::shared_ptr<CRawVideoFileIO> pRawFileIO = 
            std::make_shared<CRawVideoFileIO>();

        if (width > 0 || height > 0)
            pRawFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pRawFileIO->setFrameRate(frameRate);

        pRawFileIO->setBitsPerPixel(bitsPerPixel);
        
        if (sFourCC != "")
        {
            auto nVideoFormat = fourCcToVideoFormat(sFourCC);

            pRawFileIO->setVideoFormat(nVideoFormat);
        }

        if (mode == eFileIoMode_def::eFileIoMode_output)
            pRawFileIO->createInfoTextFile(true);

        if (!pRawFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file:{}", sFilePath);
            return pVFIO;
        }

        pVFIO = pRawFileIO;
    }
#ifdef SUPPORT_AVI_IO_LOGIC
    else if (eFileType == eFileType_avi)
    {
        // Open video file using 'raw' IO functions

        std::shared_ptr<CAviFileIO> pAviFileIO =
            std::make_shared<CAviFileIO>();

        if (width > 0 || height > 0)
            pAviFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pAviFileIO->setFrameRate(frameRate);

        //if (blockSize > 0)
        //    pAviFileIO->setIoBlockSize(blockSize);

        pAviFileIO->setVideoConfig(width, height, bitsPerPixel, 0, sFourCC);

        if (!pAviFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file:{}", sFilePath);
            return pVFIO;
        }

        pVFIO = pAviFileIO;
    }
#endif
#ifdef SUPPORT_MKV_IO_LOGIC
    else if (eFileType == eFileType_mkv)
    {
        // Open video file using 'mkv' IO functions

        std::shared_ptr<CMkvFileIO> pMkvFileIO =
            std::make_shared<CMkvFileIO>();

        if (width > 0 || height > 0)
            pMkvFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pMkvFileIO->setFrameRate(frameRate);

        //if (blockSize > 0)
        //    pMkvFileIO->setIoBlockSize(blockSize);

        pMkvFileIO->setBitsPerPixel(bitsPerPixel);

        if (!pMkvFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file:{}", sFilePath);
            return pVFIO;
        }

        pVFIO = pMkvFileIO;
    }
#endif
#ifdef SUPPORT_OCV_IO_LOGIC
    else
    {
        std::shared_ptr<COcvFileIO> pOcvFileIO = 
            std::make_shared<COcvFileIO>();

        if (width > 0 || height > 0)
            pOcvFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pOcvFileIO->setFrameRate(frameRate);

        pOcvFileIO->setBitsPerPixel(bitsPerPixel);

        if (!pOcvFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file:{}", sFilePath);
            return pVFIO;
        }

        pVFIO = pOcvFileIO;
    }
#endif

    return pVFIO;
}


/// CVideoFileIO class functions

CVideoFileIO::CVideoFileIO() 
{
    m_bitsPerPixel    	= 24;
    m_width       		= 0;
    m_height       		= 0;
    m_nFrameSize        = 0;
    m_frameRate        	= 0;
    m_sFilePath         = "";
    m_bFileOpened       = false;
    m_eMode             = eFileIoMode_unknown;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eVideoFileType_def::eFileType_unknown;
    m_eVideoFormat      = eVideoDataIoFormat_unknown;
}


CVideoFileIO::CVideoFileIO(unsigned int width, unsigned int height, unsigned int bitsPerPixel) 
{
    m_bitsPerPixel    	= 24;
	if (bitsPerPixel > 0)
	{
		m_bitsPerPixel = bitsPerPixel;
	}
    m_width       		= width;
    m_height       		= height;
    m_frameRate       	= 0;
    m_nFrameSize        = (width * height * (m_bitsPerPixel / 8));
    m_sFilePath         = "";
    m_bFileOpened       = false;
    m_eMode             = eFileIoMode_unknown;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eVideoFileType_def::eFileType_unknown;
    m_eVideoFormat      = eVideoDataIoFormat_unknown;
}


CVideoFileIO::CVideoFileIO(const std::string& sFilePath) :
    CVideoFileIO()
{
    m_sFilePath = sFilePath;
}


CVideoFileIO::~CVideoFileIO()
{
    m_bFileOpened = false;
}


void CVideoFileIO::setBitsPerPixel(int bits)
{
    m_bitsPerPixel = bits;
}


void CVideoFileIO::setFile(std::string &sFilePath)
{
    m_sFilePath = sFilePath;

    m_eFileType = getVideoFileType(m_sFilePath);
}


void CVideoFileIO::setLoopingRead(bool value)
{
    m_bUseLoopingRead = value;
}


void CVideoFileIO::setFrameSize(const unsigned int width, const unsigned int height, const unsigned int bitsPerPixel)
{
    m_width = width;
    m_height = height;
	if (bitsPerPixel > 0)
	{
		m_bitsPerPixel = bitsPerPixel;
	}
    m_nFrameSize = (width * height * (m_bitsPerPixel / 8));
}


void CVideoFileIO::setFrameRate(const unsigned int rate)
{
    m_frameRate = rate;
}


bool CVideoFileIO::isFileOpened() const
{
    return m_bFileOpened;
}


std::string CVideoFileIO::getFilePath()
{
    return m_sFilePath;
}


eVideoFileType_def CVideoFileIO::getFileType()
{
    return m_eFileType;
}


long CVideoFileIO::getNumFrames()
{
    return m_nFramesInFile;
}


unsigned int CVideoFileIO::getIoCount() const
{
    return (unsigned int) m_nIoCntr;
}


int CVideoFileIO::getCurrentFrame() const
{
    return (int) m_nCurrentFrame;
}


int CVideoFileIO::getCurrentFrameIndex() const
{
    return (int) m_nCurrentFrameIdx;
}


bool CVideoFileIO::resetPlayPosition()
{
    m_nCurrentFrameIdx = 0;

    return true;
}


/// CRawVideoFileIO class functions

CRawVideoFileIO::CRawVideoFileIO() :
    CVideoFileIO()
{
    LogTrace("class created");

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer = nullptr;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
    m_lCurrentFilePos = 0;
    m_bCreateInfoTextFile = false;
}


CRawVideoFileIO::CRawVideoFileIO(const unsigned int width, const unsigned int height, const unsigned int bitsPerPixel) :
    CVideoFileIO(width, height, bitsPerPixel)
{
    LogTrace("class created");

    m_eMode               	= eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer        	= nullptr;
    m_bitsPerPixel    		= 24;
	if (bitsPerPixel > 0)
	{
		m_bitsPerPixel = bitsPerPixel;
	}
    m_width       			= width;
    m_height       			= height;
    m_frameRate        		= 0;
    m_nFrameSize        	= (width * height * (m_bitsPerPixel / 8));
    m_nCurrentFrameIdx    	= 0;
    m_lFileSize           	= 0;
    m_lCurrentFilePos     	= 0;
    m_bCreateInfoTextFile 	= false;

    m_fileInfo.width        = m_width;
    m_fileInfo.height       = m_height;
    m_fileInfo.frameRate    = m_frameRate;
    m_fileInfo.bitsPerPixel = m_bitsPerPixel;
}


CRawVideoFileIO::CRawVideoFileIO(const std::string &sFilePath) :
    CVideoFileIO(sFilePath)
{
    LogTrace("class created");

    m_fileIO.setFilePath(sFilePath);
    m_bitsPerPixel    		= 24;
    m_width       			= 0;
    m_height       			= 0;
    m_frameRate        		= 0;
    m_eMode               	= eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer        	= nullptr;
    m_nCurrentFrameIdx    	= 0;
    m_lFileSize           	= 0;
    m_lCurrentFilePos     	= 0;
    m_bCreateInfoTextFile 	= false;
}


CRawVideoFileIO::~CRawVideoFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();
}


bool CRawVideoFileIO::parseInfoTextFile(const std::string &sFile, SVideoFormatInfo &info)
{
    std::fstream infoFile;

    infoFile.open(sFile, std::ios::in);
    if (!infoFile.is_open())
    {
        LogDebug("parseInfoFile(): ERROR: unable to open 'raw' videp info file: {}", sFile);
        return false;
    }

    info.clear();

    bool        bOut = false;

    std::string sInputText;

    while (true)
    {
        if (infoFile.eof())
            break;

        std::getline(infoFile, sInputText);

        if (sInputText.empty())
            continue;

        std::string sSearchText = "FrameWidth:";

        auto        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            int val = getNumericStringAt(sInputText, (unsigned int) sSearchText.length());

            if (val < 0)
                break;

            info.width = val;

            bOut = true;

            continue;
        }

        sSearchText = "FrameHeight:";

        pos = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            int val = getNumericStringAt(sInputText, (unsigned int)sSearchText.length());

            if (val < 0)
                break;

            info.height = val;

            bOut = true;

            continue;
        }

        sSearchText = "FrameRate:";

        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            int val = getNumericStringAt(sInputText, (unsigned int) sSearchText.length());

            if (val < 0)
                break;

            info.frameRate = val;

            bOut            = true;

            continue;
        }

        sSearchText = "PixelSize:";

        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            std::string sTemp = sInputText.substr(pos + sSearchText.length());

            sTemp             = removeLeadingSpaces(sTemp);

            bOut              = true;

            if (sTemp == "int8_t")
            {
                info.bitsPerPixel = 8;    
            }
            else if (sTemp == "int16_t")
            {
                info.bitsPerPixel = 16;    
            }
            else if (sTemp == "int24_t")
            {
                info.bitsPerPixel = 24;
            }
            else if (sTemp == "int32_t")
            {
                info.bitsPerPixel = 32;    
            }
            else if (sTemp == "int48_t")
            {
                info.bitsPerPixel = 48;
            }
            else
            {
                LogWarning("parseInfoFile(): WARNING: invalid 'frameSize' ({}) in 'raw' videp info file: {}", sTemp, sFile);
                bOut = false;
            }

            continue;
        }

        sSearchText = "FourCC:";

        pos = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            std::string sTemp = sInputText.substr(pos + sSearchText.length());

            sTemp = removeLeadingSpaces(sTemp);

            info.sFourCC = sTemp;

            bOut = true;

            continue;
        }

    }

    infoFile.close();

    return bOut;
}


bool CRawVideoFileIO::writeInfoTextFile(const std::string& sFile, SVideoFormatInfo& info)
{
    std::fstream infoFile;

    infoFile.open(sFile, std::ios::out);
    if (!infoFile.is_open())
    {
        LogDebug("parseInfoFile(): ERROR: unable to create 'raw' videp info file: {}", sFile);
        return false;
    }

    std::string sParamText;

    sParamText = ("FrameWidth: " + StrUtils::tos(info.width));

    infoFile << sParamText << std::endl;

    sParamText = ("FrameHeight: " + StrUtils::tos(info.height));

    infoFile << sParamText << std::endl;

    sParamText = ("FrameRate: " + StrUtils::tos(info.frameRate));

    infoFile << sParamText << std::endl;

    sParamText = "PixelSize: ";
        
    std::string sSize = "";

    switch (info.bitsPerPixel)
    {
        case 8:
            sSize = "int8_t";
            infoFile << sParamText << sSize << std::endl;
            break;

        case 16:
            sSize = "int16_t";
            infoFile << sParamText << sSize << std::endl;
            break;

        case 24:
            sSize = "int24_t";
            infoFile << sParamText << sSize << std::endl;
            break;

        case 32:
            sSize = "int32_t";
            infoFile << sParamText << sSize << std::endl;
            break;

        case 48:
            sSize = "int48_t";
            infoFile << sParamText << sSize << std::endl;
            break;

        default:
            LogWarning("parseInfoFile(): WARNING: invalid 'PixelSize' ({}) in 'raw' videp info file: {}", info.bitsPerPixel, sFile);
            return false;
    }

    sParamText = ("FourCC: " + info.sFourCC);

    infoFile << sParamText << std::endl;

    infoFile.close();

    return true;
}


bool CRawVideoFileIO::openFile(const eFileIoMode_def mode, const std::string &sFilePath)
{
    LogTrace("file path:{}", sFilePath);

    if (m_bFileOpened)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getVideoFileType(m_sFilePath);

    /// Allocate just 1 frame 
    switch (m_bitsPerPixel)
    {
    case 8:
    case 16:
    case 24:
    case 32:
        m_pFramebuffer = 
            calloc((m_bitsPerPixel / 8), (m_width * m_height));
        break;

    default:
        return false;
    }

    if (m_pFramebuffer == nullptr)
        return false;

    m_eMode = mode;

    m_fileIO.setBinaryMode(true);

    switch (m_eMode)
    {
        case eFileIoMode_input:
            {
                if (!m_fileIO.openFile(eFileIoMode_input, m_sFilePath))
                    return false;

                auto len = m_fileIO.getFileSize();
                if (len < 0)
                {
                    m_fileIO.closeFile();
                    return false;
                }

                /// Create out videp info text file
                std::string sInfoFilePath = getFileDir(m_sFilePath);    /// get the directory the file is in

                std::string sFileName = getFileName(m_sFilePath);       /// get the filename with no ext (.xxx)

                if (sInfoFilePath.empty() == false)
                    sInfoFilePath.append("/" + sFileName);
                else
                    sInfoFilePath.assign(sFileName);

                sInfoFilePath.append("-FileInfo.txt");                  /// append "-FileInfo.txt" to the filename

                parseInfoTextFile(sInfoFilePath, m_fileInfo);

                if (m_fileInfo.width > 0)
                    m_width = m_fileInfo.width;

                if (m_fileInfo.height > 0)
                    m_height = m_fileInfo.height;

                if (m_fileInfo.frameRate > 0)
                    m_frameRate = m_fileInfo.frameRate;

                if (m_fileInfo.bitsPerPixel > 0)
                    m_bitsPerPixel = m_fileInfo.bitsPerPixel;

                if (m_fileInfo.sFourCC != "")
                    m_eFileType = fourCcToVideoFileType(m_fileInfo.sFourCC);

                m_lFileSize       = len;
                m_nFramesInFile   = (len / m_nFrameSize);

                /// Set the read position to the beginning of the file.
                m_lCurrentFilePos = 0;
            }
            break;

        case eFileIoMode_output:
            {
                if (!m_fileIO.openFile(eFileIoMode_output, m_sFilePath))
                    return false;

                m_lFileSize       = 0;

                /// Set the write position to the end of the file.
                m_lCurrentFilePos = m_lFileSize;

                /// If "CreateInfoTextFile" option selected..
                if (m_bCreateInfoTextFile)
                {
                    m_fileInfo.width =          m_width;
                    m_fileInfo.height =         m_height;
                    m_fileInfo.frameRate =      m_frameRate;
                    m_fileInfo.bitsPerPixel =   m_bitsPerPixel;
                    m_fileInfo.sFourCC =        videoFormatToFourCC(m_eVideoFormat);

                    /// Create out videp info text file
                    std::string sInfoFilePath = getFileDir(m_sFilePath);    /// get the directory the file is in

                    std::string sFileName = getFileName(m_sFilePath);       /// get the filename with no ext (.xxx)

                    if (sInfoFilePath.empty() == false)
                        sInfoFilePath.append("/" + sFileName);
                    else
                        sInfoFilePath.assign(sFileName);

                    sInfoFilePath.append("-FileInfo.txt");                  /// append "-FileInfo.txt" to the filename

                    writeInfoTextFile(sInfoFilePath, m_fileInfo);
                }
            }
            break;

        default:
            return false;
    }

    m_nCurrentFrameIdx = 0;
    m_nIoCntr          = 0;
    m_nCurrentFrame    = 0;
    m_bFileOpened      = true;

    return true;
}


long CRawVideoFileIO::getNumFrames()
{
    LogTrace("getting number of frame in the file");

    if (!m_bFileOpened || m_eMode == eFileIoMode_unknown || m_eMode == eFileIoMode_output)
        return -1;

    /// Get the file length (which sets file pointer to EOF)
    long fileSize  = m_fileIO.getFileSize();
    
    /// Computer number of frames in the file
    int  numFrames = (int)((float) fileSize / (float)m_nFrameSize);

    /// Reset file pointer to the beginning of the file
    m_fileIO.setFilePosition(0);

    return numFrames;
}


bool CRawVideoFileIO::closeFile()
{
    LogTrace("file being closed");

    if (!m_bFileOpened)
        LogDebug("file close called when files is not open");

    /// close input file
    if (m_fileIO.isOpen())
    {
        if (!m_fileIO.closeFile())
        {
            LogCritical("CRawVideoFileIO - Error: unable to close file ");
            return false;
        }

        try
        {
            /// (If allocated) free the frameBuffer
            if (m_pFramebuffer != nullptr)
            {
                auto pTmp      = m_pFramebuffer;

                m_pFramebuffer = nullptr;
                free(pTmp);
            }
        }
        catch (...)
        {
        }
    }

    m_nCurrentFrameIdx = 0;
    m_nIoCntr          = -1;
    m_nCurrentFrame    = -1;
    m_eMode            = eFileIoMode_def::eFileIoMode_unknown;
    m_bFileOpened      = false;

    return true;
}


bool CRawVideoFileIO::isEOF()
{
    if (m_nCurrentFrameIdx >= m_nFramesInFile)
    {
        return true;
    }

    return false;
}



/// Read a frame from a "raw" data file
bool CRawVideoFileIO::readVideoFrame(void *pData)
{
    if (!m_bFileOpened || m_pFramebuffer == nullptr)
    {
        return false;
    }

    if (isEOF() == true)
    {
        return false;
    }

    /// This "read" logic reads 1 "frame" at a time 
    // from the input file.

    bool status = m_fileIO.readBlock(pData, m_nFrameSize, 1);

    if (status == false)
    {
        return false;
    }

    return true;
}


bool CRawVideoFileIO::readVideoBlock(void *pData, const unsigned int numFrames)
{
    LogTrace("numFrames:{}", numFrames);

    if (!m_bFileOpened || pData == nullptr || numFrames < 1)
    {
        return false;
    }

    if (isEOF() == true)
    {
        return false;
    }

    auto nFramesLeftInFile = ((m_lFileSize - m_lCurrentFilePos) / m_nFrameSize);

    unsigned int nReadSize = 0;

    bool status = false;

    if (nFramesLeftInFile < numFrames)
    {
        /// If our next read would exceed the file size...

        if (nFramesLeftInFile > 0)
        {
            // read what's left in the file

            status = m_fileIO.readBlock(pData, m_nFrameSize, nFramesLeftInFile);

            if (status == false)
            {
                return false;
            }
        }

        auto nNumFramesNotRead = (numFrames - nFramesLeftInFile);

        /// If the "UseLoopingRead" flag = false
        /// don't try to read anymore.
        if (!m_bUseLoopingRead)
        {
            if (nFramesLeftInFile < 0)
            {
                // there was nothing left to read
                return false;
            }

            // zero out/pad the rest of the frames (from chosen read size)

            auto nPadSize = 0;

            void *pPadStart = nullptr;

            if (m_bitsPerPixel == 16)
            {
                nPadSize = ((numFrames - nFramesLeftInFile) * m_nFrameSize * sizeof(int16_t));
                pPadStart = (void *) (((int16_t *) pData) + (nFramesLeftInFile * m_nFrameSize));
            }
            else if (m_bitsPerPixel == 32)
            {
                nPadSize = ((numFrames - nFramesLeftInFile) * m_nFrameSize * sizeof(int32_t));
                pPadStart = (void *) (((int32_t *) pData) + (nFramesLeftInFile * m_nFrameSize));
            }
            else
            {
                return false;
            }

            memset(pPadStart, 0, nPadSize);

            return true;
        }

        /// If the "UseLoopingRead" flag = true
        /// seek back to the beginning of the file

        if (!m_fileIO.setFilePosition(0))
        {
            // set file pos failed
            m_fileIO.closeFile();
            return false;
        }

        m_lCurrentFilePos = 0;

        m_nCurrentFrame = 0;

        nReadSize = nNumFramesNotRead;

        status = m_fileIO.readBlock(pData, m_nFrameSize, nReadSize);

        if (status == false)
        {
            return false;
        }
    }
    else
    {
        /// Read n frames of frames from the input file.

        nReadSize = numFrames;

        status = m_fileIO.readBlock(pData, m_nFrameSize, nReadSize);
    }

    /// Update the file read position
#ifdef UPDATE_FILE_POSITION
    m_lCurrentFilePos = m_fileIO.getFilePosition();
#else
    auto nReadSize = m_fileIO.getLastIoSize();

    if (nReadSize > 0)
    {
        m_lCurrentFilePos += nReadSize;
    }
#endif

    if (status == true)
    {
        m_nCurrentFrame += nReadSize;
    }

    return status;
}


bool CRawVideoFileIO::writeVideoFrame(const void *pData)
{
    if (pData == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid param");
        return false;
    }

    if (m_nFrameSize < 1)
    {
        LogDebug("writeVideoFrame called with invalid m_nFrameSize");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}", 
            (int) m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// at a time to the output file.

    bool status = m_fileIO.writeBlock(pData, m_nFrameSize, 1);

    if (status == false)
    {
        return false;
    }

    return true;
}


bool CRawVideoFileIO::writeVideoFrame(const void* pData, const unsigned int frameLen)
{
    if (pData == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid param");
        return false;
    }

    if (frameLen < 1)
    {
        LogDebug("writeVideoFrame called with invalid frameLen");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// at a time to the output file.

    bool status = m_fileIO.writeBlock(pData, frameLen, 1);

    if (status == false)
    {
        return false;
    }

    return true;
}


bool CRawVideoFileIO::writeVideoBlock(const void *pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - bFileOpened:{}, eMode:{}", 
            m_bFileOpened, 
            (int) m_eMode
        );
        return false;
    }

    bool status = m_fileIO.writeBlock(pData, m_nFrameSize, numFrames);

#ifdef UPDATE_FILE_POSITION
    m_lCurrentFilePos = m_fileIO.getFilePosition();
#else
    auto pos = m_fileIO.getLastIoSize();

    if (pos > 0)
        m_lCurrentFilePos += pos;
#endif
    if (status)
    {
        m_nCurrentFrame += numFrames;
        m_nFramesInFile += numFrames;
    }

    return status;
}


bool CRawVideoFileIO::setCurrentFrame(unsigned int frameNum) 
{
    if (!m_bFileOpened)
        return false;

    unsigned int newFilePos = (frameNum * m_nFrameSize);

    if (newFilePos >= m_lFileSize)
        return false;

    if (!m_fileIO.setFilePosition(newFilePos))
    {
        return false;
    }

    m_lCurrentFilePos = newFilePos;

    return true;
}


bool CRawVideoFileIO::resetPlayPosition()
{
    setCurrentFrame(0);

    CVideoFileIO::resetPlayPosition();

    return true;
}


/// CAviFileIO class functions

#ifdef SUPPORT_AVI_IO_LOGIC

CAviFileIO::CAviFileIO() :
    CVideoFileIO()
{
    LogTrace("class created");

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;

    m_fileInfo.clear();

    //m_pFramebuffer = nullptr;

    memset(&m_audioInfo, 0, sizeof(m_audioInfo));
}


CAviFileIO::CAviFileIO
    (
        const unsigned int width, 
        const unsigned int height, 
        const unsigned int bitsPerPixel, 
        const unsigned int nFrameRate, 
        const std::string &sFourCC
    ) :
    CVideoFileIO(width, height, bitsPerPixel)
{
    LogTrace("class created");

    m_pFileCtrl = nullptr;

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;

    if (bitsPerPixel > 0)
    {
        m_bitsPerPixel = bitsPerPixel;
    }
    else
    {
        m_bitsPerPixel = 24;
    }
    
    m_width = width;
    m_height = height;
    m_frameRate = 0;
    m_nFrameSize = (width * height * (m_bitsPerPixel / 8));

    m_fileInfo.width = m_width;
    m_fileInfo.height = m_height;
    m_fileInfo.frameRate = m_frameRate;
    m_fileInfo.bitsPerPixel = m_bitsPerPixel;

    m_fileInfo.sVideo4CC = sFourCC;

    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;

    //m_pFramebuffer = nullptr;

    memset(&m_audioInfo, 0, sizeof(m_audioInfo));
}


CAviFileIO::CAviFileIO(const std::string& sFilePath) :
    CVideoFileIO(sFilePath)
{
    LogTrace("class created");

    m_pFileCtrl = nullptr;

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;

    m_fileInfo.clear();

    //m_pFramebuffer = nullptr;

    memset(&m_audioInfo, 0, sizeof(m_audioInfo));
}


CAviFileIO::~CAviFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();
}


void CAviFileIO::setVideoConfig
    (
        const unsigned int width,
        const unsigned int height,
        const unsigned int bitsPerPixel,
        const unsigned int nFrameRate,
        const std::string& sFourCC
    )
{
    if (bitsPerPixel > 0)
    {
        m_bitsPerPixel = bitsPerPixel;
    }
    else
    {
        m_bitsPerPixel = 24;
    }

    m_width = width;
    m_height = height;
    m_nFrameSize = (width * height * (m_bitsPerPixel / 8));

    if (nFrameRate > 0)
    {
        m_frameRate = nFrameRate;
    }

    m_fileInfo.width = m_width;
    m_fileInfo.height = m_height;
    m_fileInfo.frameRate = m_frameRate;
    m_fileInfo.bitsPerPixel = m_bitsPerPixel;

    if (sFourCC != "")
    {
        m_fileInfo.sVideo4CC = sFourCC;
    }
}


void CAviFileIO::setAudioConfig
    (
        int         numTracks,
        int         bitsPerSample,
        int         sampleRate,
        std::string sFourCC
    )
{
    m_audioInfo.channels = numTracks;
    m_audioInfo.samples_per_second = sampleRate;
    m_audioInfo.bits = bitsPerSample;
}


bool CAviFileIO::openFile(const eFileIoMode_def mode, const std::string& sFilePath)
{
    LogTrace("file path:{}", sFilePath);

    if (m_bFileOpened)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getVideoFileType(m_sFilePath);

    if (m_eFileType != eFileType_avi)
        return false;

    m_pFileCtrl = 
        gwavi_open
        (
            m_sFilePath.c_str(),
            m_width,
            m_height,
            m_fileInfo.sVideo4CC.c_str(),     //m_sFourCC.c_str(),
            m_bitsPerPixel, 
            m_frameRate,
            &m_audioInfo
         );

    if (m_pFileCtrl == nullptr)
        return false;

    m_eMode = mode;

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = 0;
    m_nCurrentFrame = 0;
    m_bFileOpened = true;

    return true;
}


long CAviFileIO::getNumFrames()
{
    LogTrace("getting number of frame in the file");

    if (!m_bFileOpened || m_eMode == eFileIoMode_unknown || m_eMode == eFileIoMode_output)
        return -1;

    auto numFrames = -1;

    return numFrames;
}


bool CAviFileIO::closeFile()
{
    LogTrace("file being closed");

    if (!m_bFileOpened)
    { 
        LogDebug("file close called when files is not open");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("file close called with invalid m_pFileCtrl");
        return false;
    }

    bool bRetValue = true;

    auto status = gwavi_close(m_pFileCtrl);

    if (status != 0)
        bRetValue = false;

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = -1;
    m_nCurrentFrame = -1;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_bFileOpened = false;

    return bRetValue;
}


bool CAviFileIO::isEOF()
{
    if (m_nCurrentFrameIdx >= m_nFramesInFile)
    {
        return true;
    }

    return false;
}



/// Read a video frame from a "AVI" data file
bool CAviFileIO::readVideoFrame(void* pData)
{
    return false;
}


bool CAviFileIO::readVideoBlock(void* pData, const unsigned int numFrames)
{
    return false;
}


bool CAviFileIO::writeVideoFrame(const void* pData)
{
    if (pData == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid param");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid m_pFileCtrl");
        return false;
    }

    if (m_nFrameSize < 1)
    {
        LogDebug("writeVideoFrame called with invalid m_nFrameSize");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 video frame
    /// at a time to the output file.

    bool status = gwavi_add_frame(m_pFileCtrl, (const unsigned char*)pData, m_nFrameSize);

    if (status != 0)
    {
        return false;
    }

    return true;
}


bool CAviFileIO::writeVideoFrame(const void* pData, unsigned int frameSize)
{
    if (pData == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid param");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid m_pFileCtrl");
        return false;
    }

    if (frameSize < 1)
    {
        LogDebug("writeVideoFrame called with invalid frameSize");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 video frame
    /// at a time to the output file.

    bool status = gwavi_add_frame(m_pFileCtrl, (const unsigned char*) pData, frameSize);

    if (status != 0)
    {
        return false;
    }

    return true;
}


bool CAviFileIO::writeVideoBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1)
    {
        LogDebug("writeVideoBlock called with invalid param");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("writeVideoBlock called with invalid m_pFileCtrl");
        return false;
    }

    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - bFileOpened:{}, eMode:{}",
            m_bFileOpened,
            (int) m_eMode
        );
        return false;
    }

    unsigned int nDataOffset = 0;

    for (unsigned int f = 0; f < numFrames; f++)
    {
        auto status = writeVideoFrame(((unsigned char *) pData) + nDataOffset);

        if (status != 0)
        {
            return false;
        }

        nDataOffset += m_nFrameSize;
    }

    return true;
}


/// Read a audio frame from a "AVI" data file
bool CAviFileIO::readAudioFrame(void* pData)
{
    return false;
}


bool CAviFileIO::readAudioBlock(void* pData, const unsigned int numFrames)
{
    return false;
}


bool CAviFileIO::writeAudioFrame(const void* pData)
{
    if (pData == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid param");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("writeVideoFrame called with invalid m_pFileCtrl");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    unsigned int nAudioFrameSize = (m_audioInfo.channels * (m_audioInfo.bits / 8));

    /// This "write" logic writes 1 audio "frame" (numChannels * sampleSize)
    /// at a time to the output file.

    bool status = gwavi_add_audio(m_pFileCtrl, (const unsigned char*) pData, nAudioFrameSize);

    if (status != 0)
    {
        return false;
    }

    return true;
}


bool CAviFileIO::writeAudioBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1)
    {
        LogDebug("writeVideoBlock called with invalid param");
        return false;
    }

    if (m_pFileCtrl == nullptr)
    {
        LogDebug("writeVideoBlock called with invalid m_pFileCtrl");
        return false;
    }

    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input)
    {
        LogDebug
        (
            "bad param - bFileOpened:{}, eMode:{}",
            m_bFileOpened,
            (int)m_eMode
        );
        return false;
    }

    unsigned int nAudioFrameSize = (m_audioInfo.channels * (m_audioInfo.bits / 8));

    bool status = gwavi_add_audio(m_pFileCtrl, (const unsigned char*) pData, (nAudioFrameSize * numFrames));

    if (status != 0)
    {
        return false;
    }

    return true;
}


bool CAviFileIO::setCurrentFrame(unsigned int frameNum)
{
    if (!m_bFileOpened)
        return false;



    return false;
}


bool CAviFileIO::resetPlayPosition()
{
    setCurrentFrame(0);

    CVideoFileIO::resetPlayPosition();

    return true;
}

#endif


/// CMkvFileIO class functions

#ifdef SUPPORT_MKV_IO_LOGIC

CMkvFileIO::CMkvFileIO() :
    CVideoFileIO()
{
    LogTrace("class created");

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer = nullptr;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


CMkvFileIO::CMkvFileIO(const unsigned int width, const unsigned int height, const unsigned int bitsPerPixel) :
    CVideoFileIO(width, height, bitsPerPixel)
{
    LogTrace("class created");

    m_pFileCtrl = nullptr;

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer = nullptr;
    m_bitsPerPixel = 24;
    if (bitsPerPixel > 0)
    {
        m_bitsPerPixel = bitsPerPixel;
    }
    m_width = width;
    m_height = height;
    m_frameRate = 0;
    m_nFrameSize = (width * height * (m_bitsPerPixel / 8));
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


CMkvFileIO::CAviFileIO(const std::string& sFilePath) :
    CVideoFileIO(sFilePath)
{
    LogTrace("class created");

    m_pFileCtrl = nullptr;

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer = nullptr;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


CMkvFileIO::~CMkvFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();
}


bool CMkvFileIO::openFile(const eFileIoMode_def mode, const std::string& sFilePath)
{
    LogTrace("file path:{}", sFilePath);

    if (m_bFileOpened)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getVideoFileType(m_sFilePath);

    if (m_eFileType != eFileType_avi)
        return false;

    m_pFileCtrl =
        gwavi_open
        (
            m_sFilePath.c_str(),
            m_width,
            m_height,
            m_sFourCC.c_str(),
            m_frameRate,
            &m_audioInfo
        );

    if (m_pFileCtrl == nullptr)
        return false;

    m_eMode = mode;

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = 0;
    m_nCurrentFrame = 0;
    m_bFileOpened = true;

    return true;
}


long CMkvFileIO::getNumFrames()
{
    LogTrace("getting number of frame in the file");

    if (!m_bFileOpened || m_eMode == eFileIoMode_unknown || m_eMode == eFileIoMode_output)
        return -1;

    auto numFrames = -1;

    return numFrames;
}


bool CMkvFileIO::closeFile()
{
    LogTrace("file being closed");

    if (!m_bFileOpened)
    {
        LogDebug("file close called when files is not open");
        return false;
    }

    bool bRetValue = true;

    auto status = gwavi_close(m_pFileCtrl);

    if (status < 0)
        bRetValue = false;

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = -1;
    m_nCurrentFrame = -1;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_bFileOpened = false;

    return bRetValue;
}


bool CMkvFileIO::isEOF()
{
    if (m_nCurrentFrameIdx >= m_nFramesInFile)
    {
        return true;
    }

    return false;
}



/// Read a frame from a "MKV" data file
bool CMkvFileIO::readVideoFrame(void* pData)
{
    if (!m_bFileOpened || m_pFramebuffer == nullptr)
    {
        return false;
    }

    if (isEOF() == true)
    {
        return false;
    }

    auto status = gwavi_read

        if (status == false)
        {
            return false;
        }

    return true;
}


bool CMkvFileIO::readReadBlock(void* pData, const unsigned int numFrames)
{
    LogTrace("numFrames:{}", numFrames);

    if (!m_bFileOpened || pData == nullptr || numFrames < 1)
    {
        return false;
    }

    if (isEOF() == true)
    {
        return false;
    }

    auto nFramesLeftInFile = ((m_lFileSize - m_lCurrentFilePos) / m_nFrameSize);

    unsigned int nReadSize = 0;

    bool status = false;

    if (nFramesLeftInFile < numFrames)
    {
        /// If our next read would exceed the file size...

        if (nFramesLeftInFile > 0)
        {
            // read what's left in the file

            status = m_fileIO.readBlock(pData, m_nFrameSize, nFramesLeftInFile);

            if (status == false)
            {
                return false;
            }
        }

        auto nNumFramesNotRead = (numFrames - nFramesLeftInFile);

        /// If the "UseLoopingRead" flag = false
        /// don't try to read anymore.
        if (!m_bUseLoopingRead)
        {
            if (nFramesLeftInFile < 0)
            {
                // there was nothing left to read
                return false;
            }

            // zero out/pad the rest of the frames (from chosen read size)

            auto nPadSize = 0;

            void* pPadStart = nullptr;

            if (m_bitsPerPixel == 16)
            {
                nPadSize = ((numFrames - nFramesLeftInFile) * m_nFrameSize * sizeof(int16_t));
                pPadStart = (void*)(((int16_t*)pData) + (nFramesLeftInFile * m_nFrameSize));
            }
            else if (m_bitsPerPixel == 32)
            {
                nPadSize = ((numFrames - nFramesLeftInFile) * m_nFrameSize * sizeof(int32_t));
                pPadStart = (void*)(((int32_t*)pData) + (nFramesLeftInFile * m_nFrameSize));
            }
            else
            {
                return false;
            }

            memset(pPadStart, 0, nPadSize);

            return true;
        }

        /// If the "UseLoopingRead" flag = true
        /// seek back to the beginning of the file

        if (!m_fileIO.setFilePosition(0))
        {
            // set file pos failed
            m_fileIO.closeFile();
            return false;
        }

        m_lCurrentFilePos = 0;

        m_nCurrentFrame = 0;

        nReadSize = nNumFramesNotRead;

        status = m_fileIO.readBlock(pData, m_nFrameSize, nReadSize);

        if (status == false)
        {
            return false;
        }
    }
    else
    {
        /// Read n frames of frames from the input file.

        nReadSize = numFrames;

        status = m_fileIO.readBlock(pData, m_nFrameSize, nReadSize);
    }

    /// Update the file read position
#ifdef UPDATE_FILE_POSITION
    m_lCurrentFilePos = m_fileIO.getFilePosition();
#else
    auto nReadSize = m_fileIO.getLastIoSize();

    if (nReadSize > 0)
    {
        m_lCurrentFilePos += nReadSize;
    }
#endif

    if (status == true)
    {
        m_nCurrentFrame += nReadSize;
    }

    return status;
}


bool CMkvFileIO::writeVideoFrame(const void* pData)
{
    if (m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// at a time to the output file.

    bool status = m_fileIO.writeBlock(pData, m_nFrameSize, 1);

    if (status == false)
    {
        return false;
    }

    return true;
}


bool CMkvFileIO::writeVideoFrame(const void* pData, const unsigned int frameLen)
{
    if (m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// at a time to the output file.

    bool status = m_fileIO.writeBlock(pData, frameLen, 1);

    if (status == false)
    {
        return false;
    }

    return true;
}


bool CMkvFileIO::writeVideoBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - bFileOpened:{}, eMode:{}",
            m_bFileOpened,
            (int)m_eMode
        );
        return false;
    }

    bool status = m_fileIO.writeBlock(pData, m_nFrameSize, numFrames);

    auto pos = m_fileIO.getLastIoSize();

    return status;
}


bool CMkvFileIO::setCurrentFrame(unsigned int frameNum)
{
    if (!m_bFileOpened)
        return false;



    return false;
}


bool CMkvFileIO::resetPlayPosition()
{
    setCurrentFrame(0);

    CVideoFileIO::resetPlayPosition();

    return true;
}

#endif


/// COcvFileIO class functions

#ifdef SUPPORT_OCV_IO_LOGIC

COcvFileIO::COcvFileIO() :
    CVideoFileIO()
{
    LogTrace("class created");

    m_pFileInput = nullptr;
    m_pFileOutput = nullptr;
    m_pCvFrame = nullptr;

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


COcvFileIO::COcvFileIO(const unsigned int width, const unsigned int height, const unsigned int bitsPerPixel) :
    CVideoFileIO(width, height, bitsPerPixel)
{
    LogTrace("class created");

    m_pFileInput = nullptr;
    m_pFileOutput = nullptr;
    m_pCvFrame = nullptr;

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_bitsPerPixel = 24;
    if (bitsPerPixel > 0)
    {
        m_bitsPerPixel = bitsPerPixel;
    }
    m_width = width;
    m_height = height;
    m_frameRate = 0;
    m_nFrameSize = (width * height * (m_bitsPerPixel / 8));
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


COcvFileIO::COcvFileIO(const std::string& sFilePath) :
    CVideoFileIO(sFilePath)
{
    LogTrace("class created");

    m_pFileInput = nullptr;
    m_pFileOutput = nullptr;
    m_pCvFrame = nullptr;

    m_bitsPerPixel = 24;
    m_width = 0;
    m_height = 0;
    m_frameRate = 0;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_nCurrentFrameIdx = 0;
    m_lFileSize = 0;
}


COcvFileIO::~COcvFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();

    if (m_pCvFrame != nullptr)
    {
        delete m_pCvFrame;

        m_pCvFrame = nullptr;
    }
}


bool COcvFileIO::openFile(const eFileIoMode_def mode, const std::string& sFilePath)
{
    LogTrace("file path:{}", sFilePath);

    if (m_bFileOpened)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getVideoFileType(m_sFilePath);

    switch (mode)
    {
    case eFileIoMode_def::eFileIoMode_input:
        {
            m_pFileInput = new cv::VideoCapture(sFilePath);

            if (m_pFileInput == nullptr)
                return false;
            
            auto openCvFormat = (int) m_pFileInput->get(cv::CAP_PROP_FOURCC);
            auto eVideoFmt = cvFourCcToVideoFmt(openCvFormat);

            setVideoFormat((int) eVideoFmt);

            m_width = (unsigned int) m_pFileInput->get(cv::CAP_PROP_FRAME_WIDTH);
            m_height = (unsigned int) m_pFileInput->get(cv::CAP_PROP_FRAME_HEIGHT);
            m_frameRate = (unsigned int) m_pFileInput->get(cv::CAP_PROP_FPS);

            m_pCvFrame = new  cv::Mat;

            if (m_pCvFrame == nullptr)
            {
                return false;
            }
        }
        break;

    case eFileIoMode_def::eFileIoMode_output:
        {
            auto nFourCC = String2FourCC(m_sFourCC);
            auto eVideoFmt = cvFourCcToVideoFmt(nFourCC);

            setVideoFormat((int) eVideoFmt);
            
            cv::Size frameSize;
            
            frameSize.width = m_width;
            frameSize.height = m_height;
            
            m_pFileOutput = new cv::VideoWriter(sFilePath, (int) nFourCC, (double) m_frameRate, frameSize);
            
            if (m_pFileOutput == nullptr)
                return false;

            //m_pFileOutput->set(cv::CAP_PROP_FRAME_WIDTH, (double) m_width);
            //m_pFileOutput->set(cv::CAP_PROP_FRAME_HEIGHT, (double) m_height);
            //m_pFileOutput->set(cv::CAP_PROP_FPS, (double) m_frameRate);

            m_pCvFrame = new  cv::Mat(m_height, m_width, CV_8UC3);

            if (m_pCvFrame == nullptr)
            {
                return false;
            }
        }
        break;

    default:
        return false;
    }

    m_eMode = mode;

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = 0;
    m_nCurrentFrame = 0;
    m_bFileOpened = true;

    return true;
}


long COcvFileIO::getNumFrames()
{
    LogTrace("getting number of frame in the file");

    if (!m_bFileOpened || m_eMode == eFileIoMode_unknown || m_eMode == eFileIoMode_output)
        return -1;

    auto numFrames = -1;

    return numFrames;
}


bool COcvFileIO::closeFile()
{
    LogTrace("file being closed");

    if (!m_bFileOpened)
    {
        LogDebug("file close called when files is not open");
        return false;
    }

    switch (m_eMode)
    {
    case eFileIoMode_def::eFileIoMode_input:
    {
        if (m_pFileInput == nullptr)
            return false;

        m_pFileInput->release();

        delete m_pFileInput;

        m_pFileInput = nullptr;
    }
    break;

    case eFileIoMode_def::eFileIoMode_output:
    {
        if (m_pFileOutput == nullptr)
            return false;

        m_pFileOutput->release();

        delete m_pFileOutput;

        m_pFileOutput = nullptr;
    }
    break;

    default:
        return false;
    }

    m_nCurrentFrameIdx = 0;
    m_nIoCntr = -1;
    m_nCurrentFrame = -1;
    m_eMode = eFileIoMode_def::eFileIoMode_unknown;
    m_bFileOpened = false;

    return true;
}


bool COcvFileIO::isEOF()
{
    if (m_nCurrentFrameIdx >= m_nFramesInFile)
    {
        return true;
    }

    return false;
}


/// Read a video frame from file
bool COcvFileIO::readVideoFrame(void* pData)
{
    if (!m_bFileOpened || m_eMode == eFileIoMode_output || m_eMode == eFileIoMode_IO)
    {
        return false;
    }

    if (m_nFrameSize < 1)
    {
        return false;
    }

    if (m_pFileInput == nullptr)
    { 
        return false;
    }

    //if (isEOF() == true)
    //{
    //    return false;
    //}

    bool bRetValue = true;

    try
    {
        m_pFileInput->read(*m_pCvFrame);
        //*m_pFileInput >> *m_pCvFrame;

        if (m_pCvFrame->empty())
            return false;

        if (readFromCvFrame(pData, *m_pCvFrame, m_nFrameSize) == false)
            bRetValue = false;

        m_nCurrentFrame++;

        m_lastFrameRead++;
    }
    catch (...)
    {
        return false;
    }

    return bRetValue;
}


bool COcvFileIO::readVideoBlock(void* pData, const unsigned int numFrames)
{
    LogTrace("numFrames:{}", numFrames);

    if (!m_bFileOpened || pData == nullptr || numFrames < 1)
    {
        return false;
    }

    if (m_nFrameSize < 1)
    {
        return false;
    }

    //if (isEOF() == true)
    //{
    //    return false;
    //}

    bool bRetValue = true;

    int8_t* pCurFrame = (int8_t*) pData;

    for (unsigned int f = 0; f < numFrames; f++)
    {
        if (readFrame(pCurFrame) == false)
        {
            bRetValue = false;
        }

        pCurFrame = (pCurFrame + m_nFrameSize);
    }

    return bRetValue;
}


bool COcvFileIO::writeVideoFrame(const void* pData)
{
    if (m_eMode == eFileIoMode_input || m_eMode == eFileIoMode_IO)
    {
        LogDebug
        (
            "bad param - eMode:{}",
            (int)m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// at a time to the output file.

    if (m_pFileOutput == nullptr)
        return false;

    if (m_nFrameSize < 1)
        return false;

    bool bRetValue = true;

    try
    {
        if (writeToCvFrame(*m_pCvFrame, pData, m_nFrameSize) == false)
            bRetValue = false;

        m_pFileOutput->write(*m_pCvFrame);

        m_nCurrentFrame++;

        m_lastFrameWritten++;
    }
    catch (...)
    {
        return false;
    }

    return bRetValue;
}


bool COcvFileIO::writeVideoBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_eMode == eFileIoMode_IO)
    {
        LogDebug
        (
            "bad param - bFileOpened:{}, eMode:{}",
            m_bFileOpened,
            (int)m_eMode
        );
        return false;
    }

    if (m_nFrameSize < 1)
    { 
        return false;
    }

    bool bRetValue = true;

    int8_t *pCurFrame = (int8_t*) pData;

    for (unsigned int f = 0; f < numFrames; f++)
    {
        if (writeFrame(pCurFrame) == false)
        {
            bRetValue = false;
        }

        pCurFrame = (pCurFrame + m_nFrameSize);
    }

    return bRetValue;
}


bool COcvFileIO::readAudioFrame(void* pData)
{
    LogWarning("readAudioFrame Not currently implementedf");
    return false;
}


bool COcvFileIO::readVideoBlock(void* pData, const unsigned int numFrames)
{
    LogWarning("readVideoBlock Not currently implementedf");
    return false;
}


bool COcvFileIO::writeVideoFrame(const void* pData)
{
    LogWarning("writeVideoFrame Not currently implementedf");
    return false;
}


bool COcvFileIO::writeVideoBlock(const void* pData, const unsigned int numFrames)
{
    LogWarning("writeVideoBlock Not currently implementedf");
    return false;
}


bool COcvFileIO::setCurrentFrame(unsigned int frameNum)
{
    if (!m_bFileOpened)
        return false;


    return false;
}


bool COcvFileIO::resetPlayPosition()
{
    setCurrentFrame(0);


    return true;
}

#endif
