//****************************************************************************
// FILE:    CFileIO.h
//
// DESC:    C++ file input/output class 
//
// AUTHOR:  Russ Barker
//


#ifndef _CFILEIO_H
#define _CFILEIO_H

#include <locale>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LINE_IO_SIZE        1024


enum eFileIoMode_def
{
    eFileIoMode_unknown = 0,
    eFileIoMode_input,
    eFileIoMode_output,
    eFileIoMode_IO,
    eFileIoMode_append
};


class CFileIO
{

  private:

    FILE           *m_pFileHandle;
    std::string     m_sFilePath;
    eFileIoMode_def m_eMode;
    unsigned int    m_nLastIoSize;
    std::string     m_sLastErrorStr;
    int             m_nLastErrorNum;
    bool            m_bBinary;
    long            m_lCurrFilePos;

  public:

    explicit CFileIO(const std::string &sFilePath = "") :
        m_sFilePath(sFilePath)
    {
        m_pFileHandle   = nullptr;
        m_eMode         = eFileIoMode_def::eFileIoMode_unknown;
        m_nLastIoSize   = 0;
        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;
        m_bBinary       = false;
        m_lCurrFilePos  = 0;
    }

    ~CFileIO()
    {
        if (m_pFileHandle != nullptr)
            closeFile();
    }

    unsigned int getLastIoSize()
    {
        return m_nLastIoSize;
    }

    std::string getLastErrorText()
    {
        return m_sLastErrorStr;
    }

    int getLastErrorCode()
    {
        return m_nLastErrorNum;
    }

    bool setFilePath(const std::string &sFilePath)
    {
        if (sFilePath.empty())
        {
            m_sLastErrorStr = "File setFilePath called but file path is empty";

            return false;
        }

        m_sFilePath = sFilePath;

        return true;
    }

    void setBinaryMode(const bool val = true)
    {
        m_bBinary = val;
    }

    bool isOpen()
    {
        if (m_pFileHandle != nullptr)
            return true;

        return false;
    }

    bool openFile(const eFileIoMode_def eMode, const std::string &sFilePath = "")
    {
        if (m_pFileHandle != nullptr)
        {
            m_sLastErrorStr = "File open called but file is already open";

            return false;
        }

        std::string sFileMode;

        switch (eMode)
        {
            case eFileIoMode_def::eFileIoMode_input:
                {
                    sFileMode = "r";
                    m_eMode   = eMode;
                }
                break;

            case eFileIoMode_def::eFileIoMode_output:
                {
                    sFileMode = "w";
                    m_eMode   = eMode;
                }
                break;

            case eFileIoMode_def::eFileIoMode_IO:
                {
                    sFileMode = "r+";
                    m_eMode   = eMode;
                }
                break;

            case eFileIoMode_def::eFileIoMode_append:
                {
                    sFileMode = "a+";
                    m_eMode   = eMode;
                }
                break;

            default:
                return false;
        }

        if (!sFilePath.empty())
            m_sFilePath = sFilePath;

        if (m_sFilePath == "")
        {
            m_sLastErrorStr = "File open called but file path not set";

            return false;
        }

        if (m_bBinary)
            sFileMode.append("b");

        try
        {
            FILE *pFile = nullptr;
#ifdef WINDOWS
            auto status = fopen_s(&pFile, m_sFilePath.c_str(), sFileMode.c_str());
#else
            pFile = fopen(m_sFilePath.c_str(), sFileMode.c_str());
#endif
            if (pFile == nullptr)
            {
                m_sLastErrorStr = "File open operation faild";
                // m_nLastErrorNum = ferror(pFile);

                return false;
            }

            m_pFileHandle = pFile;

            auto pos      = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }

            m_lCurrFilePos = pos;
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file open operation";

            return false;
        }

        m_nLastIoSize   = 0;
        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return true;
    }

    bool closeFile()
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File close called but file not open";

            return false;
        }

        try
        {
            if (fclose(m_pFileHandle) != 0)
            {
                m_sLastErrorStr = "File close operation faild";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file close operation";

            return false;
        }

        m_pFileHandle   = nullptr;
        m_nLastIoSize   = 0;
        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return true;
    }

    long getFileSize()
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File getSize called but file not open";

            return false;
        }

        long length;

        try
        {
            fpos_t fPos;

            if (fgetpos(m_pFileHandle, &fPos) != 0)
            {
                m_sLastErrorStr = "File 'getpos' call failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return -1;
            }

            if (fseek(m_pFileHandle, 0L, SEEK_END) != 0)
            {
                m_sLastErrorStr = "File 'seek' (to EOF) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return -1;
            }

            // calculating the size of the file
            length = ftell(m_pFileHandle);
            if (length < 0)
            {
                m_sLastErrorStr = "File 'tell' (file length) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return -1;
            }

            if (fgetpos(m_pFileHandle, &fPos) != 0)
            {
                m_sLastErrorStr = "File 'getpos' call failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return -1;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file getSize operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return length;
    }

    long getFilePosition()
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File getPosition called but file not open";

            return -1;
        }

        long pos;

        try
        {
            pos = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return -1;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file getPosition operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;
        m_lCurrFilePos  = pos;

        return (long)pos;
    }

    bool setFilePosition(const unsigned long position)
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File setPosition called but file not open";

            return false;
        }

        try
        {
            if (fseek(m_pFileHandle, position, SEEK_SET) != 0)
            {
                m_sLastErrorStr = "File 'seek' (to position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file setPosition operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;
        m_lCurrFilePos  = position;

        return true;
    }

    int checkEof()
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File checkEof called but file not open";
            return false;
        }

        int retCode = 0;

        try
        {
            if (feof(m_pFileHandle) != 0)
                retCode = 1;
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file checkEof operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return -1;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return retCode;
    }

    bool readLine(std::string &sInput, int &nNumBytesRead)
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File readBlock called but file not open";

            return false;
        }

        if (m_bBinary == true)
        {
            m_sLastErrorStr = "File readLine called but file is opened in binary mode";

            return false;
        }

        try
        {
#ifdef SET_FILE_POSITION_BEFORE_READ
            try
            {
                if (fseek(m_pFileHandle, m_lCurrFilePos, SEEK_SET) != 0)
                {
                    m_sLastErrorStr = "File 'seek' (to position) operation failed";
                    m_nLastErrorNum = ferror(m_pFileHandle);

                    return false;
                }
            }
            catch (...)
            {
                m_sLastErrorStr = "Unknown exception during file 'seek' (to position) operation";
                m_nLastErrorNum = -1;
            }
#endif
            int nReadSize = MAX_LINE_IO_SIZE;

            char *pInputBuffer = (char *) calloc(sizeof(char), nReadSize);

           auto status = ::fgets(pInputBuffer, nReadSize, m_pFileHandle);

            if (status != nullptr && nReadSize > 0)
                m_nLastIoSize = (unsigned int) nReadSize;
            else
                m_nLastIoSize = (unsigned int) 0;

#ifdef UPDATE_FILE_POSITION
            auto pos = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);
                return false;
            }

            m_lCurrFilePos = pos;
#endif

            if (nReadSize < 1)
            {
                if (feof(m_pFileHandle) != 0)
                {
                    m_sLastErrorStr = "File 'fread' operation encountered EOF";
                }
                else
                {
                    m_sLastErrorStr = "File 'fread' operation failed";
                    m_nLastErrorNum = ferror(m_pFileHandle);
                }

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file readBlock operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        nNumBytesRead = m_nLastIoSize;

        return true;
    }

    bool readBlock(void *pData, const unsigned int blockSize, const unsigned int numBlocks)
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File readBlock called but file not open";

            return false;
        }

        try
        {
#ifdef SET_FILE_POSITION_BEFORE_READ
            try
            {
                if (fseek(m_pFileHandle, m_lCurrFilePos, SEEK_SET) != 0)
                {
                    m_sLastErrorStr = "File 'seek' (to position) operation failed";
                    m_nLastErrorNum = ferror(m_pFileHandle);

                    return false;
                }
            }
            catch (...)
            {
                m_sLastErrorStr = "Unknown exception during file 'seek' (to position) operation";
                m_nLastErrorNum = -1;
            }
#endif

            auto blocksToRead = numBlocks;
            auto length       = fread(pData, blockSize, blocksToRead, m_pFileHandle);

            if (length > (size_t) 0)
                m_nLastIoSize = (unsigned int) length;
            else
                m_nLastIoSize = (unsigned int) 0;

#ifdef UPDATE_FILE_POSITION
            auto pos = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);
                return false;
            }

            m_lCurrFilePos = pos;
#endif

            if (length != (size_t) blocksToRead)
            {
                if (feof(m_pFileHandle) != 0)
                {
                    m_sLastErrorStr = "File 'fread' operation encountered EOF";
                }
                else
                {
                    m_sLastErrorStr = "File 'fread' operation failed";
                    m_nLastErrorNum = ferror(m_pFileHandle);
                }

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file readBlock operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return true;
    }

    bool writeLine(const std::string &sOutput)
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File readBlock called but file not open";
            return false;
        }

        try
        {
            unsigned int bloclSize = (unsigned int) sOutput.length();

            //auto length = fputs(sOutput.c_str(), m_pFileHandle);
            auto length = fwrite((const void *) sOutput.data(), bloclSize, 1, m_pFileHandle);

            if (length > 0)
                //m_nLastIoSize = (unsigned int) length;
                m_nLastIoSize = (unsigned int) bloclSize;
            else
                m_nLastIoSize = 0;

#ifdef UPDATE_FILE_POSITION
            auto pos = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }

            m_lCurrFilePos = pos;
#endif

            if (length != sOutput.length())
            {
                m_sLastErrorStr = "File 'fputs' operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file writeBlock operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return true;
    }

    bool writeBlock(const void *pData, const unsigned int bloclSize, const unsigned int numBlocks)
    {
        if (m_pFileHandle == nullptr)
        {
            m_sLastErrorStr = "File readBlock called but file not open";
            return false;
        }

        try
        {
            auto blocksToWrite = numBlocks;
            auto length        = fwrite(pData, bloclSize, blocksToWrite, m_pFileHandle);

            if (length > (size_t) 0)
                m_nLastIoSize = (unsigned int) length;
            else
                m_nLastIoSize = 0;

#ifdef UPDATE_FILE_POSITION
            auto pos = ftell(m_pFileHandle);
            if (pos < 0)
            {
                m_sLastErrorStr = "File 'tell' (file position) operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }

            m_lCurrFilePos = pos;
#endif

            if (length != (size_t)blocksToWrite)
            {
                m_sLastErrorStr = "File 'fwrite' operation failed";
                m_nLastErrorNum = ferror(m_pFileHandle);

                return false;
            }
        }
        catch (...)
        {
            m_sLastErrorStr = "Uknown exception during file writeBlock operation";
            m_nLastErrorNum = ferror(m_pFileHandle);

            return false;
        }

        m_sLastErrorStr = "";
        m_nLastErrorNum = 0;

        return true;
    }
};

#endif // _CFILEIO_H
