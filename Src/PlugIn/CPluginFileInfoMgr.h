///****************************************************************************
/// FILE:    CPluginFileInfoMgr.h
///
/// DEESC:   Plugin (lib) interface manager class def
///
/// AUTHOR:  Russ Barker
///


#ifndef PLUGIN_FILE_INFO_MANAGER_H
#define PLUGIN_FILE_INFO_MANAGER_H


#include <string>
#include <vector>
#include <memory>
#include <vector>

#include "../Error/CError.h"

#include "../Logging/Logging.h"

#include "PluginDefs.h"


#ifndef PLUGIN_GROUP_ENUM
#define PLUGIN_GROUP_ENUM
typedef enum ePluginGroup_tag
{
    ePluginGroup_unknown = 0,

    ePluginGroup_audio_in,
    ePluginGroup_audio_out,
    ePluginGroup_audio_io,

    ePluginGroup_video_in,
    ePluginGroup_video_out,
    ePluginGroup_video_io,

    ePluginGroup_cmd_in,
    ePluginGroup_cmd_out,
    ePluginGroup_cmd_io,

    ePluginGroup_rtd_in,
    ePluginGroup_rtd_out,
    ePluginGroup_rtd_io,

    ePluginGroup_xfmt_in,
    ePluginGroup_xfmt_out,
    ePluginGroup_xfmt_io,

    ePluginGroup_msg_in,
    ePluginGroup_msg_out,

    ePluginGroup_hwc,

} ePluginGroup_def;
#endif


#ifndef PLUFIN_CAPS_DEF
#define PLUGIN_CAPS_DEF

struct PluginXfaceCap_def
{
    uint32_t            m_nMaxInputInterfaces;
    uint32_t            m_nMaxOutputInterfaces;
    uint32_t            m_nMaxIoInterfaces;

    PluginXfaceCap_def()
    {
        m_nMaxInputInterfaces = 0;
        m_nMaxOutputInterfaces = 0;
        m_nMaxIoInterfaces = 0;
    }
};

struct AllPluginCaps_def
{
    PluginXfaceCap_def  m_cmdInterfaces;
    PluginXfaceCap_def  m_rtdInterfaces;
    PluginXfaceCap_def  m_audioInterfaces;
    PluginXfaceCap_def  m_videoInterfaces;
};

#endif


// All plugins MUST provide a root class 
// derived from this  

//class PLUGIN_LIB_API CPluginFileInfoMgrBase
class CPluginFileInfoMgrBase
{

protected:

    std::string         m_sPluginApiType;

    CVersionNumber      m_PluginApiVersion;

    std::string         m_sPluginModuleName;
    std::string         m_sPluginDescription;
    
    CVersionNumber      m_PluginModuleVersion;

    int                 m_nPluginType;

    ePluginGroup_def    m_ePluginGroup;

    PluginXfaceCap_def  m_cmdInterfaces;

public:

    CPluginFileInfoMgrBase()
    {
        m_sPluginApiType = "";

        clear();
    }

    ~CPluginFileInfoMgrBase()
    {
        //clear();
    }

    void clear()
    {
        try
        { 
            m_sPluginModuleName = "";
            m_sPluginDescription = "";
        }
        catch (...)
        { }

        m_nPluginType = 0;
    }

    CPluginFileInfoMgrBase getPluginInfo()
    {
        CPluginFileInfoMgrBase out;

        out = (*this);

        return out;
    }

    bool checkPluginApiType(const std::string &sApiType)
    {
        if (m_sPluginApiType == sApiType)
        {
            return true;
        }

        return false;
    }

    eCompareResult checkModuleVersion(const std::string sVer)
    {
        return m_PluginApiVersion.compare(sVer);
    }

    std::string GetPluginModuleName()
    {
        return m_sPluginModuleName;
    }

    CVersionNumber GetPluginModuleVersion()
    {
        return m_PluginModuleVersion;
    }

    std::string GetPluginDescription()
    {
        return m_sPluginDescription;
    }

    int GetPluginType()
    {
        return m_nPluginType;
    }

    ePluginGroup_def GetPluginGroup()
    {
        return m_ePluginGroup;
    }

    virtual bool getPluginCaps(AllPluginCaps_def &allPluginCaps)
    {
        allPluginCaps.m_cmdInterfaces = m_cmdInterfaces;

        return true;
    }

};


#endif  //  PLUGIN_FILE_INFO_MANAGER_H