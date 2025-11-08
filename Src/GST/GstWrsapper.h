/// 
/// \file       GstWrapper.h
/// 
///             Defines functions & classes for CGstWrapper
///


#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <glib.h>

#include <string>
#include <vector>
#include <mutex>


static GstFlowReturn new_sample_callback(GstElement* pSink, CGstWrapper::ControlData_def* pAppData);


class CGstWrapper
{
    // data type defs

public:
    class CDataBuffer
    {
        void            *m_pIoBuffer;

        bool            m_bNetData;

        unsigned int    m_nCurDataLen;

        std::mutex      m_buferMutex;

        std::mutex      m_hasDataMutex;

        std::mutex      m_dataLenMutex;

        bool            m_bIsLocked;

    public:

        CDataBuffer()
        {
            m_pIoBuffer = nullptr;

            m_bNetData = false;

            m_nCurDataLen = 0;

            m_bIsLocked = false;
        }

        ~CDataBuffer()
        {
            if (m_bIsLocked == true)
            {
                m_buferMutex.unlock();
            }

            free();
        }

        bool allocate(const unsigned int nSize)
        {
            if (m_pIoBuffer != nullptr)
            {
                return false;
            }

            m_bIsLocked = true;

            m_buferMutex.lock();

            m_pIoBuffer = calloc(nSize, sizeof(uint8_t));

            m_buferMutex.unlock();

            m_bIsLocked = false;

            m_nCurDataLen = 0;

            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            return true;            
        }

        bool isAllocated()
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            return true;
        }

        bool free()
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            m_bIsLocked = true;

            m_buferMutex.lock();

            ::free(m_pIoBuffer);

            m_pIoBuffer = nullptr;

            m_buferMutex.unlock();

            m_bIsLocked = false;

            m_nCurDataLen = 0;

            return true;
        }

        void* map()
        {
            if (m_pIoBuffer == nullptr)
            {
                return nullptr;
            }

            m_bIsLocked = true;
            
            m_buferMutex.lock();

            return m_pIoBuffer;
        }

        bool unmap()
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            m_buferMutex.unlock();

            m_bIsLocked = false;

            return true;
        }

        bool isLocked()
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            return m_bIsLocked;
        }

        void setHasData(const bool bVal)
        {
            m_hasDataMutex.lock();

            m_bNetData = bVal;

            m_hasDataMutex.unlock();
        }

        bool hasNewData()
        {
            m_hasDataMutex.lock();

            bool bOut = m_bNetData;

            m_hasDataMutex.unlock();

            return bOut;
        }

        bool setCurrentDataLen(const unsigned int nLen)
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            m_bIsLocked = true;

            m_dataLenMutex.lock();

            m_nCurDataLen = nLen;;

            if (nLen == 0)
            {
                m_hasDataMutex.lock();

                m_bNetData = false;

                m_hasDataMutex.unlock();
            }

            m_dataLenMutex.unlock();

            m_bIsLocked = false;

            return true;
        }

        unsigned int getCurrentDataLen()
        {
            if (m_pIoBuffer == nullptr)
            {
                return 0;
            }

            m_bIsLocked = true;

            m_buferMutex.lock();

            auto nOut = m_nCurDataLen;

            m_buferMutex.unlock();

            m_bIsLocked = false;

            return nOut;
        }
    };

    // Structure to hold custom data
    typedef struct ControlData_tag
    {
        bool            m_bGstInitialized;

        GstBus          *m_pBus;
        GstMessage      *m_pMsg;

        GstElement      *m_pPipeline;

        GstAppSrc       *m_pAppsrc;
        GstElement      *m_pAppsink;

        gboolean        m_bQuit;

        unsigned int    m_width;
        unsigned int    m_height;

        unsigned int    m_bitsPerPixel;

        unsigned int    m_frameRate;

        CDataBuffer     m_ioBuffer;

        bool            m_bPipelineActive;

        ConfigData_tag()
        {
            m_bGstInitialized = false;

            m_pBus = nullptr;
            m_pMsg = nullptr;

            m_pPipeline = nullptr;

            m_pAppsrc = nullptr;
            m_pAppsink = nullptr;

            m_bQuit = false;

            m_width = 0;
            m_height = 0;

            m_bitsPerPixel = 0;

            m_frameRate = 0;

            m_bPipelineActive = false;
        }

        bool createIoBuffer()
        {
            if (m_ioBuffer.isAllocated() == true)
            {
                return false;
            }

            unsigned int nLengthInBytes = ((m_width * m_height * m_bitsPerPixel) / 8);

            if (nLengthInBytes < 1)
            {
                return false;
            }

            return m_ioBuffer.allocate(nLengthInBytes);
        }

        bool freeIoBuffer()
        {
            if (m_ioBuffer.isAllocated() == false)
            {
                return false;
            }

            return m_ioBuffer.free();
        }

    } ControlData_def;

    typedef std::vector<std::string>    InitParamList_def;

protected:

    // Data member defs

    InitParamList_def           m_paramList;

    ControlData_def             m_controlData;

public:

    CGstWrapper()
    {
    }

    ~CGstWrapper()
    {
    }

    bool SetParamList(InitParamList_def &paramList)
    {
        m_paramList = paramList;

        return true;
    }

    bool SetParamList(int argc, char* argv[])
    {
        for (int i = 0; i < argc; i++)
        {
            m_paramList.push_back(*argv[i]);
        }

        return true;
    }

    bool Initialize();

    bool BuildInputPipeline(const std::string& sPipeline, const std::string& sAppSibk);

    bool BuildOutputPipeline(const std::string& sPipeline, const std::string& sAppSource);

    bool BuildIoPipeline(const std::string& sPipeline, const std::string& sAppSink, const std::string& sAppSource);

    bool SetSignalCallback(const std::string& sSignal);

    void FreeBuffer(GstBuffer *pBuffer);

    GstBuffer *CreateBuffer(const unsigned int nSize);

    GstBuffer * CreateAndFillBuffer(const unsigned char *pData, const unsigned int nSize);

    bool FillGstBuffer(GstBuffer* pBuffer, const unsigned char* pData, const unsigned int nSize);

    // Function to push buffers into the pipeline
    bool WriteToPipeline(GstBuffer* pBuffer);

    bool StartPipeline();

    bool StopPipeline();

    bool DataReceivedFromGst()
    {
        return m_controlData.freeIoBuffer.hasNewData();
    }

    bool ResetGstDataReceived()
    {
        return m_controlData.freeIoBuffer.setHasData(false);
    }

    unsigned int GetCurrentGstDataLen()
    {
        return m_gstConfigData.m_ioBuffer.getCurrentDataLen();
    }

    unsigned int ResetCurrentGstDataLen()
    {
        return m_gstConfigData.m_ioBuffer.setCurrentDataLen(0);
    }

    void Release();

};


