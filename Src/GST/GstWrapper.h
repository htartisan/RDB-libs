/// 
/// \file       GstWrapper.h
/// 
///             Defines functions & classes for CGstWrapper
///


#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>

#include <glib.h>

#include <string>
#include <vector>
#include <mutex>
#include <future> 

#ifdef WINDOWS
#include <windows.h>
#endif

#include <thread>

#include "../Thread/ThreadBase.h"


typedef struct SGstElementInfo_tag
{
    GstElement*     m_pElement;

    GType           m_elementType;

    std::string     m_sElementName;     

    SGstElementInfo_tag()
    {
        m_pElement = nullptr;

        m_elementType = 0;

        m_sElementName = "";
    }
    
    SGstElementInfo_tag(GstElement* pElement)
    {
        m_pElement = pElement;

        if (m_pElement != nullptr)
        {
            m_elementType = G_OBJECT_TYPE(m_pElement);

            m_sElementName = gst_element_get_name(m_pElement);
        }
    }

}SGstElementInfo_def;



typedef std::vector<SGstElementInfo_def>       gstElementList_def;


bool listAllPipelineElements(GstPipeline* pPipeline, gstElementList_def& elementList);


class CGstWrapper
{
    // data type defs

public:

    enum eMediaType
    {
        eMediaType_unknown = 0,
        eMediaType_video,
        eMediaType_audio
    };

    struct SMediaInfo
    {
        eMediaType      m_eMediaType;

        std::string     m_sFourCC;
        
        unsigned int    m_nCurDataLen;

        SMediaInfo()
        {
            m_eMediaType = eMediaType_unknown;

            m_sFourCC = "";

            m_nCurDataLen = 0;
        }
    };

    class CMediaBuffer
    {
        void            *m_pIoBuffer;

        bool            m_bNetData;

        unsigned int    m_nMaxDataLen;

        std::mutex      m_buferMutex;

        std::mutex      m_hasDataMutex;

        std::mutex      m_dataLenMutex;

        bool            m_bIsLocked;

        SMediaInfo      m_mediaInfo;

    public:

        CMediaBuffer()
        {
            m_pIoBuffer = nullptr;

            m_bNetData = false;

            m_bIsLocked = false;
        }

        ~CMediaBuffer()
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

            m_mediaInfo.m_nCurDataLen = 0;

            m_buferMutex.unlock();

            m_bIsLocked = false;

            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            m_nMaxDataLen = nSize;

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

            m_mediaInfo.m_nCurDataLen = 0;

            m_pIoBuffer = nullptr;

            m_buferMutex.unlock();

            m_bIsLocked = false;

            return true;
        }

        bool clear()
        {
            if (m_pIoBuffer == nullptr)
            {
                return false;
            }

            m_bIsLocked = true;

            m_buferMutex.lock();

            m_mediaInfo.m_nCurDataLen = 0;

            m_bNetData = false;

            m_buferMutex.unlock();

            m_bIsLocked = false;

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

            m_mediaInfo.m_nCurDataLen = nLen;;

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

            auto nOut = m_mediaInfo.m_nCurDataLen;

            m_buferMutex.unlock();

            m_bIsLocked = false;

            return nOut;
        }

        unsigned int getMaxDataLen()
        {
            if (m_pIoBuffer == nullptr)
            {
                return 0;
            }

            m_bIsLocked = true;

            return m_nMaxDataLen;
        }

        void SetMediaType(eMediaType eType)
        {
            m_mediaInfo.m_eMediaType = eType;
        }

        eMediaType GetMediaType()
        {
            return m_mediaInfo.m_eMediaType;
        }

        void SetFourCC(const std::string sFourCC)
        {
            m_mediaInfo.m_sFourCC = sFourCC;
        }

        std::string GetFourCC()
        {
            return m_mediaInfo.m_sFourCC;
        }
    };

    typedef struct VideoConfig_tag
    {
        unsigned int    m_width;
        unsigned int    m_height;
        unsigned int    m_bitsPerPixel;
        unsigned int    m_frameRate;

        VideoConfig_tag()
        {
            m_width = 0;
            m_height = 0;
            m_bitsPerPixel = 0;
            m_frameRate = 0;
        }

    } VideoConfig_def;

    typedef struct AudioConfig_tag
    {
        unsigned int    m_framesPerBlock;
        unsigned int    m_numChannels;
        unsigned int    m_bitsPerSample;
        unsigned int    m_sampleRate;

        AudioConfig_tag()
        {
            m_framesPerBlock = 0;
            m_numChannels = 0;
            m_bitsPerSample = 0;
            m_sampleRate = 0;
        }

    } AudioConfig_def;

    // Structure to hold custom data
    typedef struct ControlData_tag
    {
        bool                m_bGstInitialized;

        GstBus              *m_pBus;
        GstMessage          *m_pMsg;

        GMainLoop           *m_pBusLoop;

        GstPipeline         *m_pPipeline;

        gstElementList_def  m_elementsList;     // A std::vvector with a list of pipeline element pointers

        GstAppSrc           *m_pAppsrc;
        GstAppSink          *m_pAppsink;

        gboolean            m_bQuit;

        VideoConfig_def     m_videoConfig;

        AudioConfig_def     m_audioConfig;

        CMediaBuffer        m_mediaBuffer;

        bool                m_bPipelineActive;

        std::string         m_sLastError;

        ControlData_tag()
        {
            m_bGstInitialized = false;

            m_pBus = nullptr;
            m_pMsg = nullptr;

            m_pBusLoop = nullptr;

            m_pPipeline = nullptr;

            m_pAppsrc = nullptr;
            m_pAppsink = nullptr;

            m_bQuit = false;

            m_bPipelineActive = false;

            m_sLastError = "";
        }

        bool createFrameBuffer(eMediaType eType)
        {
            if (m_mediaBuffer.isAllocated() == true)
            {
                return false;
            }

            unsigned int nLengthInBytes = 0;

            switch (eType)
            {
                case eMediaType::eMediaType_video:
                    nLengthInBytes = 
                        ((m_videoConfig.m_width * m_videoConfig.m_height * m_videoConfig.m_bitsPerPixel) / 8);
                    break;

                case eMediaType::eMediaType_audio:
                    nLengthInBytes =
                        ((m_audioConfig.m_framesPerBlock * m_audioConfig.m_numChannels * m_audioConfig.m_bitsPerSample) / 8);
                    break;

                default:
                    return false;
            }

            if (nLengthInBytes < 1)
            {
                return false;
            }

            return m_mediaBuffer.allocate(nLengthInBytes);
        }

        bool freeFrameuffer()
        {
            if (m_mediaBuffer.isAllocated() == false)
            {
                return false;
            }

            return m_mediaBuffer.free();
        }

        bool isPipelineActive()
        {
            return m_bPipelineActive;
        }

        bool stopPipeline()
        {
            if (m_pPipeline != nullptr)
            {
                try
                {
                    // Set the pipeline to PLAYING state
                    auto ret = gst_element_set_state((GstElement*) m_pPipeline, GST_STATE_NULL);

                    if (ret == GST_STATE_CHANGE_FAILURE)
                    {
                        return false;
                    }
                }
                catch (...)
                {
                    return false;
                }

                return true;
            }

#ifdef USE_GST_SAMPLE_CALLBACK
            if (m_pBusLoop != nullptr)
            {
                if (g_main_loop_is_running(m_pBusLoop) != false)
                {
                    g_main_loop_quit(m_pBusLoop);

                    unsigned int nCounter = 0;

                    while (g_main_loop_is_running(m_pBusLoop) != false)
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(5));

                        if (nCounter++ > 20)
                        {
                            break;
                        }
                    }

                    m_pBusLoop = nullptr;
                }
            }
#endif

            return true;
        }

    } ControlData_def;

    typedef std::vector<std::string>    InitParamList_def;

protected:

    // Data member defs

    InitParamList_def           m_paramList;

    ControlData_def             m_controlData;

    // Protected function defs

    static void RunBusLoop(void *pData);

    void releaseAllElements();

public:

    CGstWrapper()
    {
    }

    ~CGstWrapper()
    {
        Release();
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
            m_paramList.push_back(argv[i]);
        }

        return true;
    }

    bool Initialize();

    void SetFrameSize(const unsigned int nWidth, const unsigned int nHeight)
    {
        m_controlData.m_videoConfig.m_width = nWidth;
        m_controlData.m_videoConfig.m_height = nHeight;
    }

    void SetPixelSize(const unsigned int nBitsPerPixel)
    {
        m_controlData.m_videoConfig.m_bitsPerPixel = nBitsPerPixel;
    }

    void SetFrameRate(const unsigned int nRate)
    {
        m_controlData.m_videoConfig.m_frameRate = nRate;
    }

    bool BuildPipeline(const std::string& sPipeline);

    bool BuildInputPipeline(const std::string& sPipeline, const std::string& sAppSibk, const eMediaType eType);

    bool BuildOutputPipeline(const std::string& sPipeline, const std::string& sAppSource, const eMediaType eType);

    bool BuildIoPipeline(const std::string& sPipeline, const std::string& sAppSink, const std::string& sAppSource, const eMediaType eType);

    bool SetSignalCallback(const std::string& sSignal);

    void FreeBuffer(GstBuffer *pBuffer);

    GstBuffer *CreateBuffer(const unsigned int nSize);

    GstBuffer * CreateAndFillBuffer(const unsigned char *pData, const unsigned int nSize);

    bool FillGstBuffer(GstBuffer* pBuffer, const unsigned char* pData, const unsigned int nSize, const std::string sFourCC = "");

    bool PullSampleFromAppSink
        (
            void* pTarget,
            const unsigned int nBufferSize,
            SMediaInfo& mediaInfo
        );

    // Function to push buffers into the pipeline
    bool WriteToPipeline(GstBuffer* pBuffer);

    bool StartPipeline();

    bool StopPipeline();

    bool DataReceivedFromGst()
    {
        return m_controlData.m_mediaBuffer.hasNewData();
    }

    void ResetGstDataReceived()
    {
        m_controlData.m_mediaBuffer.setHasData(false);
    }

    unsigned int GetCurrentGstDataLen()
    {
        return m_controlData.m_mediaBuffer.getCurrentDataLen();
    }

    bool GetPipelineActive()
    {
        return m_controlData.m_bPipelineActive;
    }

    void ResetCurrentGstDataLen()
    {
        m_controlData.m_mediaBuffer.setCurrentDataLen(0);
    }

    void* MapFrameBuffer()
    {
        return m_controlData.m_mediaBuffer.map();
    }

    int GetMediaBufferType()
    {
        return m_controlData.m_mediaBuffer.GetMediaType();
    }

    std::string GetMediaBufferFourCC()
    {
        return m_controlData.m_mediaBuffer.GetFourCC();
    }

    bool UnmapFrameBuffer()
    {
        return m_controlData.m_mediaBuffer.unmap();
    }

    void Release();

    std::string GetLastError()
    {
        auto sOut = m_controlData.m_sLastError;

        m_controlData.m_sLastError = "";

        return sOut;
    }

    bool ReceivedEStopSignal()
    {
        return m_controlData.m_bQuit;
    }

};


