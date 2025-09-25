/// 
/// \file       CVideoFileIO.cpp
/// 
///             CVideoFileIO function definitions
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



std::string videpFileTypeToString(eVideoFileType_def value)
{
    static std::map<eVideoFileType_def, std::string> map = 
    {
        {eFileType_raw, "raw"},   
        {eFileType_mp4, "mp4"},   
        {eFileType_mov, "mov"},   
        {eFileType_avi, "avi"},
        {eFileType_avi, "mpg"},
        {eFileType_wmv, "wmv"},
        {eFileType_mkv, "mkv"},
        {eFileType_webm, "webm"},
        {eFileType_info, "info"}, 
        {eFileType_text, "text"},
    };

    std::string name = "unknown value " + std::to_string(static_cast<int>(value));
    
    auto        it   = map.find(value);
    if (it != map.end())
        name = it->second;
    
    return name;
}


eVideoFileType_def getVideoFileType(const std::string &filepath)
{
    if (filepath.empty())
        return eFileType_unknown;

    std::string ext = std::filesystem::path(filepath).extension().string();

    //ext             = strToLower(ext);

    if (ext.empty() || ext == ".raw")
        return eFileType_raw;

    if (ext == ".mp4")
        return eFileType_mp4;

    if (ext == ".mov")
        return eFileType_mov;

    if (ext == ".avi")
        return eFileType_avi;

    if (ext == ".mpg")
        return eFileType_mpg;

    if (ext == ".wmv")
        return eFileType_wmv;

    if (ext == ".mkv")
        return eFileType_mkv;

    if (ext == ".webm")
        return eFileType_webm;

    if (ext == ".info")
        return eFileType_info;

    if (ext == ".txt" || ext == ".text")
        return eFileType_text;

    return eFileType_unknown;
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
        int width,
        int height,
        int frameRate, 
        int blockSize,
        int bitsPerPixel
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

        if (blockSize > 0)
            pRawFileIO->setIoBlockSize(blockSize);

        pRawFileIO->setBitsPerPixel(bitsPerPixel);        

        if (mode == eFileIoMode_def::eFileIoMode_output)
            pRawFileIO->createInfoTextFile(true);

        if (!pRawFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file={}", sFilePath);
            return pVFIO;
        }

        pVFIO = pRawFileIO;
    }
#if 0
    else if (eFileType == eFileType_avi)
    {
        // Open video file using 'raw' IO functions

        std::shared_ptr<CAviFileIO> pAviFileIO =
            std::make_shared<CAviFileIO>();

        if (width > 0 || height > 0)
            pAviFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pAviFileIO->setFrameRate(frameRate);

        if (blockSize > 0)
            pAviFileIO->setIoBlockSize(blockSize);

        pAviFileIO->setBitsPerPixel(bitsPerPixel);

        if (!pAviFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file={}", sFilePath);
            return pVFIO;
        }

        pVFIO = pAviFileIO;
    }
#endif
    else
    {
        std::shared_ptr<COcvFileIO> pOcvFileIO = 
            std::make_shared<COcvFileIO>();

        if (width > 0 || height > 0)
            pOcvFileIO->setFrameSize(width, height);

        if (frameRate > 0)
            pOcvFileIO->setFrameRate(frameRate);

        if (blockSize > 0)
            pOcvFileIO->setIoBlockSize(blockSize);

        pOcvFileIO->setBitsPerPixel(bitsPerPixel);

        if (!pOcvFileIO->openFile(mode, sFilePath))
        {
            LogDebug("unable to open file={}", sFilePath);
            return pVFIO;
        }

        pVFIO = pOcvFileIO;
    }

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
    m_nIoBlockSize      = 0;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eVideoFileType_def::eFileType_unknown;
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
    m_nIoBlockSize      = 0;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eVideoFileType_def::eFileType_unknown;
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


void CVideoFileIO::setIoBlockSize(int numFrames)
{
    m_nIoBlockSize = numFrames;
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


void CRawVideoFileIO::createInfoTextFile(const bool value)
{
    m_bCreateInfoTextFile = value;
}


bool CRawVideoFileIO::parseInfoTextFile(const std::string &sFile, SRawFileInfo &info)
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

        std::string sSearchText = "NumberOfChannels:";

        auto        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            int val = getNumericStringAt(sInputText, (unsigned int) sSearchText.length());

            if (val < 0)
                break;

            bOut             = true;

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

        sSearchText = "FrameSize:";

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
            else
            {
                LogWarning("parseInfoFile(): WARNING: invalid 'frameSize' ({}) in 'raw' videp info file: {}", sTemp, sFile);
                bOut = false;
            }

            continue;
        }
    }

    infoFile.close();

    return bOut;
}


bool CRawVideoFileIO::openFile(const eFileIoMode_def mode, const std::string &sFilePath)
{
    LogTrace("file path={}", sFilePath);

    if (m_bFileOpened)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getVideoFileType(m_sFilePath);

    if (m_nIoBlockSize < 1)
    {
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
    }
    else
    {
        /// Allocate 'm_nIoBlockSize' (number of) frames 
        switch (m_bitsPerPixel)
        {
        case 8:
        case 16:
        case 24:
        case 32:
            m_pFramebuffer = 
                calloc((m_bitsPerPixel / 8), (m_width * m_height * m_nIoBlockSize));
            break;

        default:
            return false;
        }
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
                if (m_eFileType == eVideoFileType_def::eFileType_raw && m_bCreateInfoTextFile)
                {
                    /// Create out videp info text file
                    std::string sInfoFilePath = getFileDir(m_sFilePath);    /// get the directory the file is in

                    std::string sFileName = getFileName(m_sFilePath);       /// get the filename with no ext (.xxx)

                    if (sInfoFilePath.empty() == false)
                        sInfoFilePath.append("/" + sFileName);
                    else
                        sInfoFilePath.assign(sFileName);

                    sInfoFilePath.append("-FileInfo.txt");                  /// append "-FileInfo.txt" to the filename

                    CFileIO fileInfo;

                    if (fileInfo.openFile(eFileIoMode_output, sInfoFilePath))
                    {
                        /// create a text buffer with into about the "raw" file we just created
                        std::string infoText;

                        std::string sTmp = "";

                        switch (m_bitsPerPixel)
                        {
                            case 8:
                                sTmp = "int8_t";
                                break;

                            case 16:
                                sTmp = "int16_t";
                                break;

                            case 24:
                                sTmp = "int24_t";
                                break;

                            case 32:
                                sTmp = "int32_t";
                                break;

                            default:
                                sTmp = "unknown";
                                break;
                        }

                        infoText.append("FrameSize: " + sTmp + " \n");

                        if (m_frameRate != 0)
                            infoText.append("FrameRate: " + std::to_string(m_frameRate) + " \n");

                        /// write "raw" file into to new text file
                        if (!fileInfo.writeBlock(infoText.c_str(), (unsigned int) infoText.size(), 1))
                            LogDebug("write failed to videp output text info file failed, path={}", sFilePath);

                        fileInfo.closeFile();
                    }
                    else
                    {
                        LogDebug("failed to create videp output text info file, path={}", sFilePath);
                    }
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
bool CRawVideoFileIO::readFrame(void *pData)
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


bool CRawVideoFileIO::readBlock(void *pData, const unsigned int numFrames)
{
    LogTrace("numFrames={}", numFrames);

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


bool CRawVideoFileIO::writeFrame(const void *pData)
{
    if (m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - eMode={}", 
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


bool CRawVideoFileIO::writeBlock(const void *pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - bFileOpened={}, eMode={}", 
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

#if 0

CAviFileIO::CAviFileIO() :
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


CAviFileIO::CAviFileIO(const unsigned int width, const unsigned int height, const unsigned int bitsPerPixel) :
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


CAviFileIO::CAviFileIO(const std::string& sFilePath) :
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


CAviFileIO::~CAviFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();
}


bool CAviFileIO::openFile(const eFileIoMode_def mode, const std::string& sFilePath)
{
    LogTrace("file path={}", sFilePath);

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


bool CAviFileIO::isEOF()
{
    if (m_nCurrentFrameIdx >= m_nFramesInFile)
    {
        return true;
    }

    return false;
}



/// Read a frame from a "AVI" data file
bool CAviFileIO::readFrame(void* pData)
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


bool CAviFileIO::readBlock(void* pData, const unsigned int numFrames)
{
    LogTrace("numFrames={}", numFrames);

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


bool CAviFileIO::writeFrame(const void* pData)
{
    if (m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - eMode={}",
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


bool CAviFileIO::writeBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr)
    {
        LogDebug
        (
            "bad param - bFileOpened={}, eMode={}",
            m_bFileOpened,
            (int)m_eMode
        );
        return false;
    }

    bool status = m_fileIO.writeBlock(pData, m_nFrameSize, numFrames);

    auto pos = m_fileIO.getLastIoSize();

    return status;
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


/// COcvFileIO class functions

COcvFileIO::COcvFileIO() :
    CVideoFileIO()
{
    LogTrace("class created");

    m_pFileInput = nullptr;
    m_pFileOutput = nullptr;

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
}


bool COcvFileIO::openFile(const eFileIoMode_def mode, const std::string& sFilePath)
{
    LogTrace("file path={}", sFilePath);

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
        }
        break;

    case eFileIoMode_def::eFileIoMode_output:
        {
            int nFourCC = String2FourCC(m_sFourCC);
            cv::Size frameSize;
            frameSize.width = m_width;
            frameSize.height = m_height;
            m_pFileOutput = new cv::VideoWriter(sFilePath, (int) nFourCC, (double) m_frameRate, frameSize);
            if (m_pFileOutput == nullptr)
                return false;
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


/// Read a frame from file
bool COcvFileIO::readFrame(void* pData)
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

    try
    {
        cv::Mat     frame;

        m_pFileInput->read(frame);

        auto pFrameData = frame.ptr();

        memcpy(pData, pFrameData, m_nFrameSize);

        m_nCurrentFrame++;

        m_lastFrameRead++;
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool COcvFileIO::readBlock(void* pData, const unsigned int numFrames)
{
    LogTrace("numFrames={}", numFrames);

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


bool COcvFileIO::writeFrame(const void* pData)
{
    if (m_eMode == eFileIoMode_input || m_eMode == eFileIoMode_IO)
    {
        LogDebug
        (
            "bad param - eMode={}",
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

    try
    {
        cv::Mat     frame;

        frame.resize(m_nFrameSize);

        auto pFrameData = frame.ptr();

        memcpy(pFrameData, pData, m_nFrameSize);

        m_pFileOutput->write(frame);

        m_nCurrentFrame++;

        m_lastFrameWritten++;
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool COcvFileIO::writeBlock(const void* pData, const unsigned int numFrames)
{
    if (pData == nullptr || numFrames < 1 || m_eMode == eFileIoMode_input || m_eMode == eFileIoMode_IO)
    {
        LogDebug
        (
            "bad param - bFileOpened={}, eMode={}",
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


