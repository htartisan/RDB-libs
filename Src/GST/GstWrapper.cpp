/// 
/// \file       GstWrapper.cpp
/// 
///             Defines functions & classes for CGstWrapper::
///


#include "GstWrapper.h"

#include <thread>

#include <chrono>

#include "../String/StrUtils.h"


// Callback function for new samples from appsink
static GstFlowReturn gst_new_sample_callback
    (
        GstElement* pSink, 
        CGstWrapper::ControlData_def* pControlData
    )
{
    if (pSink == nullptr || pControlData == nullptr)
    {
        return GST_FLOW_ERROR;
    }

    if (pControlData->m_mediaBuffer.isAllocated() == false)
    {
        return GST_FLOW_ERROR;
    }

    GstSample* pSample = nullptr;
    GstBuffer* pBuffer = nullptr;
    GstCaps* pMediaCaps = nullptr;

    GstMapInfo map_info;

    // Pull the sample from the appsink
    g_signal_emit_by_name(pSink, "pull-sample", &pSample);

    if (pSample != nullptr)
    {
        pMediaCaps = gst_sample_get_caps(pSample);

        if (pMediaCaps != nullptr)
        {
            auto numCapsEntried = gst_caps_get_size(pMediaCaps);

            for (int c = 0; c < numCapsEntried; c++)
            {
                GstStructure* structure = gst_caps_get_structure(pMediaCaps, c);

                if (structure != nullptr)
                {
                    if (pControlData->m_mediaBuffer.GetMediaType() == CGstWrapper::eMediaType::eMediaType_unknown)
                    {
                        auto mediaInfo = gst_structure_get_name(structure);

                        std::string sMediaInfo = mediaInfo;

                        if (StrUtils::strCompare(sMediaInfo, "video", 5) == 0)
                        {
                            pControlData->m_mediaBuffer.SetMediaType(CGstWrapper::eMediaType::eMediaType_video);
                        }
                        else if (StrUtils::strCompare(sMediaInfo, "audio", 5) == 0)
                        {
                            pControlData->m_mediaBuffer.SetMediaType(CGstWrapper::eMediaType::eMediaType_audio);
                        }
                    }

                    if (pControlData->m_mediaBuffer.GetFourCC() == "")
                    {
                        auto mediaFormat = gst_structure_get_string(structure, "format");

                        std::string sMediaFormat = mediaFormat;

                        pControlData->m_mediaBuffer.SetFourCC(sMediaFormat);
                    }
                }
            }
        }

        pBuffer = gst_sample_get_buffer(pSample);

        if (pBuffer == nullptr)
        {
            return GST_FLOW_ERROR;
        }

        // Map the buffer to access its m_controlData
        if (gst_buffer_map(pBuffer, &map_info, GST_MAP_READ))
        {
            // Copy the frame m_controlData to the application IO buffer
            // Ensure the frame buffer is allocated with enough memory (width * height * bytes_per_pixel)

            auto pIoBuffer = pControlData->m_mediaBuffer.map();

            memcpy(pIoBuffer, map_info.data, map_info.size);

            pControlData->m_mediaBuffer.unmap();

            pControlData->m_mediaBuffer.setCurrentDataLen((unsigned int) map_info.size);

            gst_buffer_unmap(pBuffer, &map_info);

            pControlData->m_mediaBuffer.setHasData(true);
        }
        else
        {
            // Problem mapping GstBuffer

            return GST_FLOW_ERROR;
        }

        gst_sample_unref(pSample);

        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}



bool CGstWrapper::Initialize()
{
    if (m_controlData.m_bGstInitialized == true)
    {
        return false;
    }

#if 0

    int nParamCnt = (int) m_paramList.size();

    char *pArgArray[100] = {};

    for (int i = 0; i < nParamCnt; i++)
    {
        pArgArray[i] = m_paramList[i].m_controlData();
    }

    auto ppArray = &(pArgArray);

#else

    int nParamCnt = 0;

    char *** ppArray = nullptr;

#endif

    gst_init(&nParamCnt, (char***) ppArray);

    m_controlData.m_bGstInitialized = true;

    return true;
}


bool CGstWrapper::BuildInputPipeline
    (
        const std::string& sPipeline, 
        const std::string& sAppSink, 
        const eMediaType eType
    )
{
    if (sPipeline == "" || sAppSink == "")
    {
        return false;
    }

    if (eType == eMediaType::eMediaType_video)
    {
        if (m_controlData.m_videoConfig.m_width < 1 || 
            m_controlData.m_videoConfig.m_height < 1 ||
            m_controlData.m_videoConfig.m_bitsPerPixel < 8)
        {
            return false;
        }
    }
    else if (eType == eMediaType::eMediaType_audio)
    {
        if (m_controlData.m_audioConfig.m_framesPerBlock < 1 || 
            m_controlData.m_audioConfig.m_numChannels < 1 ||
            m_controlData.m_audioConfig.m_bitsPerSample < 8)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (m_controlData.m_mediaBuffer.isAllocated() == false)
    {
        auto status = m_controlData.createFrameBuffer(eType);

        if (status == false)
        {
            return false;
        }
    }

    //GstStateChangeReturn ret;

    try
    {
        if (m_controlData.m_bGstInitialized == false)
        {
            Initialize();
        }

        // Build the pipeline using gst_parse_launch

        GError *pGError = nullptr;

        m_controlData.m_pPipeline = 
            gst_parse_launch(sPipeline.c_str(), &pGError);

        if (m_controlData.m_pPipeline == nullptr)
        {
            return false;
        }

        // Get the appsink element

        m_controlData.m_pAppsink = 
            gst_bin_get_by_name(GST_BIN(m_controlData.m_pPipeline), sAppSink.c_str());
        
        if (m_controlData.m_pAppsink == nullptr)
        {
            return false;
        }

        g_signal_connect(m_controlData.m_pAppsink, "new-sample", G_CALLBACK(gst_new_sample_callback), &m_controlData);

        //gst_app_sink_set_emit_signals((GstAppSink*) m_controlData.m_pAppsink, TRUE);
        //gst_app_sink_set_drop((GstAppSink*)m_controlData.m_pAppsink, TRUE); // Drop old buffers if app is slow

        // Configure appsink

        g_object_set(G_OBJECT(m_controlData.m_pAppsink), "emit-signals", TRUE, "max-buffers", 1, "drop", TRUE, NULL);
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool CGstWrapper::BuildOutputPipeline
    (
        const std::string& sPipeline, 
        const std::string& sAppSource, 
        const eMediaType eType
    )
{
    if (sPipeline == "" || sAppSource == "")
    {
        return false;
    }

    if (eType == eMediaType::eMediaType_video)
    {
        if (m_controlData.m_videoConfig.m_width < 1 ||
            m_controlData.m_videoConfig.m_height < 1 ||
            m_controlData.m_videoConfig.m_bitsPerPixel < 8)
        {
            return false;
        }
    }
    else if (eType == eMediaType::eMediaType_audio)
    {
        if (m_controlData.m_audioConfig.m_framesPerBlock < 1 ||
            m_controlData.m_audioConfig.m_numChannels < 1 ||
            m_controlData.m_audioConfig.m_bitsPerSample < 8)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    GstStateChangeReturn ret;

    try
    {
        // Build the pipeline using gst_parse_launch

        GError* pGError = nullptr;

        m_controlData.m_pPipeline = 
            gst_parse_launch(sPipeline.c_str(), &pGError);

        if (m_controlData.m_pPipeline == nullptr)
        {
            return false;
        }

        // Get the appsrc element

        m_controlData.m_pAppsrc = 
            GST_APP_SRC(gst_bin_get_by_name(GST_BIN(m_controlData.m_pPipeline), sAppSource.c_str()));

        if (m_controlData.m_pAppsrc == nullptr)
        {
            return false;
        }

        // Configure appsrc

        g_object_set(G_OBJECT(m_controlData.m_pAppsrc), "emit-signals", TRUE, "is-live", TRUE, "format", GST_FORMAT_TIME, NULL);

        // Set the pipeline to the PLAYING state

        ret = gst_element_set_state(m_controlData.m_pPipeline, GST_STATE_PLAYING);

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


bool CGstWrapper::BuildIoPipeline
    (
        const std::string& sPipeline, 
        const std::string& sAppSink, 
        const std::string& sAppSource, 
        const eMediaType eType
    )
{
    if (sPipeline == "" || sAppSource == "")
    {
        return false;
    }

    if (eType == eMediaType::eMediaType_video)
    {
        if (m_controlData.m_videoConfig.m_width < 1 ||
            m_controlData.m_videoConfig.m_height < 1 ||
            m_controlData.m_videoConfig.m_bitsPerPixel < 8)
        {
            return false;
        }
    }
    else if (eType == eMediaType::eMediaType_audio)
    {
        if (m_controlData.m_audioConfig.m_framesPerBlock < 1 ||
            m_controlData.m_audioConfig.m_numChannels < 1 ||
            m_controlData.m_audioConfig.m_bitsPerSample < 8)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    if (m_controlData.m_mediaBuffer.isAllocated() == false)
    {
        auto status = m_controlData.createFrameBuffer(eType);

        if (status == false)
        {
            return false;
        }
    }

    //GstStateChangeReturn ret;

    try
    {
        // Build the pipeline using gst_parse_launch

        GError* pGError = nullptr;

        m_controlData.m_pPipeline = 
            gst_parse_launch(sPipeline.c_str(), &pGError);

        if (m_controlData.m_pPipeline == nullptr)
        {
            return false;
        }

        // Get the appsrc element

        m_controlData.m_pAppsrc = 
            GST_APP_SRC(gst_bin_get_by_name(GST_BIN(m_controlData.m_pPipeline), sAppSource.c_str()));

        if (m_controlData.m_pAppsrc == nullptr)
        {
            return false;
        }

        // Get the appsink element

        m_controlData.m_pAppsink = 
            gst_bin_get_by_name(GST_BIN(m_controlData.m_pPipeline), sAppSink.c_str());

        if (m_controlData.m_pAppsink == nullptr)
        {
            return false;
        }

        // Configure appsink

        g_signal_connect(m_controlData.m_pAppsink, "new-sample", G_CALLBACK(gst_new_sample_callback), &m_controlData);

        //gst_app_sink_set_emit_signals((GstAppSink*) m_controlData.m_pAppsink, TRUE);
        //gst_app_sink_set_drop((GstAppSink*)m_controlData.m_pAppsink, TRUE); // Drop old buffers if app is slow

        // Configure appsrc

        g_object_set(G_OBJECT(m_controlData.m_pAppsink), "emit-signals", TRUE, "max-buffers", 1, "drop", TRUE, NULL);
    }
    catch (...)
    {
        return false;
    }

    return true;
}


void CGstWrapper::FreeBuffer(GstBuffer* pBuffer)
{
    if (pBuffer == nullptr)
    {
        return ;
    }

    gst_object_unref(pBuffer);

    pBuffer = nullptr;
}


GstBuffer* CGstWrapper::CreateBuffer(const unsigned int nSize)
{
    if (nSize < 1)
    {
        return nullptr;
    }

    GstBuffer* pBuffer = nullptr;

    pBuffer = gst_buffer_new_allocate(nullptr, nSize, nullptr);

    if (pBuffer == nullptr)
    {
        return nullptr;
    }

    return pBuffer;
}


GstBuffer * CGstWrapper::CreateAndFillBuffer
    (
        const unsigned char* pData, 
        const unsigned int nSize
    )
{
    if (pData == nullptr || nSize < 1)
    {
        return nullptr;
    }

    GstBuffer* pBuffer = nullptr;;

    // Allocate a new GstBuffer with memory of the given size

    pBuffer = gst_buffer_new_and_alloc(nSize);

    if (pBuffer != nullptr)
    {
        if (pBuffer == nullptr)
        {
            return nullptr;
        }

        // Fill the allocated buffer with your raw YUY2 m_controlData

        if (gst_buffer_fill(pBuffer, 0, pData, nSize) != nSize)
        {
            gst_object_unref(pBuffer);

            return nullptr;
        }

        // Optional: Set buffer metadata as above
    }

    return pBuffer;
}


bool CGstWrapper::FillGstBuffer(GstBuffer* pBuffer, const unsigned char* pData, const unsigned int nSize)
{
    if (pBuffer == nullptr || pData == nullptr || nSize < 1)
    {
        return false;
    }

    // Sleep (for 5ms at a time) until buffer is writable

    unsigned int nLoopCntr = 0;

    if (gst_buffer_is_writable(pBuffer) == false)
    {
        return false;
    }

    GstFlowReturn ret = GST_FLOW_OK;

    // Map the buffer and fill it with some dummy YUY2 m_controlData (e.g., a simple gradient or static image)

    try
    { 
        GstMapInfo map_info;

        memset(&map_info, 0, sizeof(GstMapInfo));

        auto status = gst_buffer_map(pBuffer, &map_info, GST_MAP_WRITE);

        if (status == false)
            return false;

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        // Copy video m_controlData into map.m_controlData

        memcpy(map_info.data, pData, nSize);

        gst_buffer_unmap(pBuffer, &map_info);

        std::this_thread::sleep_for(std::chrono::microseconds(2));
    }
    catch (...)
    {
        return false;
    }

    return true;
}


// Function to push buffers into the pipeline

bool CGstWrapper::WriteToPipeline(GstBuffer* pBuffer)
{
    if (m_controlData.m_pAppsrc == nullptr || pBuffer == nullptr)
    {
        return false;
    }

    if ((m_controlData.m_videoConfig.m_width < 1) || 
        (m_controlData.m_videoConfig.m_height < 1) || 
        (m_controlData.m_videoConfig.m_bitsPerPixel < 8))
    {
        return false;
    }

    // Set buffer timestamp and duration (essential for smooth playback)

    //GST_BUFFER_PTS(pBuffer) = gst_util_uint64_scale_int(g_get_monotonic_time(), GST_SECOND, 1000000);
    GST_BUFFER_PTS(pBuffer) = GST_CLOCK_TIME_NONE;

    //GST_BUFFER_DURATION(pBuffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30); // Assuming 30 fps
    GST_BUFFER_DTS(pBuffer) = GST_CLOCK_TIME_NONE;

    // Push the buffer to the appsrc

    GstFlowReturn ret = GST_FLOW_OK;

#if 0

    g_signal_emit_by_name(m_controlData.m_pAppsrc, "push-buffer", pBuffer, &ret);

    if (ret != GST_FLOW_OK)
    {
        // Got an error, stop pushing m_controlData

        m_controlData.m_bQuit = true;

        return false;
    }

#else

    ret = gst_app_src_push_buffer(GST_APP_SRC(m_controlData.m_pAppsrc), pBuffer);

    if (ret != GST_FLOW_OK) 
    {
        return false;
    }

#endif

    // sleep for a few ms to give gStreamer time to process the buffer

    long delay = (long) ((float) 1000 / (float) (m_controlData.m_videoConfig.m_frameRate / 2));

    std::this_thread::sleep_for(std::chrono::microseconds(delay));

    return true; // Success - Continue ..
}


bool CGstWrapper::StartPipeline()
{
    GstStateChangeReturn ret;

    // Set the pipeline to PLAYING state
    ret = gst_element_set_state(m_controlData.m_pPipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) 
    {
        return false;
    }

    if (m_controlData.m_pBus == nullptr)
    { 
        m_controlData.m_pBus = gst_element_get_bus(m_controlData.m_pPipeline);
    }

    if (m_controlData.m_pBus == nullptr)
    {
        return false;
    }

#if 0
    if (m_controlData.m_pMsg == nullptr)
    {
        m_controlData.m_pMsg =
            gst_bus_timed_pop_filtered
            (
                m_controlData.m_pBus,
                GST_CLOCK_TIME_NONE, 
                (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
            );
    }
#endif

    m_controlData.m_bPipelineActive = true;

    return true;
}


bool CGstWrapper::StopPipeline()
{
    if (m_controlData.m_bPipelineActive == true)
    {
        return false;
    }    

    m_controlData.m_bQuit = true;
        
    std::this_thread::sleep_for(std::chrono::microseconds(5));

    GstStateChangeReturn ret;

    // Set the pipeline to PLAYING state
    ret = gst_element_set_state(m_controlData.m_pPipeline, GST_STATE_NULL);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }

    m_controlData.m_bPipelineActive = false;

    return true;
}


void CGstWrapper::Release()
{
    if (m_controlData.m_bPipelineActive == true)
    {
        StopPipeline();
    }

    // Free resources

    if (m_controlData.m_pPipeline != nullptr)
    {
        GstEvent* eos_event = gst_event_new_eos();

        gst_element_send_event(m_controlData.m_pPipeline, eos_event);

        std::this_thread::sleep_for(std::chrono::microseconds(10));

        gst_element_set_state(m_controlData.m_pPipeline, GST_STATE_NULL);

        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    if (m_controlData.m_pBus != nullptr)
    { 
        gst_object_unref(m_controlData.m_pBus);
    }

    if (m_controlData.m_pPipeline != nullptr)
    { 
        gst_object_unref(m_controlData.m_pPipeline);
    }
}


