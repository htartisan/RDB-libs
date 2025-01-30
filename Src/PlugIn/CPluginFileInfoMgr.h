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

#include "PluginDefs.h"


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

public:

    CPluginFileInfoMgrBase()
    {
        clear();
    }

    ~CPluginFileInfoMgrBase()
    {
        clear();
    }

    void clear()
    {
        m_sPluginModuleName.clear();
        m_sPluginDescription.clear();
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

};


#endif  //  PLUGIN_FILE_INFO_MANAGER_H