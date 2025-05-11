/// 
/// \file       CAudioFileIO.cpp
/// 
///             CAudioFileIO function definitions
///


#include "../Logging/Logging.h"

#include "CAudioFileIO.h"

#include <map>

#include <filesystem>
#include <optional>

#include "../String/StrUtils.h"


#ifdef  USE_DR_WAV

/// This needs to be defined in only one place 
///  to include the implementation code for dr_wav.
#define DR_WAV_IMPLEMENTATION

#include <dr_wav.h>

#endif


// Utility functions defs

std::string removeLeadingSpaces(std::string &sStr);

std::string removeTrailingSpaces(std::string& sStr);

std::string getFileDir(std::string& sPath);

std::string getFileExt(std::string &sPath);

std::string getFileName(std::string& sPath);


/// class CRawFileIO;
/// class CWavFileIO;


std::string audioFileTypeToString(eAudioFileType_def value)
{
    static std::map<eAudioFileType_def, std::string> map = 
    {
        {eFileType_raw, "raw"},   
        {eFileType_wav, "wav"},   
        {eFileType_aiff, "aiff"},
        {eFileType_info, "info"}, 
        {eFileType_text, "text"},
    };

    std::string name = "unknown value " + std::to_string(static_cast<int>(value));
    
    auto        it   = map.find(value);
    if (it != map.end())
        name = it->second;
    
    return name;
}


eAudioFileType_def getAudioFileType(const std::string &filepath)
{
    if (filepath.empty())
        return eFileType_unknown;

    std::string ext = std::filesystem::path(filepath).extension().string();

    //ext             = strToLower(ext);

    if (ext.empty() || ext == ".raw")
        return eFileType_raw;

    if (ext == ".wav")
        return eFileType_wav;

    if (ext == ".aiff")
        return eFileType_aiff;

    if (ext == ".info")
        return eFileType_info;

    if (ext == ".txt" || ext == ".text")
        return eFileType_text;

    return eFileType_unknown;
}


std::shared_ptr<CAudioFileIO> CAudioFileIO::openFileTypeByExt
    (
        const std::string &sFilePath, 
        const eFileIoMode_def mode, 
        int numChannels,
        int frameRate, 
        int blockSize,
        int bitsPerSasmple
    )
{
    if (sFilePath.empty())
    {
        LogError("file path not set");
        return nullptr;
    }

    if (numChannels < 1 && mode == eFileIoMode_def::eFileIoMode_output)
    {
        LogError("number of channels MUST be > 0 for output files");
        return nullptr;
    }

    std::shared_ptr<CAudioFileIO> pAFIO = nullptr;

    auto eFileType = getAudioFileType(sFilePath);

    if (eFileType == eFileType_raw)
    {
        if (numChannels < 1)
        {
            LogError("number of channels MUST be > 0 for raw audio files");
            return pAFIO;
        }

        std::shared_ptr<CRawFileIO> pRawFileIO = 
            std::make_shared<CRawFileIO>(numChannels);

        if (frameRate > 0)
            pRawFileIO->setSampleRate(frameRate);

        if (blockSize > 0)
            pRawFileIO->setIoBlockSize(blockSize);

        pRawFileIO->setSampleSize(bitsPerSasmple);        /// this also sets the "frameSize"

        if (mode == eFileIoMode_def::eFileIoMode_output)
            pRawFileIO->createInfoTextFile(true);

        if (!pRawFileIO->openFile(mode, sFilePath))
        {
            LogError("unable to open file={}", sFilePath);
            return pAFIO;
        }

        if (pRawFileIO->getNumChannels() != (int)numChannels)
        {
            LogError("file does not have correct number of channels");
            return pAFIO;
        }

        pAFIO = pRawFileIO;
    }
    else if (eFileType == eFileType_wav || eFileType == eFileType_aiff)
    {
        std::shared_ptr<CWavFileIO> pWavFileIO = 
            std::make_shared<CWavFileIO>(numChannels);

        if (frameRate > 0)
            pWavFileIO->setSampleRate(frameRate);

        if (blockSize > 0)
            pWavFileIO->setIoBlockSize(blockSize);

        pWavFileIO->setSampleSize(bitsPerSasmple);            /// this also sets the "frameSize"

        if (!pWavFileIO->openFile(mode, sFilePath))
        {
            LogError("unable to open file={}", sFilePath);
            return pAFIO;
        }

        if (mode == eFileIoMode_def::eFileIoMode_output)
        {
            if (pWavFileIO->getNumChannels() != (int)numChannels)
            {
                LogError("file does not have correct number of channels");
                return pAFIO;
            }
        }

        pAFIO = pWavFileIO;
    }

    return pAFIO;
}


CAudioFileIO::CAudioFileIO() 
{
    m_numChannels       = 0;
    m_nFrameSize        = 0;
    m_sampleRate        = 0;
    m_sFilePath         = "";
    m_bFileOpened       = false;
    m_eMode             = eFileIoMode_unknown;
    m_nIoBlockSize      = 0;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eAudioFileType_def::eFileType_unknown;
    m_nBitsPerSample    = 16;
}


CAudioFileIO::CAudioFileIO(unsigned int numChannels) :
    m_numChannels(numChannels)
{
    m_nFrameSize        = (numChannels * sizeof(int16_t));
    m_sampleRate        = 0;
    m_sFilePath         = "";
    m_bFileOpened       = false;
    m_eMode             = eFileIoMode_unknown;
    m_nIoBlockSize      = 0;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eAudioFileType_def::eFileType_unknown;
    m_nBitsPerSample    = 16;
}


CAudioFileIO::CAudioFileIO(const unsigned int numChannels, const std::string &sFilePath) :
    m_numChannels(numChannels)
{
    m_nFrameSize        = (numChannels * sizeof(int16_t));
    m_sampleRate        = 0;
    m_sFilePath         = sFilePath;
    m_bFileOpened       = false;
    m_eMode             = eFileIoMode_unknown;
    m_nIoBlockSize      = 0;
    m_nFramesInFile     = 0;
    m_nCurrentFrameIdx  = 0;
    m_nIoCntr           = -1;
    m_nCurrentFrame     = -1;
    m_bUseLoopingRead   = false;
    m_eFileType         = eAudioFileType_def::eFileType_unknown;
    m_nBitsPerSample    = 16;
}


CAudioFileIO::~CAudioFileIO()
{
    m_bFileOpened = false;
}


void CAudioFileIO::setFile(std::string &sFilePath)
{
    m_sFilePath = sFilePath;

    m_eFileType = getAudioFileType(m_sFilePath);
}


void CAudioFileIO::setLoopingRead(bool value)
{
    m_bUseLoopingRead = value;
}


void CAudioFileIO::setNumChannels(const unsigned int numChls)
{
    m_numChannels = numChls;
    m_nFrameSize = (numChls * (m_nBitsPerSample / 8));
}


void CAudioFileIO::setSampleRate(const unsigned int rate)
{
    m_sampleRate = rate;
}


bool CAudioFileIO::isFileOpened() const
{
    return m_bFileOpened;
}


std::string CAudioFileIO::getFilePath()
{
    return m_sFilePath;
}


eAudioFileType_def CAudioFileIO::getFileType()
{
    return m_eFileType;
}


int CAudioFileIO::getNumChannels()
{
    return (int)m_numChannels;
}


long CAudioFileIO::getNumFramesInFile()
{
    return m_nFramesInFile;
}


void CAudioFileIO::setIoBlockSize(int numFrames)
{
    m_nIoBlockSize = numFrames;
}


void CAudioFileIO::setSampleSize(int numBits)
{
    if (numBits == 16 || numBits == 32 || numBits == 8)
        m_nBitsPerSample = numBits;

    if (m_nBitsPerSample == 32)
        m_nFrameSize  = (m_numChannels * sizeof(int32_t));
    else if (m_nBitsPerSample == 8)
        m_nFrameSize  = (m_numChannels * sizeof(int8_t));
    else
        m_nFrameSize  = (m_numChannels * sizeof(int16_t));
}


unsigned int CAudioFileIO::getIoCount() const
{
    return (unsigned int)m_nIoCntr;
}


int CAudioFileIO::getCurrentFrame() const
{
    return (int)m_nCurrentFrame;
}


int CAudioFileIO::getCurrentFrameIndex() const
{
    return (int)m_nCurrentFrameIdx;
}


int CRawFileIO::getNumericStringAt(const std::string &sText, const unsigned int pos)
{
    auto len = sText.length();

    if (pos >= len)
        return -1;

    std::string sTemp = sText.substr(pos);

    sTemp             = removeLeadingSpaces(sTemp);

    sTemp             = removeTrailingSpaces(sTemp);

    if (!StrUtils::isNumeric(sTemp))
        return -1;

    return std::stoi(sTemp);
}


CRawFileIO::CRawFileIO(const unsigned int numChannels) :
    CAudioFileIO(numChannels)
{
    LogTrace("class created");

    m_eMode               = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer        = nullptr;
    m_nFrameSize          = (numChannels * sizeof(int16_t));
    m_nCurrentFrameIdx    = 0;
    m_lFileSize           = 0;
    m_lCurrentFilePos     = 0;
    m_lastChlRead         = -1;
    m_lastChlWritten      = -1;
    m_bCreateInfoTextFile = false;
}


CRawFileIO::CRawFileIO(const unsigned int numChannels, const std::string &sFilePath) :
    CAudioFileIO(numChannels, sFilePath)
{
    LogTrace("class created");

    m_fileIO.setFilePath(sFilePath);
    m_nFrameSize          = (numChannels * sizeof(int16_t));
    m_eMode               = eFileIoMode_def::eFileIoMode_unknown;
    m_pFramebuffer        = nullptr;
    m_nCurrentFrameIdx    = 0;
    m_lFileSize           = 0;
    m_lCurrentFilePos     = 0;
    m_lastChlRead         = -1;
    m_lastChlWritten      = -1;
    m_bCreateInfoTextFile = false;
}


CRawFileIO::~CRawFileIO()
{
    LogTrace("class being destroyed");

    if (m_bFileOpened)
        closeFile();
}


void CRawFileIO::createInfoTextFile(const bool value)
{
    m_bCreateInfoTextFile = value;
}


bool CRawFileIO::parseInfoTextFile(const std::string &sFile, SRawFileInfo &info)
{
    std::fstream infoFile;

    infoFile.open(sFile, std::ios::in);
    if (!infoFile.is_open())
    {
        LogError("parseInfoFile(): ERROR: unable to open 'raw' audio info file: {}", sFile);
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

            info.numChannels = val;

            bOut             = true;

            continue;
        }

        sSearchText = "SampleRate:";

        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            int val = getNumericStringAt(sInputText, (unsigned int) sSearchText.length());

            if (val < 0)
                break;

            info.sampleRate = val;

            bOut            = true;

            continue;
        }

        sSearchText = "SampleSize:";

        pos         = sInputText.find(sSearchText);
        if (pos != std::string::npos)
        {
            std::string sTemp = sInputText.substr(pos + sSearchText.length());

            sTemp             = removeLeadingSpaces(sTemp);

            bOut              = true;

            if (sTemp == "int8_t")
            {
                info.bitsPerSample = 8;    
            }
            else if (sTemp == "int16_t")
            {
                info.bitsPerSample = 16;    
            }
            else if (sTemp == "int32_t")
            {
                info.bitsPerSample = 32;    
            }
            else
            {
                LogWarning("parseInfoFile(): WARNING: invalid 'sampleSize' ({}) in 'raw' audio info file: {}", sTemp, sFile);
                bOut = false;
            }

            continue;
        }
    }

    infoFile.close();

    return bOut;
}


bool CRawFileIO::openFile(const eFileIoMode_def mode, const std::string &sFilePath)
{
    LogTrace("file path={}", sFilePath);

    if (m_bFileOpened || m_numChannels < 1)
        return false;

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
        return false;

    m_eFileType = getAudioFileType(m_sFilePath);

    if (m_nIoBlockSize < 1)
    {
        /// Allocate just 1 frame (size of 'm_numChannels')
        if (m_nBitsPerSample == 32)
            m_pFramebuffer = calloc(sizeof(int32_t), m_numChannels);
        else if (m_nBitsPerSample == 8)
            m_pFramebuffer = calloc(sizeof(int8_t), m_numChannels);
        else
            m_pFramebuffer = calloc(sizeof(int16_t), m_numChannels);
    }
    else
    {
        /// Allocate 'm_nIoBlockSize' (number of) frames (* size of 'm_numChannels')
        if (m_nBitsPerSample == 32)
            m_pFramebuffer = calloc(sizeof(int32_t), (m_numChannels * m_nIoBlockSize));
        else if (m_nBitsPerSample == 8)
            m_pFramebuffer = calloc(sizeof(int8_t), (m_numChannels * m_nIoBlockSize));
        else
            m_pFramebuffer = calloc(sizeof(int16_t), (m_numChannels * m_nIoBlockSize));
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
                m_lastChlRead     = -1;

                nextFrame();
            }
            break;

        case eFileIoMode_output:
            {
                if (!m_fileIO.openFile(eFileIoMode_output, m_sFilePath))
                    return false;

                m_lFileSize       = 0;

                /// Set the write position to the end of the file.
                m_lCurrentFilePos = m_lFileSize;
                m_lastChlWritten  = -1;

                /// If "CreateInfoTextFile" option selected..
                if (m_eFileType == eAudioFileType_def::eFileType_raw && m_bCreateInfoTextFile)
                {
                    /// Create out audio info text file
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

                        switch (m_nBitsPerSample)
                        {
                            case 8:
                                sTmp = "int8_t";
                                break;

                            case 32:
                                sTmp = "int32_t";
                                break;

                            default:
                                sTmp = "int16_t";
                                break;
                        }

                        infoText.append("SampleSize: " + sTmp + " \n");

                        infoText.append("NumberOfChannels: " + std::to_string(m_numChannels) + " \n");

                        if (m_sampleRate != 0)
                            infoText.append("SampleRate: " + std::to_string(m_sampleRate) + " \n");

                        /// write "raw" file into to new text file
                        if (!fileInfo.writeBlock(infoText.c_str(), (unsigned int) infoText.size(), 1))
                            LogError("write failed to audio output text info file failed, path={}", sFilePath);

                        fileInfo.closeFile();
                    }
                    else
                    {
                        LogError("failed to create audio output text info file, path={}", sFilePath);
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


long CRawFileIO::getNumFrames()
{
    LogTrace("getting number of frame in the file");

    if (!m_bFileOpened || m_eMode == eFileIoMode_unknown || m_eMode == eFileIoMode_output || m_numChannels < 1)
        return -1;

    /// Get the file length (which sets file pointer to EOF)
    long fileSize  = m_fileIO.getFileSize();
    
    /// Computer number of frames in the file
    int  numFrames = (int)((float) fileSize / (float)m_nFrameSize);

    /// Reset file pointer to the beginning of the file
    m_fileIO.setFilePosition(0);

    return numFrames;
}


bool CRawFileIO::closeFile()
{
    LogTrace("file being closed");

    if (!m_bFileOpened)
        LogDebug("file close called when files is not open");

    /// close input file
    if (m_fileIO.isOpen())
    {
        if (!m_fileIO.closeFile())
        {
            LogCritical("CRawFileIO - Error: unable to close file ");
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


/// Read a sample from a "raw" data file
bool CRawFileIO::readSample(int16_t &data, const unsigned int chl)
{
    LogTrace("channel={}", chl);

    if (!m_bFileOpened || m_pFramebuffer == nullptr || chl >= m_numChannels || m_nBitsPerSample != 16)
        return false;

    /// This "read" logic reads 1 "frame" (int16_t sample * m_numChannels)
    /// at a time from the input file, and stores it in the
    /// FrameBuffer.	 It then gets sample values from the buffer.

    if (m_nCurrentFrameIdx < 1)
    {
        data = *(((int16_t *) m_pFramebuffer) + chl);
    }
    else
    {
        data = *(((int16_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl);
    }

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }

    return true;
}


bool CRawFileIO::readSample(int32_t &data, const unsigned int chl)
{
    LogTrace("channel={}", chl);

    if (!m_bFileOpened || m_pFramebuffer == nullptr || chl >= m_numChannels || m_nBitsPerSample != 32)
        return false;

    /// This "read" logic reads 1 "frame" (int16_t sample * m_numChannels)
    /// at a time from the input file, and stores it in the
    /// FrameBuffer.	 It then gets sample values from the buffer.

    if (m_nCurrentFrameIdx < 1)
    {
        data = *(((int32_t *) m_pFramebuffer) + chl);
    }
    else
    {
        data = *(((int32_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl);
    }

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }

    return true;
}


bool CRawFileIO::readBlock(void *pData, const unsigned int numFrames)
{
    LogTrace("numFrames={}", numFrames);

    if (!m_bFileOpened || pData == nullptr || numFrames < 1)
        return false;

    if (m_lCurrentFilePos + (m_nFrameSize * numFrames) > m_lFileSize)
    {
        /// If our next read would exceed the file size...
        /// If the "UseLoopingRead" flag = false
        /// don't try to read anymore.
        if (!m_bUseLoopingRead)
            return false;

        /// If the "UseLoopingRead" flag = true
        /// seek back to the beginning of the file

        if (!m_fileIO.setFilePosition(0))
        {
            m_fileIO.closeFile();
            return false;
        }

        m_lCurrentFilePos = 0;
    }

    /// Read 1 frame of samples from the input file.

    bool status = m_fileIO.readBlock(pData, m_nFrameSize, numFrames);

    /// Update the file read position
#ifdef UPDATE_FILE_POSITION
    m_lCurrentFilePos = m_fileIO.getFilePosition();
#else
    auto pos = m_fileIO.getLastIoSize();

    if (pos > 0)
        m_lCurrentFilePos += pos;
#endif

    if (status)
        m_nCurrentFrame += numFrames;

    return status;
}


bool CRawFileIO::writeSample(const int16_t data, const unsigned int chl)
{
    if (chl >= m_numChannels || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr || m_nBitsPerSample != 16)
    {
        LogDebug
        (
            "bad param - chl={}, numChannels={}, eMode={}", 
            chl, 
            m_numChannels,
            (int) m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" 
    /// (int16_t sample * m_numChannels)
    /// at a time to the output file.

    /// First set the sample value for each channel.

    if (m_nCurrentFrameIdx < 1)
    {
        *(((int16_t *) m_pFramebuffer) + chl) = data;
    }
    else
    {
        *(((int16_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl) = data;
    }

    if ((int)chl != m_lastChlWritten)
    {
        /// This logic makes sure the same channel number
        /// isn't being written over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlWritten = (int)chl;
    }

    /// After the samples values for all channels have been set
    /// call 'nextFrame' to write the frame data to the file.

    return true;
}


bool CRawFileIO::writeSample(const int32_t data, const unsigned int chl)
{
    if (chl >= m_numChannels || m_eMode == eFileIoMode_input || m_pFramebuffer == nullptr || m_nBitsPerSample != 32)
    {
        LogDebug
        (
            "bad param - chl={}, numChannels={}, eMode={}", 
            chl, 
            m_numChannels,
            (int) m_eMode
        );
        return false;
    }

    /// This "write" logic writes 1 "frame" (int16_t sample * m_numChannels)
    /// at a time to the output file.

    /// First set the sample value for each channel.

    if (m_nCurrentFrameIdx < 1)
    {
        *(((int32_t *) m_pFramebuffer) + chl) = data;
    }
    else
    {
        *(((int32_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl) = data;
    }

    if ((int)chl != m_lastChlWritten)
    {
        /// This logic makes sure the same channel number
        /// isn't being written over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlWritten = (int)chl;
    }

    /// After the samples values for all channels have been set
    /// call 'nextFrame' to write the frame data to the file.

    return true;
}


bool CRawFileIO::writeBlock(const void *pData, const unsigned int numFrames)
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


/// Move to next input/output frame
bool CRawFileIO::nextFrame()
{
    if (m_pFramebuffer == nullptr)
        return false;

    if (m_nIoBlockSize < 1)
    {
        m_nCurrentFrameIdx = 0;

        switch (m_eMode)
        {
            case eFileIoMode_input:
                {
                    auto status = readBlock(m_pFramebuffer, 1);
                    if (!status)
                        return false;

                    m_lastChlRead = -1;
                }
                break;

            case eFileIoMode_output:
                {
                    auto status = writeBlock(m_pFramebuffer, 1);
                    if (!status)
                        return false;

                    m_lastChlWritten = -1;
                }
                break;

            default:
                return false;
        }
    }
    else
    {
        m_nCurrentFrameIdx++;

        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
        {
            switch (m_eMode)
            {
                case eFileIoMode_input:
                    {
                        auto status = readBlock(m_pFramebuffer, m_nIoBlockSize);
                        if (!status)
                            return false;

                        m_lastChlRead = -1;
                    }
                    break;

                case eFileIoMode_output:
                    {
                        auto status = writeBlock(m_pFramebuffer, m_nIoBlockSize);
                        if (!status)
                            return false;

                        m_lastChlWritten = -1;
                    }
                    break;

                default:
                    return false;
            }

            m_nCurrentFrameIdx = 0;
        }
    }

    return true;
}


bool CRawFileIO::setCurrentFrame(unsigned int frameNum) 
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


CWavFileIO::CWavFileIO(const unsigned int numChannels) :
    CAudioFileIO(numChannels)
{
    LogTrace("class created");

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;

#ifdef USE_DR_WAV
    m_pFramebuffer = nullptr;
#endif
    m_nCurrentFrameIdx       = 0;
    m_currChannel            = 0;
    m_lastChlRead            = -1;
    m_lastChlWritten         = -1;
    m_bWriteFileForEachFrame = false;
}


CWavFileIO::CWavFileIO(const unsigned int numChannels, const std::string &sFilePath) :
    CAudioFileIO(numChannels, sFilePath)
{
    LogTrace("class created");

    m_eMode = eFileIoMode_def::eFileIoMode_unknown;

#ifdef USE_DR_WAV
    m_pFramebuffer           = nullptr;
#else
    m_blockSize              = 0;
#endif
    m_nCurrentFrameIdx       = 0;
    m_currChannel            = 0;
    m_lastChlRead            = -1;
    m_lastChlWritten         = -1;
    m_bWriteFileForEachFrame = false;
}


CWavFileIO::~CWavFileIO()
{
    if (m_bFileOpened)
        closeFile();

#ifdef USE_DR_WAV
    drwav_uninit(&m_audioFile);
#endif
}


bool CWavFileIO::openFile(const eFileIoMode_def mode, const std::string &sFilePath)
{
    LogTrace("file path={}", sFilePath);

    if (m_pFramebuffer != nullptr)
    {
        /// LogWarning("CWavFileIO::openFile() - 
        /// openFile called ({}) while a file is already open ", sFilePath);
        return false;
    }

    if (!sFilePath.empty())
        m_sFilePath = sFilePath;

    if (m_sFilePath.empty())
    {
        LogError("file path is empty");
        return false;
    }

    m_eFileType = getAudioFileType(m_sFilePath);

    m_eMode     = mode;
    switch (m_eMode)
    {
        case eFileIoMode_input:
            {
#ifndef USE_DR_WAV
                LogTrace("loading WAV file for input");

                auto status = m_audioFile.load(m_sFilePath);
                if (!status)
                {
                    LogError("audiofile load failed");
                    return false;
                }

                if (m_audioFile.getNumSamplesPerChannel() < 1)
                {
                    LogError("audiofile does not contain AT LEAST 1 frame");
                    return false;
                }

                if (m_numChannels != (unsigned int)m_audioFile.getNumChannels())
                {
                    LogError("bad audiofile num Channels");
                    return false;
                }

                if (m_sampleRate != 0)
                {
                    if (m_sampleRate != (unsigned int)m_audioFile.getSampleRate())
                    {
                        LogError("bad audiofile sample rate");
                        return false;
                    }
                }
#else
                LogTrace("loading WAV file for input");
                if (!drwav_init_file(&m_audioFile, m_sFilePath.c_str(), nullptr))
                {
                    LogError("problem during dr_wav input file init, file={}", m_sFilePath);
                    m_bFileOpened = false;
                    return false;
                }

                m_numChannels = (unsigned int) m_audioFile.channels;
                m_sampleRate  = (int) m_audioFile.sampleRate;

                if (m_nIoBlockSize < 1)
                {
                    /// Allocate just 1 frame (size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), m_numChannels);
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), m_numChannels);
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), m_numChannels);
                }
                else
                {
                    /// Allocate 'm_nIoBlockSize' (number of) frames (* size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), (m_numChannels * m_nIoBlockSize));
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), (m_numChannels * m_nIoBlockSize));
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), (m_numChannels * m_nIoBlockSize));
                }

                if (m_pFramebuffer == nullptr)
                {
                    LogError("invalid frame buffer pointer");
                    return false;
                }

                drwav_uint64 numFrames = 0;
                auto         status    = drwav_get_length_in_pcm_frames(&m_audioFile, &numFrames);

                if (status == DRWAV_SUCCESS)
                    m_nFramesInFile = (long)numFrames;

                nextFrame();
#endif
            }
            break;

        case eFileIoMode_output:
            {
#ifndef USE_DR_WAV
                LogTrace("opening WAV file for output");

                m_audioFile.setNumChannels(m_numChannels);
                if (m_sampleRate != 0)
                    m_audioFile.setSampleRate(m_sampleRate);

#else
                LogTrace("opening WAV file for output");

                //drwav_fmt             wavFmt;

                drwav_data_format       dataFormat;

                dataFormat.container     = drwav_container_riff; /// <-- drwav_container_riff = normal WAV files,
                                                                 /// drwav_container_w64 = Sony Wave64.
                dataFormat.format        = DR_WAVE_FORMAT_PCM;   /// <-- Any of the DR_WAVE_FORMAT_* codes.
                dataFormat.channels      = m_numChannels;
                dataFormat.sampleRate    = m_sampleRate;
                dataFormat.bitsPerSample = m_nBitsPerSample;

                if (!drwav_init_file_write(&m_audioFile, m_sFilePath.c_str(), &dataFormat, nullptr))
                {
                    LogError("problem during dr_wav output file init, file={}", m_sFilePath);
                    m_bFileOpened = false;
                    return false;
                }

                if (m_nIoBlockSize < 1)
                {
                    /// Allocate just 1 frame (size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), m_numChannels);
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), m_numChannels);
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), m_numChannels);
                }
                else
                {
                    /// Allocate 'm_nIoBlockSize' (number of) frames (* size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), (m_numChannels * m_nIoBlockSize));
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), (m_numChannels * m_nIoBlockSize));
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), (m_numChannels * m_nIoBlockSize));
                }

                if (m_pFramebuffer == nullptr)
                {
                    LogError("invalid frame buffer pointer");
                    return false;
                }

                m_nFramesInFile = 0;
#endif
            }
            break;

        case eFileIoMode_IO:
            {
#ifndef USE_DR_WAV
                LogTrace("opening WAV file");

                auto status = m_audioFile.load(m_sFilePath);
                if (!status)
                {
                    m_audioFile.setNumChannels(m_numChannels);
                    if (m_sampleRate != 0)
                        m_audioFile.setSampleRate(m_sampleRate);
                }
#else
                LogTrace("opening WAV file");

                if (!drwav_init_file(&m_audioFile, m_sFilePath.c_str(), nullptr))
                {
                    LogError("problem during dr_wav file I/O init, file={}", m_sFilePath);
                    m_bFileOpened = false;
                    return false;
                }

                if (m_nIoBlockSize < 1)
                {
                    /// Allocate just 1 frame (size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), m_numChannels);
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), m_numChannels);
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), m_numChannels);
                }
                else
                {
                    /// Allocate 'm_nIoBlockSize' (number of) frames (* size of 'm_numChannels')
                    if (m_nBitsPerSample == 32)
                        m_pFramebuffer = calloc(sizeof(int32_t), (m_numChannels * m_nIoBlockSize));
                    else if (m_nBitsPerSample == 8)
                        m_pFramebuffer = calloc(sizeof(int8_t), (m_numChannels * m_nIoBlockSize));
                    else
                        m_pFramebuffer = calloc(sizeof(int16_t), (m_numChannels * m_nIoBlockSize));
                }

                if (m_pFramebuffer == nullptr)
                    return false;

                drwav_uint64 numFrames = 0;

                auto         status    = drwav_get_length_in_pcm_frames(&m_audioFile, &numFrames);

                if (status == DRWAV_SUCCESS)
                    m_nFramesInFile = (long)numFrames;

                nextFrame();
#endif //	USE_DR_WAV
            }
            break;

        default:
            return false;
    }

    m_nCurrentFrameIdx = 0;
    m_nIoCntr          = 0;
    m_nCurrentFrame    = 0;
    m_currChannel      = 0;
    m_lastChlRead      = -1;
    m_lastChlWritten   = -1;
    m_bFileOpened      = true;

    return true;
}


/**
@note For "wav" and "aiff" files.
The data is ONLY written to the disk file when the file is closed unless m_bWriteFileForEachFrame = true
*/
bool CWavFileIO::closeFile()
{
    LogTrace("file being closed");

    if (m_bFileOpened)
    {
#ifndef USE_DR_WAV
        if (m_eMode == eFileIoMode_output || m_eMode == eFileIoMode_IO)
        {
            if (m_eFileType == eFileType_wav)
                m_audioFile.save(m_sFilePath, AudioFileFormat::Wave);
            else
                m_audioFile.save(m_sFilePath, AudioFileFormat::Aiff);
        }
#else
        try
        {
            if (m_pFramebuffer != nullptr)
            {
                auto pTmp      = m_pFramebuffer;

                m_pFramebuffer = nullptr;
                free(pTmp);
            }
        }
        catch (...)
        {
            ;
        }
#endif
    }

    m_nIoCntr       = -1;
    m_nCurrentFrame = -1;
    m_eMode         = eFileIoMode_def::eFileIoMode_unknown;
    m_bFileOpened   = false;

    return true;
}


void CWavFileIO::setFrameOutputWriteFlag(const bool value)
{
    m_bWriteFileForEachFrame = value;
}


void CWavFileIO::setSampleRate(const unsigned int rate)
{
    m_sampleRate = rate;

#ifndef USE_DR_WAV
    m_audioFile.setSampleRate(m_sampleRate);
#endif
}


#ifndef USE_DR_WAV
void CWavFileIO::setBlockSize(const unsigned int blockSize)
{
    m_blockSize = blockSize;

    m_audioFile.setAudioBufferSize(m_numChannels, m_blockSize);
}
#endif


int CWavFileIO::getSampleRate()
{
#ifndef USE_DR_WAV
    m_sampleRate = (unsigned int)m_audioFile.getSampleRate();
#else
    m_sampleRate  = (unsigned int)m_audioFile.sampleRate;
#endif

    return (int) m_sampleRate;
}


int CWavFileIO::getNumChannels()
{
#ifndef USE_DR_WAV
    m_numChannels = (unsigned int)m_audioFile.getNumChannels();
#else
    m_numChannels = (unsigned int)m_audioFile.channels;
#endif

    return (int)m_numChannels;
}


int CWavFileIO::getNumFrames() const
{
#ifndef USE_DR_WAV
    return (int)m_audioFile.getNumSamplesPerChannel();
#else
    return (int)m_audioFile.totalPCMFrameCount;
#endif
}


bool CWavFileIO::readSample(int16_t &data, const unsigned int chl)
{
    LogTrace("channel={}", chl);

    if (chl >= m_numChannels || m_eMode == eFileIoMode_output || m_nBitsPerSample != 16)
        return false;

#ifndef USE_DR_WAV
    auto totalNumFrames = m_audioFile.getNumSamplesPerChannel();
#else
    auto totalNumFrames = m_audioFile.totalPCMFrameCount;
#endif
    if (m_nCurrentFrame > (int)totalNumFrames)
    {
        if (!m_bUseLoopingRead)
            return false;

        m_nCurrentFrame = 0;
    }

#ifndef USE_DR_WAV
    float   fTmp = m_audioFile.samples[chl][m_nCurrentFrame];

    int16_t iTmp = ConvertFloatToInt16(fTmp);

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }
#else
    if (m_pFramebuffer == nullptr)
        return false;

    /// This "read" logic reads 1 "frame" (int16_t sample * m_numChannels)
    /// at a time from the input file, and stores it in the
    /// FrameBuffer.	 It then gets sample values from the buffer.

    if (m_nCurrentFrameIdx < 1)
    {
        data = *(((int16_t *) m_pFramebuffer) + chl);
    }
    else
    {
        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
            return false;

        data = *(((int16_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl);
    }

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }
#endif

    return true;
}


bool CWavFileIO::readSample(int32_t &data, const unsigned int chl)
{
    LogTrace("channel={}", chl);

    if (chl >= m_numChannels || m_eMode == eFileIoMode_output || m_nBitsPerSample != 32)
        return false;

#ifndef USE_DR_WAV
    auto totalNumFrames = m_audioFile.getNumSamplesPerChannel();
#else
    auto totalNumFrames = m_audioFile.totalPCMFrameCount;
#endif
    if (m_nCurrentFrame > (int)totalNumFrames)
    {
        if (!m_bUseLoopingRead)
            return false;

        m_nCurrentFrame = 0;
    }

#ifndef USE_DR_WAV
    float   fTmp = m_audioFile.samples[chl][m_nCurrentFrame];

    int16_t iTmp = ConvertFloatToInt16(fTmp);

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }
#else
    if (m_pFramebuffer == nullptr)
        return false;

    /// This "read" logic reads 1 "frame" (int16_t sample * m_numChannels)
    /// at a time from the input file, and stores it in the
    /// FrameBuffer.	 It then gets sample values from the buffer.

    if (m_nCurrentFrameIdx < 1)
    {
        data = *(((int32_t *) m_pFramebuffer) + chl);
    }
    else
    {
        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
            return false;

        data = *(((int32_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl);
    }

    if ((int)chl != m_lastChlRead)
    {
        /// This logic makes sure the same channel number
        /// isn't being read over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlRead = (int)chl;
    }
#endif

    return true;
}


bool CWavFileIO::readBlock(void *pData, const unsigned int numFrames)
{
    LogTrace("numFrames={}", numFrames);

    if (pData == nullptr || m_eMode == eFileIoMode_output)
    {
        LogDebug
        (
            "bad param - eMode={}", 
            (int) m_eMode
        );
        return false;
    }

#ifndef USE_DR_WAV
    auto totalNumFrames = m_audioFile.getNumSamplesPerChannel();
#else
    auto totalNumFrames = m_audioFile.totalPCMFrameCount;
#endif
    if ((m_nCurrentFrame + numFrames) > (unsigned int)totalNumFrames)
    {
        if (!m_bUseLoopingRead)
            return false;

        m_nCurrentFrame = 0;

#ifdef USE_DR_WAV
        drwav_bool32 status = drwav_seek_to_pcm_frame(&m_audioFile, (drwav_uint64)0);
        if (!status)
            LogWarning("drwav_seek_to_pcm_frame to frame 0 failed");
#endif
    }

#ifndef USE_DR_WAV

    unsigned int readOffset = 0;
    for (unsigned int i = 0; i < numFrames; i++)
    {
        for (unsigned int chl = 0; chl < m_numChannels; chl++)
        {
            int16_t iTmp  = 
                ConvertFloatToInt16(m_audioFile.samples[chl][m_nCurrentFrame + i]);
            
            *((int16_t *) pData + readOffset++) = iTmp;
        }
    }
#else
    drwav_uint64 framesRead = 0;
    //auto framesRead = drwav_read_pcm_frames(&m_audioFile, numFrames, pData);
    if (m_nBitsPerSample == 16)
    {
        framesRead = 
            drwav_read_pcm_frames_s16__pcm(&m_audioFile, numFrames, (drwav_int16 *) pData);
    }
    else if (m_nBitsPerSample == 32)
    {
        framesRead = 
            drwav_read_pcm_frames_s32__pcm(&m_audioFile, numFrames, (drwav_int32 *) pData);
    }
    else
    {
        LogWarning("invalid sample size");
    }

    if (framesRead != (drwav_uint64)numFrames)
        return false;
#endif

    m_nCurrentFrame += numFrames;

    return true;
}


bool CWavFileIO::writeSample(const int16_t data, const unsigned int chl)
{
    if (chl >= m_numChannels || m_eMode == eFileIoMode_input || m_nBitsPerSample != 16)
    {
        LogDebug
        (
            "bad param - chl={}, numChannels={}, eMode={}", 
            chl, 
            m_numChannels,
            (int) m_eMode
        );

        return false;
    }

#ifndef USE_DR_WAV
    float fTmp = ConvertInt16ToFloat(data);

    if ((int)m_audioFile.samples.size() <= (int)chl)
        m_audioFile.samples.resize(chl + 1);

    if ((int)m_audioFile.samples[chl].size() <= (int)m_nCurrentFrame)
        m_audioFile.samples[chl].resize(m_nCurrentFrame + 1);

    m_audioFile.samples[chl][m_nCurrentFrame] = fTmp;
#else
    if (m_pFramebuffer == nullptr)
        return false;

    /// This "write" logic writes 1 "frame" (int16_t sample * m_numChannels)
    /// at a time to the output file.

    if (m_nCurrentFrameIdx < 1)
    {
        *(((int16_t *) m_pFramebuffer) + chl) = data;
    }
    else
    {
        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
            return false;

        *(((int16_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl) = data;
    }
#endif

    if ((int)chl != m_lastChlWritten)
    {
        /// This logic makes sure the same channel number
        /// isn't being written over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlWritten = (int)chl;
    }

    return true;
}


bool CWavFileIO::writeSample(const int32_t data, const unsigned int chl)
{
    if (chl >= m_numChannels || m_eMode == eFileIoMode_input || m_nBitsPerSample != 32)
    {
        LogDebug
        (
            "bad param - chl={}, numChannels={}, eMode={}", 
            chl, 
            m_numChannels,
            (int) m_eMode
        );
        return false;
    }

#ifndef USE_DR_WAV
    float fTmp = ConvertInt16ToFloat(data);

    if ((int)m_audioFile.samples.size() <= (int)chl)
        m_audioFile.samples.resize(chl + 1);

    if ((int)m_audioFile.samples[chl].size() <= (int)m_nCurrentFrame)
        m_audioFile.samples[chl].resize(m_nCurrentFrame + 1);

    m_audioFile.samples[chl][m_nCurrentFrame] = fTmp;
#else
    if (m_pFramebuffer == nullptr)
        return false;

    /// This "write" logic writes 1 "frame" (int16_t sample * m_numChannels)
    /// at a time to the output file.

    if (m_nCurrentFrameIdx < 1)
    {
        *(((int32_t *) m_pFramebuffer) + chl) = data;
    }
    else
    {
        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
            return false;

        *(((int32_t *) m_pFramebuffer) + (m_nCurrentFrameIdx * m_numChannels) + chl) = data;
    }
#endif

    if ((int)chl != m_lastChlWritten)
    {
        /// This logic makes sure the same channel number
        /// isn't being written over and over.

        m_nIoCntr++; /// Increment the number of samples read.
        m_lastChlWritten = (int)chl;
    }

    return true;
}


bool CWavFileIO::writeBlock(const void *pData, const unsigned int numFrames)
{
    if (pData == nullptr)
    {
        LogDebug("invalid input data pointer");
        return false;
    }

    if (numFrames < 1)
    {
        LogDebug("invalid number of frames to write (numFrames <= 0)");
        return false;
    }

    if (m_numChannels < 1)
    {
        LogDebug("invalid number of channels (m_numChannels <= 0)");
        return false;
    }

    if (m_eMode == eFileIoMode_input)
        return false;

#ifndef USE_DR_WAV
    unsigned int writeOffset = 0;
    for (unsigned int i = 0; i < numFrames; i++)
    {
        for (unsigned int chl = 0; chl < m_numChannels; chl++)
        {
            if (((int16_t *) pData + writeOffset) == nullptr)
            {
                LogTrace("write with invalid data input pointer value (pData + {})", writeOffset);
                m_audioFile.samples[chl][m_nCurrentFrame + i] = 0;
            }
            else
            {
                int16_t iTmp = *((int16_t*) pData + writeOffset++);

                m_audioFile.samples[chl][m_nCurrentFrame + i] = ConvertInt16ToFloat(iTmp);
            }
        }
    }

    m_nFramesInFile += numFrames;
    if (m_bWriteFileForEachFrame || m_nCurrentFrameIdx >= m_nIoBlockSize)
    {
        if (m_eMode == eFileIoMode_output || m_eMode == eFileIoMode_IO)
        {
            if (m_eFileType == eFileType_wav)
                m_audioFile.save(m_sFilePath, AudioFileFormat::Wave);
            else
                m_audioFile.save(m_sFilePath, AudioFileFormat::Aiff);
        }

        m_nFramesInFile = 0;
    }
#else
    drwav_uint64 framesWritten = 0;
    
    framesWritten = drwav_write_pcm_frames(&m_audioFile, numFrames, pData);

    if (framesWritten != (drwav_uint64)numFrames)
        return false;
#endif
    m_nCurrentFrame += numFrames;
    m_nFramesInFile += numFrames;

    return true;
}


/// Move to next input/output frame
bool CWavFileIO::nextFrame()
{
#ifndef USE_DR_WAV
    switch (m_eMode)
    {
        case eFileIoMode_input:
            {
                m_lastChlRead = -1;
            }
            break;

        case eFileIoMode_output:
            {
                if (m_nIoBlockSize < 1)
                {
                    if (m_eFileType == eFileType_wav)
                        m_audioFile.save(m_sFilePath, AudioFileFormat::Wave);
                    else
                        m_audioFile.save(m_sFilePath, AudioFileFormat::Aiff);
                }
                else
                {
                    m_nCurrentFrameIdx++;

                    if (m_nCurrentFrameIdx >= m_nIoBlockSize)
                    {
                        if (m_eFileType == eFileType_wav)
                            m_audioFile.save(m_sFilePath, AudioFileFormat::Wave);
                        else
                            m_audioFile.save(m_sFilePath, AudioFileFormat::Aiff);

                        m_nCurrentFrameIdx = 0;
                    }
                }
                m_lastChlWritten = -1;
            }
            break;

        default:
            return false;
    }

    m_nCurrentFrame++;
    m_nFramesInFile++;
#else
    if (m_pFramebuffer == nullptr)
        return false;

    if (m_nIoBlockSize < 1)
    {
        m_nCurrentFrameIdx = 0;

        switch (m_eMode)
        {
            case eFileIoMode_input:
                {
                    auto status = readBlock(m_pFramebuffer, 1);
                    if (!status)
                        return false;

                    m_lastChlRead = -1;
                }
                break;

            case eFileIoMode_output:
                {
                    auto status = writeBlock(m_pFramebuffer, 1);
                    if (!status)
                        return false;

                    m_lastChlWritten = -1;
                }
                break;

            default:
                return false;
        }
    }
    else
    {
        m_nCurrentFrameIdx++;
        if (m_nCurrentFrameIdx >= m_nIoBlockSize)
        {
            switch (m_eMode)
            {
                case eFileIoMode_input:
                    {
                        auto status = readBlock(m_pFramebuffer, m_nIoBlockSize);
                        if (!status)
                            return false;

                        m_lastChlRead = -1;
                    }
                    break;

                case eFileIoMode_output:
                    {
                        auto status = writeBlock(m_pFramebuffer, m_nIoBlockSize);
                        if (!status)
                            return false;

                        m_lastChlWritten = -1;
                    }
                    break;

                default:
                    return false;
            }

            m_nCurrentFrameIdx = 0;
        }
    }
#endif

    return true;
}


bool CWavFileIO::setCurrentFrame(unsigned int frameNum) 
{
    if (!m_bFileOpened)
        return false;

#ifndef USE_DR_WAV
    auto totalNumFrames = m_audioFile.getNumSamplesPerChannel();
#else
    auto totalNumFrames = m_audioFile.totalPCMFrameCount;
#endif

    if (frameNum >= (unsigned int) totalNumFrames)
        return false;

    m_nCurrentFrame = frameNum;

    return true;
}


#ifndef _UTILITY_FUNCTION_DEFS_
#define _UTILITY_FUNCTION_DEFS_


std::string removeLeadingSpaces(std::string& sStr)
{
    std::string sOut = "";

    bool bFirstCharFound = false;

    // skip leading spaces
    for (auto x = sStr.begin(); x != sStr.end(); x++)
    {
        if (bFirstCharFound == false && (*x) == ' ')
        { 
            continue;
        }

        bFirstCharFound = true;

        sOut.push_back((*x));
    }

    return sOut;
}


std::string removeTrailingSpaces(std::string& sStr)
{
    std::string sOut = "";

    int nLen = (int) sStr.size();

    // find string len - num trailing spaces
    for (auto x = (sStr.end() - 1); x != sStr.begin(); x--)
    {
        if ((*x) != ' ')
        {
            break;
        }

        nLen--;
    }

    sOut.append(sStr.substr(0, nLen));

    return sOut;

}


std::string getFileDir(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.parent_path().string());
}


std::string getFileExt(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.extension().string());
}


std::string getFileName(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.stem().string());
}


#endif
